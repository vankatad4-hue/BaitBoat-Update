// R30H-16: bounded primary STOP attempts until Boat status, host only.
#include "r30h15_primary_stop_retry.h"
#include <assert.h>
#include <stdint.h>
struct StopCommand{uint32_t commandId=0;int type=1;};
int main(){
    using Q=flytec_r30h15::PrimaryStopRetry<StopCommand>;
    Q q;
    assert(!q.pending());
    assert(!q.mayAttempt(true,true,false,0U));
    assert(q.maySendPeriodic(true,false));
    q.queue({10U,1},100U);
    assert(q.pending()&&q.commandId()==10U);
    assert(q.command().type==1);
    assert(!q.maySendPeriodic(true,false));
    assert(!q.mayAttempt(false,true,false,100U));
    assert(!q.mayAttempt(true,false,false,100U));
    assert(!q.mayAttempt(true,true,true,100U));
    assert(q.mayAttempt(true,true,false,100U));
    q.noteAdmitted(100U); // TX buffer admission, *not* Boat execution
    assert(q.pending()&&q.transmissions()==1U);
    assert(!q.maySendPeriodic(true,false));
    assert(!q.mayAttempt(true,true,false,249U));
    assert(q.mayAttempt(true,true,false,250U));
    q.noteAdmitted(250U);
    assert(q.commandId()==10U&&q.transmissions()==2U);
    // ACK only says ACCEPTED; do not call reset until authenticated status.
    assert(q.pending());
    uint32_t at=400U;
    while(q.transmissions()<Q::MAX_TRANSMISSIONS){
        assert(q.mayAttempt(true,true,false,at));
        q.noteAdmitted(at);
        at+=Q::RETRANSMIT_GAP_MS;
    }
    assert(q.pending()&&q.transmissions()==Q::MAX_TRANSMISSIONS);
    assert(!q.mayAttempt(true,true,false,at)); // bounded max retries
    assert(q.maySendPeriodic(true,false)); // cap reached: keep telemetry link alive
    assert(!q.maySendPeriodic(true,true)); // never while RF24 is TX-busy
    q.reset(); // only caller may do so on validated Boat status
    assert(!q.pending()&&q.maySendPeriodic(true,false));
    q.queue({11U,1},100U);
    assert(q.mayAttempt(true,true,false,2600U));
    assert(!q.expired(2600U));
    assert(q.expired(2601U));
    assert(!q.mayAttempt(true,true,false,2601U));
    q.reset();
    q.queue({12U,1},200U);
    q.noteAdmitted(200U);
    q.queue({13U,1},300U); // newer local STOP supersedes earlier ID
    assert(q.commandId()==13U&&q.transmissions()==0U&&q.queuedAtMs()==300U);
    q.reset(); // auth epoch change forgets old logical ID
    assert(!q.pending());
    q.queue({14U,1},0xfffffffaU);
    assert(q.mayAttempt(true,true,false,115U));
    q.noteAdmitted(115U);
    assert(!q.mayAttempt(true,true,false,264U));
    assert(q.mayAttempt(true,true,false,265U));
    assert(q.expired(2496U)); // 32-bit rollover
    q.reset();
    return 0;
}
