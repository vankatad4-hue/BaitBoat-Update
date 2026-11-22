#include "../source/boat_telemetry.h"
#include "../source/radio_safety_diagnostic_v1.h"
#include <assert.h>
uint32_t millis(){return 0;}
int main(){
 telemetryInit();
 BoatState state{};state.radioPrimaryConnected=true;
 state.radioActiveLink=RADIO_UI_NRF_PRIMARY;
 auto d=RadioDecodedPayload{};
 d.kind=RADIO_PAYLOAD_SAFETY_DIAGNOSTIC;
 d.diagnosticReason=1;d.diagnosticFlags=0x0A02U;
 d.diagnosticMillivolts=0xffffU;d.diagnosticSequence=31;
 // Denied backup frame can never populate or extend diagnostic state.
 assert(!telemetryApplyRadioPayload(d,false,1000));
 assert(!telemetry.diagnosticValid);
 assert(telemetryApplyRadioPayload(d,true,1000));
 assert(telemetry.diagnosticValid&&telemetry.diagnosticPrimary);
 assert(telemetry.diagnosticReceivedAtMs==1000);
 assert(telemetry.statusReceivedAtMs==0 && !telemetry.statusValid);
 assert(telemetry.batteryReceivedAtMs==0 && !telemetry.batteryVoltageValid);
 assert(telemetry.receivedAtMs==0); // important: R30F does not renew control freshness
 telemetryApplyToBoatState(state,1100);
 assert(state.diagnosticFresh && state.diagnosticReason==1);
 assert(state.diagnosticFlags==0x0A02U);
 assert(!state.actuatorSafetyReady);
 assert(!state.emergencyStopConfirmed);
 // Valid future "uncommissioned" diagnostic still does not claim STOP state.
 d.diagnosticReason=14;d.diagnosticFlags=0x0803U;
 d.diagnosticMillivolts=12000U;d.diagnosticSequence=32;
 assert(telemetryApplyRadioPayload(d,true,1200));
 telemetryApplyToBoatState(state,1300);
 assert(state.diagnosticFresh&&state.diagnosticReason==14);
 assert(!state.actuatorSafetyReady&&!state.emergencyStopConfirmed);
 // Do not let a caller skip strict validation by injecting decoded objects.
 d.diagnosticFlags=0x0003U;
 assert(!telemetryApplyRadioPayload(d,true,1400));
 d.diagnosticFlags=0x0803U;d.diagnosticMillivolts=0xffffU;
 assert(!telemetryApplyRadioPayload(d,true,1400));
 assert(telemetry.diagnosticReceivedAtMs==1200);
 // A real trusted STOP/status packet remains solely authoritative for STOP.
 auto st=RadioDecodedPayload{};
 st.kind=RADIO_PAYLOAD_STATUS;st.boatMode=0;st.stopState=1;
 st.batteryPercent=60;st.safetyFlags=0;
 assert(telemetryApplyRadioPayload(st,true,1500));
 telemetryApplyToBoatState(state,1600);
 assert(state.emergencyStop && state.emergencyStopConfirmed);
 assert(!state.actuatorSafetyReady);
 // A subsequent fresh authenticated diagnostic remains display-only: even
 // while normal STOP status is confirmed it must not clear or override it.
 d.diagnosticReason=1;d.diagnosticFlags=0x0A02U;
 d.diagnosticMillivolts=0xffffU;
 assert(telemetryApplyRadioPayload(d,true,1650));
 telemetryApplyToBoatState(state,1660);
 assert(state.emergencyStop && state.emergencyStopConfirmed);
 assert(!state.actuatorSafetyReady);
 const uint32_t oldStatus=telemetry.statusReceivedAtMs;
 const uint32_t oldBattery=telemetry.batteryReceivedAtMs;
 d.diagnosticReason=1;d.diagnosticFlags=0x0A02U;
 d.diagnosticMillivolts=0xffffU;
 assert(telemetryApplyRadioPayload(d,true,1700));
 assert(telemetry.statusReceivedAtMs==oldStatus);
 assert(telemetry.batteryReceivedAtMs==oldBattery);
 telemetryApplyToBoatState(state,7201);
 assert(state.diagnosticFresh); // diagnostic received at 1700, 5501ms elapsed
 telemetryApplyToBoatState(state,7701);
 assert(!state.diagnosticFresh && state.diagnosticReason==0);
 assert(state.diagnosticFlags==0);
 assert(!state.emergencyStopConfirmed&&!state.actuatorSafetyReady);
 // Link falls to backup: keep diagnostics hidden despite fresh cached data.
 assert(telemetryApplyRadioPayload(d,true,8000));
 state.radioActiveLink=RADIO_UI_LORA_BACKUP;
 telemetryApplyToBoatState(state,8010);
 assert(!state.diagnosticFresh);
 state.radioActiveLink=RADIO_UI_NRF_PRIMARY;
 state.radioPrimaryConnected=false;
 telemetryApplyToBoatState(state,8010);
 assert(!state.diagnosticFresh);
 telemetryInit();
 assert(!telemetry.diagnosticValid);
 return 0;
}
