#pragma once

#include <stdint.h>

// Side-effect-free codec for the TUSS4470's 16-bit MSB-first SPI frame.
// It never opens SPI, toggles NCS, or accesses a sensor register.
namespace tuss4470_spi {

struct Response {
  bool controllerParityError; // Device reports an error in the previous MCU frame.
  uint8_t status;              // Preserve six device status bits without interpretation.
  uint8_t data;                // Read data or write-frame echo, depending on the request.
};

// Encode R/W, six-bit register address, odd frame parity, and data into two
// bytes in wire order. `read=true` emits the read bit and caller-supplied dummy byte.
bool encode(bool read, uint8_t address, uint8_t data, uint8_t frame[2]);

// Validate whole-frame odd parity before exposing device status or returned data.
bool decode(const uint8_t frame[2], Response &response);

} // namespace tuss4470_spi
