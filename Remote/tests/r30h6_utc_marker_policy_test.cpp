#include "../source/r30h6_utc_marker_policy.h"
#include <assert.h>
#include <stdio.h>
int main(){
 using namespace flytec_r30h6;
 flytec_time::Discipline clock;
 const uint64_t utc=1795347296123ULL;
 assert(clock.observe(utc,500));
 assert(clock.observe(utc+5000,5500));
 auto q=report(clock,5510,false,true);
 assert(q.quality==ClockQuality::AUTHENTICATED_REMOTE_COARSE);
 assert(q.unixMs==utc+5010ULL && q.ageMs==10);
 assert(!q.traceReferenceCertified&&!q.rfEndToEndStopClock);
 assert(report(clock,5510,false,false).quality==ClockQuality::INVALID);
 assert(report(clock,25501,false,true).quality==ClockQuality::INVALID);
 printf("R30H-6 Remote UTC quality observes authenticated primary link only (NO_RF)\n");
}
