// R30H-20: actual locally sampled Boat physical STATUS and critical
// battery must be representable on wire. Host-only: no actuator hardware.
#include "../source/boat_hardware_status.h"
#include "../source/boat_r30h19_status_outbox.h"
#include <cassert>
#include <iostream>
int main(){
    BoatHardwareSample s{};
    s.safetyAt=1000;s.safetyValid=true;s.outputFeedbackValid=true;
    s.watchdogReady=true;s.stationary=true;s.releaseIdle=true;
    s.releaseInterlocks=true;s.physicalStopReleased=false;
    s.batteryHealthy=false;s.batteryBand=BOAT_BATTERY_CRITICAL;
    s.batteryPercent=1;
    const uint32_t now=1020U;
    const auto sample=boatHardware_status(s,now,true,MC_IDLE,false,true);
    assert(sample.kind==RADIO_PAYLOAD_STATUS);
    assert(sample.stopState==1U);
    assert((sample.safetyFlags&0x20U)!=0U); // low/critical battery warning
    assert((sample.safetyFlags&0x80U)==0U); // strict reserved bit contract
    assert(sample.batteryPercent==1U);
    uint8_t wire[8]{};
    assert(radio_encodeTelemetryStatusPayload(sample.boatMode,sample.stopState,
           sample.safetyFlags,sample.batteryPercent,sample.gpsFixType,
           sample.satellites,wire));
    RadioDecodedPayload parsed{};
    assert(radio_decodeTelemetryPayload(wire,parsed));
    assert(parsed.kind==RADIO_PAYLOAD_STATUS&&parsed.stopState==1U);
    assert((parsed.safetyFlags&0x20U)!=0U);
    boat_r30h19::StatusOutbox outbox{};
    assert(outbox.offer(sample,now));
    assert(outbox.due(now,true,true,false,false));
    // Losing locally sampled output feedback changes STATUS to UNKNOWN,
    // and it cannot confirm a new STOP by inference from an old snapshot.
    s.outputFeedbackValid=false;
    const auto unknown=boatHardware_status(s,now,true,MC_IDLE,false,true);
    assert(unknown.stopState==2U);
    assert(outbox.offer(unknown,now+1));
    assert(outbox.status().stopState==2U);
    // A fully stale/invalid sample also cannot magically clear STOP.
    s.outputFeedbackValid=true;s.watchdogReady=false;
    const auto watchdogFault=boatHardware_status(s,now,true,MC_IDLE,false,true);
    assert(watchdogFault.stopState==2U);
    std::cout<<"R30H-20 real Boat battery/status wire contract PASS (host only)\n";
}
