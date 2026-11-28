#pragma once

#ifndef FLYTEC_SONAR_SPI_DRIVER_COMPILE
#define FLYTEC_SONAR_SPI_DRIVER_COMPILE 0
#endif

#if FLYTEC_SONAR_SPI_DRIVER_COMPILE

#include <Arduino.h>
#include <SPI.h>
#include "tuss4470_spi_codec.h"

// TUSS4470 transport status keeps frame-level failures visible to callers.
enum class Tuss4470TransportStatus : uint8_t {
  OK,
  NOT_READY,
  INVALID_ARGUMENT,
  RESPONSE_PARITY_ERROR,
  PREVIOUS_CONTROLLER_PARITY_ERROR,
  WRITE_ADDRESS_ECHO_MISMATCH,
  DEVICE_FAULT,
};

// Hardware adapter for one 16-bit frame per NCS pulse. The SPI pin audit and
// hardware-enable build flag must both be reviewed before begin() can start SPI.
class Esp32Tuss4470Transport {
 public:
  Esp32Tuss4470Transport() = default;
  Esp32Tuss4470Transport(const Esp32Tuss4470Transport &) = delete;
  Esp32Tuss4470Transport &operator=(const Esp32Tuss4470Transport &) = delete;

  // Start only when compile-time hardware enable, pin audit, and runtime approval agree.
  bool begin(bool commissioningApproved);

  // Release the SPI peripheral while leaving NCS high if it was started.
  void end();

  // Set `value` only on OK; parity-valid response diagnostics are returned separately.
  Tuss4470TransportStatus readRegister(uint8_t address, uint8_t &value,
                                       tuss4470_spi::Response &response);

  // Write one register frame and return its response/status diagnostics.
  Tuss4470TransportStatus writeRegister(uint8_t address, uint8_t value,
                                        tuss4470_spi::Response &response);

  bool ready() const { return ready_; }

 private:
  Tuss4470TransportStatus transfer(const uint8_t txFrame[2],
                                   tuss4470_spi::Response &response);

  static constexpr uint32_t SPI_RATE_HZ = 400000U; // Below TI's 500 kHz maximum.
  static constexpr uint32_t NCS_HIGH_GUARD_US = 3U; // Conservative gap; verify on a scope.

  bool ready_ = false;
};

#endif // FLYTEC_SONAR_SPI_DRIVER_COMPILE
