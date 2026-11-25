#include "../source/utc_marker_policy.h"
#include <assert.h>
#include <stdio.h>
using namespace flytec_r30h6;
int main(){
 flytec_time::Discipline clock;
 auto none=report(clock,1000,true,false);
 assert(none.quality==ClockQuality::INVALID);
 assert(!none.traceReferenceCertified&&!none.rfEndToEndStopClock);
 assert(clock.lastSampleAgeMs(1000)==0xFFFFFFFFU);
 const uint64_t utc=1795347296123ULL;
 assert(clock.observe(utc,1000));
 assert(report(clock,1000,true,false).quality==ClockQuality::INVALID);
 assert(clock.observe(utc+1000,2000));
 auto boat=report(clock,2010,true,false);
 assert(boat.quality==ClockQuality::RECEIVER_COARSE);
 assert(boat.unixMs==utc+1010ULL && boat.ageMs==10U);
 assert(!boat.traceReferenceCertified&&!boat.rfEndToEndStopClock);
 auto remote=report(clock,2010,false,true);
 assert(remote.quality==ClockQuality::AUTHENTICATED_REMOTE_COARSE);
 assert(remote.unixMs==boat.unixMs);
 assert(report(clock,2010,false,false).quality==ClockQuality::INVALID);
 assert(report(clock,22001,false,true).quality==ClockQuality::INVALID);
 MarkerReadiness gate{};
 assert(!mayConnectExternalMarker(gate));
 gate.boardRevisionAudited=true;
 gate.pinCrosswalkSigned=true;
 gate.dedicatedDebugOnlyOutput=true;
 gate.voltageLevelMeasured=true;
 gate.actuatorHalUnattached=true;
 gate.sharedExternalClockPresent=true;
 assert(!mayConnectExternalMarker(gate)); // secrets evidence still missing
 gate.secretsRedacted=true;
 assert(mayConnectExternalMarker(gate)); // planning gate only, NO GPIO creation
 gate.actuatorHalUnattached=false;
 assert(!mayConnectExternalMarker(gate));
 printf("R30H-6 coarse UTC quality + no-GPIO physical-marker gate (NO_RF_NO_PPS_NO_ACTUATORS)\n");
}
