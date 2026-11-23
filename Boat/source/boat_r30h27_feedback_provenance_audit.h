#pragma once
#include <stdint.h>
#include "boat_common_bus_readonly_port_r30d.h"
#include "boat_hardware_status.h"

// R30H-27: DIAGNOSTIC-ONLY origin classification of the seven R30D
// common-3S-bus observations. It has no GPIO, actuation or commissioning API.
// Matching source IDs, mock claims and software STOP=1 are NOT independent
// load-current measurements. A real signed independent review is unavailable.
namespace boat_r30h27 {
static constexpr bool PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE = false;
enum class Origin : uint8_t {
 UNKNOWN=0, SOFTWARE_COMMAND_ECHO, MCP_OUTPUT_LATCH_READBACK,
 MCU_DERIVED_ESTIMATE, EXTERNAL_DRY_CONTACT, EXTERNAL_PULSE_LIMITER,
 EXTERNAL_CURRENT_MEASUREMENT, EXTERNAL_BUS_VOLTAGE_MEASUREMENT
};
enum class Verdict : uint8_t {
 NO_VALID_DIAGNOSTIC=0, STALE_DIAGNOSTIC, SOURCE_ID_MISMATCH,
 COMMAND_OR_DERIVED_ECHO, ORIGIN_TYPE_INCOMPATIBLE,
 SOURCE_REVIEW_MISSING, REPORTED_SAFETY_FAULT,
 REPORTED_CURRENT_NOT_ZERO, PHYSICAL_EVIDENCE_NOT_SIGNED
};
struct SourceDeclaration {
 Origin origin=Origin::UNKNOWN;
 uint8_t rawSourceId=0;
 bool claimedPolarityChecked=false;
 bool claimedWiringReviewed=false;
 bool claimedIndependentMeasurementChecked=false;
};
struct Profile {
 // Order: STOP loop, external watchdog, common DC disconnect aux,
 // independent one-hot pulse limiter, coil current, motor current,
 // INA231 BUS-only pack voltage. Sources still unselected/uncommissioned.
 SourceDeclaration channels[7]{};
};
struct Review {
 Verdict verdict=Verdict::NO_VALID_DIAGNOSTIC;
 bool rawFrameAvailable=false;
 // Intentionally NEVER true in current uncommissioned firmware.
 bool independentlyProvenPhysicalStop=false;
};
inline Origin requiredOrigin(unsigned i) {
 if(i<3U)return Origin::EXTERNAL_DRY_CONTACT;
 if(i==3U)return Origin::EXTERNAL_PULSE_LIMITER;
 if(i==4U||i==5U)return Origin::EXTERNAL_CURRENT_MEASUREMENT;
 return Origin::EXTERNAL_BUS_VOLTAGE_MEASUREMENT;
}
inline Review inspect(const boat_r30d::Capture &capture,
                      const Profile &profile,uint32_t nowMs) {
 Review out{};
 if(!capture.observed ||
    capture.result!=boat_r30d::Result::OK_DIAGNOSTIC_ONLY)
  return out;
 out.rawFrameAvailable=true;
 if((uint32_t)(nowMs-capture.feedback.sampledAtMs)>=
       boat_r30c::MAX_FEEDBACK_AGE_MS ||
    (uint32_t)(nowMs-capture.raw.battery.acquiredAtMs)>
       boat_r30d::MAX_BATTERY_AGE_MS) {
  out.verdict=Verdict::STALE_DIAGNOSTIC;return out;
 }
 const uint8_t ids[7]={
   capture.raw.stop.sourceId,capture.raw.watchdog.sourceId,
   capture.raw.commonDisconnectAux.sourceId,
   capture.raw.coilPulseLimiter.sourceId,
   capture.raw.coilCurrentZero.sourceId,
   capture.raw.motorCurrentZero.sourceId,capture.raw.battery.sourceId
 };
 for(unsigned i=0;i<7U;++i) {
  // Provenance metadata cannot rename or impersonate a sampled source.
  if(!ids[i]||ids[i]!=profile.channels[i].rawSourceId) {
   out.verdict=Verdict::SOURCE_ID_MISMATCH;return out;
  }
  const Origin origin=profile.channels[i].origin;
  if(origin==Origin::UNKNOWN ||
     origin==Origin::SOFTWARE_COMMAND_ECHO ||
     origin==Origin::MCP_OUTPUT_LATCH_READBACK ||
     origin==Origin::MCU_DERIVED_ESTIMATE) {
   out.verdict=Verdict::COMMAND_OR_DERIVED_ECHO;return out;
  }
  if(origin!=requiredOrigin(i)) {
   out.verdict=Verdict::ORIGIN_TYPE_INCOMPATIBLE;return out;
  }
  const auto &claim=profile.channels[i];
  if(!claim.claimedPolarityChecked||!claim.claimedWiringReviewed||
     !claim.claimedIndependentMeasurementChecked) {
   out.verdict=Verdict::SOURCE_REVIEW_MISSING;return out;
  }
 }
 if(!capture.raw.stop.asserted||!capture.raw.watchdog.asserted||
    !capture.raw.commonDisconnectAux.asserted||
    !capture.raw.coilPulseLimiter.asserted) {
  out.verdict=Verdict::REPORTED_SAFETY_FAULT;return out;
 }
 if(!capture.raw.motorCurrentZero.asserted||
    !capture.raw.coilCurrentZero.asserted) {
  out.verdict=Verdict::REPORTED_CURRENT_NOT_ZERO;return out;
 }
 // A perfect synthetic frame and self-reported provenance claims are NOT
 // physical commissioning, and can never turn these flags into proof.
 out.verdict=Verdict::PHYSICAL_EVIDENCE_NOT_SIGNED;
 return out;
}
inline bool mayCertifyCurrentZero(const Review &review) {
 return PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE &&
        review.independentlyProvenPhysicalStop;
}
} // namespace boat_r30h27
