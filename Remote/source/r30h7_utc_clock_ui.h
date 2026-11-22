#pragma once
#include "r30h6_utc_marker_policy.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// R30H-7: read-only coarse GPS UTC header presentation. The displayed
// wall-clock is NEVER an external common stopwatch, PPS, STOP timer or
// authenticated radio authority for motors/release/mission.
namespace flytec_r30h7 {
inline bool formatUtc(const flytec_r30h6::ClockReport &report,
                      char *out,size_t capacity) {
 if(!out||capacity==0U)return false;
 if(report.quality!=flytec_r30h6::ClockQuality::AUTHENTICATED_REMOTE_COARSE ||
    report.traceReferenceCertified||report.rfEndToEndStopClock||
    report.unixMs<flytec_time::MIN_MS||report.unixMs>=flytec_time::MAX_MS ||
    report.ageMs>flytec_time::DISCIPLINE_MAX_AGE_MS){
  (void)snprintf(out,capacity,"GPS UTC: --:--:--");
  return false;
 }
 const uint32_t secondsOfDay=(uint32_t)((report.unixMs/1000ULL)%86400ULL);
 const unsigned hh=(unsigned)(secondsOfDay/3600U);
 const unsigned mm=(unsigned)((secondsOfDay/60U)%60U);
 const unsigned ss=(unsigned)(secondsOfDay%60U);
 const int n=snprintf(out,capacity,"GPS UTC: %02u:%02u:%02u ~",hh,mm,ss);
 return n>=0&&(size_t)n<capacity;
}
}
