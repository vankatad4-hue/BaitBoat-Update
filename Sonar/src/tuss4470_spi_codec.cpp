#include "tuss4470_spi_codec.h"

namespace {
uint8_t countOnes(uint16_t value) {
  uint8_t count = 0;
  while (value != 0U) {
    count = static_cast<uint8_t>(count + (value & 1U));
    value = static_cast<uint16_t>(value >> 1U);
  }
  return count;
}

uint16_t combine(const uint8_t frame[2]) {
  return static_cast<uint16_t>((static_cast<uint16_t>(frame[0]) << 8U) | frame[1]);
}
}

bool tuss4470_spi::encode(bool read, uint8_t address, uint8_t data,
                          uint8_t frame[2]) {
  if (!frame || address > 0x3FU) return false;
  uint16_t word = static_cast<uint16_t>((read ? 0x8000U : 0U) |
                                        (static_cast<uint16_t>(address) << 9U) |
                                        data);
  // Bit 8 is the controller parity field; include it only when needed to make
  // the complete 16-bit frame contain an odd number of one bits.
  if ((countOnes(word) & 1U) == 0U) word |= 0x0100U;
  frame[0] = static_cast<uint8_t>(word >> 8U);
  frame[1] = static_cast<uint8_t>(word & 0xFFU);
  return true;
}

bool tuss4470_spi::decode(const uint8_t frame[2], Response &response) {
  if (!frame) return false;
  const uint16_t word = combine(frame);
  if ((countOnes(word) & 1U) == 0U) return false;
  response.controllerParityError = (word & 0x8000U) != 0U;
  response.status = static_cast<uint8_t>((word >> 9U) & 0x3FU);
  response.data = static_cast<uint8_t>(word & 0xFFU);
  return true;
}
