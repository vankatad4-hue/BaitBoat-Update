#pragma once

#include <stdint.h>

// Bounded sonar sensor-data frames for the dedicated Sonar-to-Boat UART link.
// The power-control protocol has separate magic and remains independently
// authenticated by Boat before it is forwarded to this physical UART.
namespace sonar_data_v1 {
// 115200 8N1 reduces a full 136-byte echo-profile frame from about 142 ms at
// 9600 baud to about 12 ms while remaining conservative for ESP32 UART links.
constexpr uint32_t LINK_BAUD_RATE = 115200UL;
constexpr uint8_t MAGIC0 = 0xC3U;
constexpr uint8_t MAGIC1 = 0x3CU;
constexpr uint8_t VERSION = 1U;
constexpr uint8_t ECHO_PROFILE = 1U;
constexpr uint8_t WATER_TEMPERATURE = 2U;
constexpr uint8_t MAX_PAYLOAD = 128U;
constexpr uint8_t OVERHEAD = 8U; // header 6, CRC16 2
constexpr uint8_t MAX_FRAME = MAX_PAYLOAD + OVERHEAD;

struct Frame {
  uint8_t type = 0U;
  uint8_t sequence = 0U;
  uint8_t length = 0U;
  uint8_t payload[MAX_PAYLOAD]{};
};

inline uint16_t crc16(const uint8_t *data, uint8_t length) {
  uint16_t crc = 0xFFFFU;
  for (uint8_t i = 0U; i < length; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8U;
    for (uint8_t bit = 0U; bit < 8U; ++bit)
      crc = (crc & 0x8000U) ? static_cast<uint16_t>((crc << 1U) ^ 0x1021U)
                             : static_cast<uint16_t>(crc << 1U);
  }
  return crc;
}

inline bool validTypeLength(uint8_t type, uint8_t length) {
  return (type == ECHO_PROFILE && length == 128U) ||
         (type == WATER_TEMPERATURE && length == 3U);
}

inline bool encode(uint8_t type, uint8_t sequence, const uint8_t *payload,
                   uint8_t length, uint8_t *out, uint8_t capacity,
                   uint8_t &written) {
  written = 0U;
  if (!payload || !out || !validTypeLength(type, length) ||
      capacity < static_cast<uint8_t>(length + OVERHEAD)) return false;
  out[0] = MAGIC0; out[1] = MAGIC1; out[2] = VERSION; out[3] = type;
  out[4] = sequence; out[5] = length;
  for (uint8_t i = 0U; i < length; ++i) out[6U + i] = payload[i];
  const uint16_t crc = crc16(out + 2U, static_cast<uint8_t>(4U + length));
  out[6U + length] = static_cast<uint8_t>(crc >> 8U);
  out[7U + length] = static_cast<uint8_t>(crc);
  written = static_cast<uint8_t>(length + OVERHEAD);
  return true;
}

class Parser {
 public:
  void reset() { used_ = 0U; expected_ = 0U; }
  bool pending() const { return used_ != 0U; }

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
    if (used_ >= sizeof(bytes_)) { reset(); return false; }
    bytes_[used_++] = byte;
    if (used_ == 6U) {
      if (bytes_[2] != VERSION || !validTypeLength(bytes_[3], bytes_[5])) {
        reset(); return false;
      }
      expected_ = static_cast<uint8_t>(bytes_[5] + OVERHEAD);
    }
    if (!expected_ || used_ < expected_) return false;
    const uint8_t payloadLength = bytes_[5];
    const uint16_t received = static_cast<uint16_t>(bytes_[6U + payloadLength] << 8U) |
                              bytes_[7U + payloadLength];
    const uint16_t computed = crc16(bytes_ + 2U, static_cast<uint8_t>(4U + payloadLength));
    const bool valid = received == computed;
    if (valid) {
      out.type = bytes_[3]; out.sequence = bytes_[4]; out.length = payloadLength;
      for (uint8_t i = 0U; i < payloadLength; ++i) out.payload[i] = bytes_[6U + i];
    }
    reset();
    return valid;
  }

 private:
  uint8_t bytes_[MAX_FRAME]{};
  uint8_t used_ = 0U;
  uint8_t expected_ = 0U;
};
} // namespace sonar_data_v1
