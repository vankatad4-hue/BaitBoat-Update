#pragma once
#include <stdint.h>
#include "radio_utc_time_v1.h"

// R30H-6: UTC quality and external-marker commissioning rules.
// Non-actuating, no Arduino imports, no GPIO, no HMAC keys.
// Clock adjustment NEVER changes millis() or proves RF STOP latency.
namespace flytec_r30h6 {
enum class ClockQuality : uint8_t {
 INVALID=0, RECEIVER_COARSE=1, AUTHENTICATED_REMOTE_COARSE=2
};
struct ClockReport {
 ClockQuality quality=ClockQuality::INVALID;
 uint64_t unixMs=0;
 uint32_t ageMs=0;
 bool traceReferenceCertified=false; // always false without external PPS/common clock
 bool rfEndToEndStopClock=false;
};

inline ClockReport report(const flytec_time::Discipline &clock,
                          uint32_t nowMs, bool localReceiver,
                          bool authenticatedPrimaryLink){
 ClockReport out{};
 if(!localReceiver && !authenticatedPrimaryLink)return out;
 uint64_t utc=0;
 if(!clock.utcNow(nowMs,utc))return out;
 out.quality=localReceiver?ClockQuality::RECEIVER_COARSE:
                          ClockQuality::AUTHENTICATED_REMOTE_COARSE;
 out.unixMs=utc;
 out.ageMs=clock.lastSampleAgeMs(nowMs);
 return out;
}

enum class MarkerKind : uint8_t {
 AUTHENTICATED_RX=1, STOP_INTENT=2, LOCAL_RTH_ENTRY=3
};
struct MarkerReadiness {
 bool boardRevisionAudited=false;
 bool pinCrosswalkSigned=false;
 bool dedicatedDebugOnlyOutput=false;
 bool voltageLevelMeasured=false;
 bool actuatorHalUnattached=false;
 bool sharedExternalClockPresent=false;
 bool secretsRedacted=false;
};
inline bool mayConnectExternalMarker(const MarkerReadiness &s){
 return s.boardRevisionAudited&&s.pinCrosswalkSigned&&
        s.dedicatedDebugOnlyOutput&&s.voltageLevelMeasured&&
        s.actuatorHalUnattached&&s.sharedExternalClockPresent&&
        s.secretsRedacted;
}
// There is no actual marker GPIO implementation, pin number, or output
// side-effect in this module. Physical mark is NOT_INSTALLED until ALL
// conditions have separate evidence.
} // namespace flytec_r30h6
