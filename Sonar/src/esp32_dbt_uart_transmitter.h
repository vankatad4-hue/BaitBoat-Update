#pragma once

#ifndef FLYTEC_SONAR_UART_DRIVER_COMPILE
#define FLYTEC_SONAR_UART_DRIVER_COMPILE 0
#endif

#if FLYTEC_SONAR_UART_DRIVER_COMPILE

#include <Arduino.h>
#include "echo_profile_detector.h"
#include "sonar_data_protocol.h"

// Status for the compact depth-only link from sonar coprocessor to Boat.
enum class SonarDbtTransmitStatus : uint8_t {
  SENT,
  NOT_READY,
  NO_VALID_DEPTH,
  FORMAT_ERROR,
  UART_WRITE_ERROR,
};

// Gated one-way 9600-8N1 NMEA DBT output. It accepts only a FOUND detector
// result so invalid captures and no-echo results cannot become stale depths.
class Esp32DbtUartTransmitter {
 public:
  Esp32DbtUartTransmitter() = default;
  Esp32DbtUartTransmitter(const Esp32DbtUartTransmitter &) = delete;
  Esp32DbtUartTransmitter &operator=(const Esp32DbtUartTransmitter &) = delete;

  // Start UART1 only after the hardware enable, pin audit, and approval gates pass.
  bool begin(bool commissioningApproved);

  // Stop the UART after raising no other GPIO or transducer controls.
  void end();

  // Send one checksummed DBT sentence only for a validated detector result.
  SonarDbtTransmitStatus sendDetectedDepth(const EchoDetectionResult &result);

  // Send a complete 128-bin processed echo profile for Remote rendering.
  bool sendEchoProfile(uint8_t profileId, const uint8_t bins[128]);

  // Send signed centi-degrees Celsius and the sensor sample counter.
  bool sendWaterTemperature(int16_t temperatureCentiC, uint8_t sample);

  bool ready() const { return ready_; }

 private:
  static constexpr uint32_t BAUD_RATE = 9600U;
  bool sendDataFrame(uint8_t type, uint8_t sequence, const uint8_t *payload,
                     uint8_t length);
  bool ready_ = false;
};

#endif // FLYTEC_SONAR_UART_DRIVER_COMPILE
