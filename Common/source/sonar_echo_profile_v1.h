#pragma once

#include <stdint.h>
#include <string.h>

// Authenticated telemetry profile fragment format. Each complete profile is
// a 128-bin, 8-bit echo envelope produced from measured ADC samples. Five bins
// fit in each 8-byte radio payload after subtype, profile ID and chunk index.
namespace sonar_profile_v1 {
constexpr uint8_t SUBTYPE = 11U;
constexpr uint8_t BIN_COUNT = 128U;
constexpr uint8_t BINS_PER_CHUNK = 5U;
constexpr uint8_t CHUNK_COUNT = (BIN_COUNT + BINS_PER_CHUNK - 1U) / BINS_PER_CHUNK;
constexpr uint32_t ASSEMBLY_TIMEOUT_MS = 4000U;

struct Fragment {
  uint8_t profileId;
  uint8_t chunkIndex;
  uint8_t bins[BINS_PER_CHUNK];
};

inline bool encode(uint8_t profileId, uint8_t chunkIndex,
                   const uint8_t profile[BIN_COUNT], uint8_t payload[8]) {
  if (payload == nullptr) return false;
  memset(payload, 0, 8U);
  if (profile == nullptr || profileId == 0U || chunkIndex >= CHUNK_COUNT)
    return false;
  payload[0] = SUBTYPE;
  payload[1] = profileId;
  payload[2] = chunkIndex;
  const uint16_t offset = static_cast<uint16_t>(chunkIndex) * BINS_PER_CHUNK;
  for (uint8_t i = 0; i < BINS_PER_CHUNK && offset + i < BIN_COUNT; ++i)
    payload[3U + i] = profile[offset + i];
  return true;
}

inline bool decode(const uint8_t payload[8], Fragment &fragment) {
  if (payload == nullptr || payload[0] != SUBTYPE || payload[1] == 0U ||
      payload[2] >= CHUNK_COUNT) return false;
  const uint16_t offset = static_cast<uint16_t>(payload[2]) * BINS_PER_CHUNK;
  const uint8_t valid = static_cast<uint8_t>(BIN_COUNT - offset < BINS_PER_CHUNK
      ? BIN_COUNT - offset : BINS_PER_CHUNK);
  for (uint8_t i = valid; i < BINS_PER_CHUNK; ++i)
    if (payload[3U + i] != 0U) return false;
  fragment.profileId = payload[1];
  fragment.chunkIndex = payload[2];
  memcpy(fragment.bins, payload + 3U, BINS_PER_CHUNK);
  return true;
}

// Fixed-memory out-of-order receiver. Caller must pass only authenticated
// frames; this class validates fragment shape and completion, not HMAC.
class Reassembler {
 public:
  bool push(const uint8_t payload[8], uint32_t now,
            uint8_t completed[BIN_COUNT]) {
    Fragment fragment{};
    if (!decode(payload, fragment) || completed == nullptr) return false;
    if (!active_ || fragment.profileId != profileId_ ||
        static_cast<uint32_t>(now - lastAt_) > ASSEMBLY_TIMEOUT_MS) {
      reset(fragment.profileId, now);
    }
    lastAt_ = now;
    const uint32_t bit = 1UL << fragment.chunkIndex;
    const uint16_t offset = static_cast<uint16_t>(fragment.chunkIndex) * BINS_PER_CHUNK;
    const uint8_t valid = static_cast<uint8_t>(BIN_COUNT - offset < BINS_PER_CHUNK
        ? BIN_COUNT - offset : BINS_PER_CHUNK);
    memcpy(profile_ + offset, fragment.bins, valid);
    received_ |= bit;
    const uint32_t completeMask = (1UL << CHUNK_COUNT) - 1UL;
    if (received_ != completeMask) return false;
    memcpy(completed, profile_, BIN_COUNT);
    active_ = false;
    return true;
  }

  void clear() { active_ = false; profileId_ = 0U; received_ = 0U; lastAt_ = 0U; }

 private:
  void reset(uint8_t id, uint32_t now) {
    memset(profile_, 0, sizeof(profile_));
    profileId_ = id;
    received_ = 0U;
    lastAt_ = now;
    active_ = true;
  }

  uint8_t profile_[BIN_COUNT]{};
  uint8_t profileId_ = 0U;
  uint32_t received_ = 0U;
  uint32_t lastAt_ = 0U;
  bool active_ = false;
};
} // namespace sonar_profile_v1
