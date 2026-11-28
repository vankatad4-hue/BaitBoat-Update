#pragma once
#include <stdint.h>

// Fixed-size authenticated-link-to-UART envelope. Radio authentication happens
// before Boat forwards this compact command to the Sonar coprocessor.
namespace sonar_power_v1 {
constexpr uint8_t FRAME_SIZE = 8U;
constexpr uint8_t MAGIC0 = 0xA5U;
constexpr uint8_t MAGIC1 = 0x5AU;
constexpr uint8_t VERSION = 1U;
constexpr uint8_t REQUEST = 1U;
constexpr uint8_t ACK = 2U;
constexpr uint8_t OFF = 0U;
constexpr uint8_t ON = 1U;

struct Frame {
  uint8_t type = 0U;
  uint8_t state = 0U;
  uint16_t commandId = 0U;
};

inline uint8_t checksum(const uint8_t *bytes, uint8_t count) {
  uint8_t value = 0U;
  for (uint8_t i = 0U; i < count; ++i) value ^= bytes[i];
  return value;
}

inline bool encode(uint8_t type, uint8_t state, uint16_t commandId,
                   uint8_t out[FRAME_SIZE]) {
  if (!out || (type != REQUEST && type != ACK) || state > ON || commandId == 0U)
    return false;
  out[0] = MAGIC0; out[1] = MAGIC1; out[2] = VERSION; out[3] = type;
  out[4] = state; out[5] = static_cast<uint8_t>(commandId);
  out[6] = static_cast<uint8_t>(commandId >> 8U);
  out[7] = checksum(out + 2U, 5U);
  return true;
}

class Parser {
 public:
  void reset() { used_ = 0U; }
  bool feed(uint8_t byte, Frame &out) {
    if (used_ == 0U) {
      if (byte == MAGIC0) bytes_[used_++] = byte;
      return false;
    }
    if (used_ == 1U && byte != MAGIC1) {
      used_ = byte == MAGIC0 ? 1U : 0U;
      if (used_) bytes_[0] = MAGIC0;
      return false;
    }
    bytes_[used_++] = byte;
    if (used_ < FRAME_SIZE) return false;
    used_ = 0U;
    const uint16_t id = static_cast<uint16_t>(bytes_[5]) |
                        (static_cast<uint16_t>(bytes_[6]) << 8U);
    if (bytes_[2] != VERSION || (bytes_[3] != REQUEST && bytes_[3] != ACK) ||
        bytes_[4] > ON || id == 0U || checksum(bytes_ + 2U, 5U) != bytes_[7])
      return false;
    out = {bytes_[3], bytes_[4], id};
    return true;
  }
 private:
  uint8_t bytes_[FRAME_SIZE]{};
  uint8_t used_ = 0U;
};
} // namespace sonar_power_v1
