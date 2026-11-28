#pragma once

#include <stdint.h>

// Side-effect-free codec for the TUSS4470's 16-bit MSB-first SPI frame.
// It never opens SPI, toggles NCS, or accesses a sensor register.
namespace tuss4470_spi {

enum class DeviceState : uint8_t {
  LISTEN = 0U,
  BURST = 1U,
  STANDBY = 2U,
  SLEEP = 3U,
};

struct Response {
  bool controllerParityError; // Device reports an error in the previous MCU frame.
  uint8_t status;              // Raw six status bits, retained for diagnostics.
  bool vdrvReady;              // STAT5: regulator reached the configured voltage.
  bool pulseNumberFault;       // STAT4: burst ended before the configured pulse count.
  bool driverPulseFault;       // STAT3: TUSS4470 detected a driver pulse fault.
  bool eepromCrcFault;         // STAT2: internal EEPROM load reported a CRC error.
  DeviceState deviceState;     // STAT1:0: listen, burst, standby, or sleep.
  uint8_t data;                // Raw returned byte; meaning depends on SPI frame sequence.
};

// Encode a register read with the required zero data byte and odd frame parity.
bool encodeRead(uint8_t address, uint8_t frame[2]);

// Encode a register write with odd frame parity.
bool encodeWrite(uint8_t address, uint8_t data, uint8_t frame[2]);

// Validate whole-frame odd parity before exposing device status or returned data.
bool decode(const uint8_t frame[2], Response &response);

} // namespace tuss4470_spi
