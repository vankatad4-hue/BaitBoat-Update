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

bool encodeCommand(bool read, uint8_t address, uint8_t data, uint8_t frame[2]) {
  if (!frame || address > 0x3FU) return false;
  const uint16_t word = static_cast<uint16_t>((read ? 0x8000U : 0U) |
                                               (static_cast<uint16_t>(address) << 9U) |
                                               data);
  uint16_t frameWord = word;
  // Bit 8 is the controller parity field; add it only when required to make
  // the full 16-bit command frame contain an odd number of one bits.
  if ((countOnes(frameWord) & 1U) == 0U) frameWord |= 0x0100U;
  frame[0] = static_cast<uint8_t>(frameWord >> 8U);
  frame[1] = static_cast<uint8_t>(frameWord & 0xFFU);
  return true;
}
}

bool tuss4470_spi::encodeRead(uint8_t address, uint8_t frame[2]) {
  // TUSS4470 read commands send 0x00 in the command data field.
  return encodeCommand(true, address, 0U, frame);
}

bool tuss4470_spi::encodeWrite(uint8_t address, uint8_t data, uint8_t frame[2]) {
  return encodeCommand(false, address, data, frame);
}

bool tuss4470_spi::decode(const uint8_t frame[2], Response &response) {
  if (!frame) return false;
  const uint16_t word = combine(frame);
  if ((countOnes(word) & 1U) == 0U) return false;
  response.controllerParityError = (word & 0x8000U) != 0U;
  response.status = static_cast<uint8_t>((word >> 9U) & 0x3FU);
  response.vdrvReady = (response.status & 0x20U) != 0U;
  response.pulseNumberFault = (response.status & 0x10U) != 0U;
  response.driverPulseFault = (response.status & 0x08U) != 0U;
  response.eepromCrcFault = (response.status & 0x04U) != 0U;
  response.deviceState = static_cast<DeviceState>(response.status & 0x03U);
  response.data = static_cast<uint8_t>(word & 0xFFU);
  return true;
}
