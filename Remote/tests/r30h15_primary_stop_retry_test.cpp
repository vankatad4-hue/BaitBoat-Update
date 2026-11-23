// R30H-15 local retry policy only, no RF, actuator, ESP or real clock.
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
    assert(!q.maySendPeriodic(true,true));
    q.queue({10U,1},100U);
    assert(q.pending()&&q.commandId()==10U);
    assert(q.command().type==1);
    assert(!q.maySendPeriodic(true,false)); // STOP precedes heartbeat/mission
    assert(!q.mayAttempt(false,true,false,100U)); // no session
    assert(!q.mayAttempt(true,false,false,100U)); // no link
    assert(!q.mayAttempt(true,true,true,100U)); // old TX outstanding
    assert(q.mayAttempt(true,true,false,100U)); // TX done: retry first
    assert(q.mayAttempt(true,true,false,2600U)); // exactly 2500 ms
    assert(!q.expired(2600U));
    assert(q.expired(2601U));
    assert(!q.mayAttempt(true,true,false,2601U)); // never stale replay
    q.reset();
    assert(!q.pending());
    assert(q.maySendPeriodic(true,false)); // normal cycle after reset
    q.queue({11U,1},300U);
    q.queue({12U,1},302U); // latest local STOP supersedes pending old one
    assert(q.commandId()==12U&&q.queuedAtMs()==302U);
    q.reset(); // session epoch/key change flushes queued intent
    assert(!q.pending()&&!q.mayAttempt(true,true,false,500U));
    q.queue({13U,1},0xfffffffaU); // millis wrap
    assert(q.mayAttempt(true,true,false,115U));
    assert(q.expired(2496U)); // modulo 2^32 elapsed >2500
    q.reset();
    assert(!q.pending());
    return 0;
}
