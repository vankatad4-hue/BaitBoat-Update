#include "lora_link_policy.h"
#include <cassert>
#include <cstdint>

using namespace flytec_rtos;

struct Driver {
  enum class Admission { Accepted, Busy, DutyLimited, Invalid, Unavailable, Uncertain };
  unsigned writes = 0;
  bool ready() const { return true; }
  bool txPending() const { return false; }
  bool transportFault() const { return false; }
  Admission trySendFrame(const uint8_t &, uint32_t) {
    ++writes;
    return Admission::Accepted;
  }
};

static void fillQuiet(LoRaQuietRestart &proof, uint32_t start) {
  proof.observe(start, true);
  uint32_t now = start;
  for (unsigned i = 0; i < LoRaQuietRestart::QuietMs / LoRaQuietRestart::MaxSampleGap; ++i) {
    now += LoRaQuietRestart::MaxSampleGap;
    proof.observe(now, true);
  }
  assert(proof.ready(now));
}

int main() {
  LoRaQuietRestart boundary;
  boundary.observe(10, true);
  boundary.observe(1010, true); // exactly MaxSampleGap remains continuous
  boundary.observe(2011, true); // one millisecond over the limit restarts
  assert(!boundary.ready(2011));
  assert(!boundary.consume(2011));

  LoRaQuietRestart interrupted;
  interrupted.observe(10, true);
  interrupted.observe(11, false);
  interrupted.observe(12, true);
  assert(!interrupted.ready(12));

  LoRaQuietRestart wrapped;
  const uint32_t nearWrap = UINT32_MAX - 999U;
  fillQuiet(wrapped, nearWrap);
  const uint32_t wrappedNow = nearWrap + LoRaQuietRestart::QuietMs;
  assert(wrapped.consume(wrappedNow));
  assert(!wrapped.ready(wrappedNow)); // proof cannot be reused
  wrapped.observe(wrappedNow + 1U, true);
  assert(!wrapped.ready(wrappedNow + 1U));

  LoRaAirtimeProfile invalid{};
  invalid.commissioned = true;
  invalid.packetMs = 30;
  invalid.reserveMs = 20;
  LoRaQuietRestart invalidProof;
  fillQuiet(invalidProof, 0);
  LoRaAirtimePolicy invalidPolicy;
  assert(!invalidPolicy.configureAfterQuiet(invalid, LoRaQuietRestart::QuietMs, invalidProof));
  assert(invalidProof.ready(LoRaQuietRestart::QuietMs)); // bad profile cannot consume valid isolation proof
  LoRaAirtimeProfile corrected{};
  corrected.commissioned = true;
  assert(invalidPolicy.configureAfterQuiet(corrected, LoRaQuietRestart::QuietMs, invalidProof));
  assert(!invalidProof.ready(LoRaQuietRestart::QuietMs)); // successful configuration consumes proof

  LoRaAirtimeProfile profile{};
  profile.commissioned = true;
  LoRaQuietRestart proof;
  fillQuiet(proof, UINT32_MAX - 999U);
  const uint32_t configuredAt = UINT32_MAX - 999U + LoRaQuietRestart::QuietMs;
  LoRaAirtimePolicy policy;
  Driver driver;
  E22AdaptiveAdmission<Driver> admission(driver, policy, 20, true);
  const uint8_t frame = 0;
  using Traffic = LoRaAirtimePolicy::Traffic;
  assert(admission.start(frame, Traffic::Routine, configuredAt) == E22AdaptiveAdmission<Driver>::Result::Denied);
  assert(driver.writes == 0); // no airtime policy before the proof is configured
  assert(policy.configureAfterQuiet(profile, configuredAt, proof));
  assert(admission.start(frame, Traffic::Routine, configuredAt) == E22AdaptiveAdmission<Driver>::Result::Accepted);
  assert(driver.writes == 1);
}
