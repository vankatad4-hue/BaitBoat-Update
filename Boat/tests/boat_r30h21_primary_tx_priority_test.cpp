#include "../source/boat_stop_delivery.h"
#include "../source/radio_session_v2.h"
#include "../source/radio_payload_codec.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using boat_r30h21::PrimaryTxConditions;
using boat_r30h21::ordinarySlotFree;
using boat_r30h18::StopAckOutbox;
using boat_r30h19::StatusOutbox;
static bool ordinary(bool txBusy,const StopAckOutbox &ack,const StatusOutbox &status){
    return ordinarySlotFree({true,true,txBusy,ack.pending(),status.pending()});
}
static RadioDecodedPayload sample(uint8_t stop){
    RadioDecodedPayload s{};s.kind=RADIO_PAYLOAD_STATUS;
    s.stopState=stop;s.boatMode=0;s.batteryPercent=75;return s;
}
int main(){
    // No normal traffic may claim a failed transport, expired session or
    // occupied PHY, even when both priority queues are empty.
    for(unsigned bits=0;bits<32;++bits){
        const PrimaryTxConditions state={
          (bits&1U)!=0, (bits&2U)!=0, (bits&4U)!=0,
          (bits&8U)!=0, (bits&16U)!=0
        };
        assert(ordinarySlotFree(state)==(bits==3U));
    }
    StopAckOutbox ack;StatusOutbox status;
    uint16_t seq=0;
    auto buildOrdinary=[&](bool txBusy){
        if(!ordinary(txBusy,ack,status))return false;
        ++seq;return true;
    };
    // 50,000 fake scheduler iterations while STOP and TX-busy are active
    // must NOT burn authenticated session sequence numbers.
    assert(ack.offer(801,9,0,2,0,100U));
    for(unsigned i=0;i<50000;++i){
        assert(!buildOrdinary(true));
        assert(!buildOrdinary(false));
    }
    assert(seq==0&&ack.pending());
    assert(!ack.due(150U,true,true,true));
    assert(ack.due(150U,true,true,false));
    ack.admitted(150U);ack.completed(false);
    assert(!ack.due(224U,true,true,false));
    assert(ack.due(225U,true,true,false));
    ack.admitted(225U);ack.completed(true);
    assert(!ack.pending());
    // Real STATUS still has priority over heartbeat and may be UNKNOWN;
    // a positive STOP state is never invented by this admission policy.
    assert(status.offer(sample(2),230U));
    for(unsigned i=0;i<5000;++i)assert(!buildOrdinary(false));
    assert(seq==0 && !ordinary(false,ack,status));
    assert(status.due(231U,true,true,false,false));
    assert(status.status().stopState==2U);
    status.sent();
    assert(buildOrdinary(false)&&seq==1U);

    // Many iterations in a simulated busy radio only consume sequences
    // when the scheduler can actually admit an ordinary primary frame.
    // Do not infer on-air timing from these synthetic integer ticks.
    for(uint32_t now=1000U;now<6000U;++now){
        const bool initialBusy=now%4U!=0U; // deterministic stress pattern
        bool occupied=initialBusy;
        if(now==1500U)assert(ack.offer(802,9,0,2,0,now));
        if(now==1750U)assert(ack.offer(803,9,0,2,0,now)); // supersede
        if(now%200U==0U)assert(status.offer(sample((now/200U)&1U?2:1),now));
        if(ack.due(now,true,true,!initialBusy)){
            // Simulate the priority ACK claiming the first free slot.
            ack.admitted(now);ack.completed(true);occupied=true;
        }
        if(status.due(now,true,true,occupied,ack.pending())){
            status.sent();occupied=true;
        }
        for(unsigned burst=0;burst<20;++burst)
          if(buildOrdinary(occupied))occupied=true;
    }
    // 100,000+ scheduling attempts are bounded by actual free slots, not
    // an unbounded 16-bit application sequence churn/rekey.
    assert(seq>1U&&seq<1252U);
    assert(seq<RadioSessionCounter::REKEY_AT-1U);

    // A new authenticated session or missing primary PHY revokes STOP/STATUS
    // without spending ordinary sequence IDs or resuming actuator commands.
    const auto before=seq;
    assert(ack.offer(900,10,0,2,0,6500U));
    assert(status.offer(sample(2),6500U));
    assert(!ack.due(6501U,false,true,false));
    assert(!status.due(6501U,false,true,false,false));
    assert(!ack.pending()&&!status.pending());
    assert(!ordinarySlotFree({false,true,false,false,false}));
    assert(seq==before);

    std::cout<<"R30H-21 primary STOP/STATUS TX-busy sequence-budget stress PASS (host only)\n";
}
