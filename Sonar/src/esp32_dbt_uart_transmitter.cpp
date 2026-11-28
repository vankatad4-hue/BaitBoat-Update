#include "esp32_dbt_uart_transmitter.h"

#if FLYTEC_SONAR_UART_DRIVER_COMPILE

#include <string.h>
#include "dbt_sentence.h"
#include "sonar_pin_map.h"

#ifndef FLYTEC_SONAR_HARDWARE_ENABLED
#define FLYTEC_SONAR_HARDWARE_ENABLED 0
#endif

bool Esp32DbtUartTransmitter::begin(bool commissioningApproved) {
  end();

  // Candidate TX stays untouched until code and board-level reviews are signed.
  if (!commissioningApproved || !sonar_pins::BOAT_UART_TX_AUDIT_SIGNED ||
      FLYTEC_SONAR_HARDWARE_ENABLED != 1) {
    return false;
  }

  Serial1.begin(BAUD_RATE, SERIAL_8N1, -1, sonar_pins::BOAT_UART_TX);
  ready_ = true;
  return true;
}

void Esp32DbtUartTransmitter::end() {
  if (!ready_) return;
  Serial1.end();
  ready_ = false;
}

SonarDbtTransmitStatus Esp32DbtUartTransmitter::sendDetectedDepth(
    const EchoDetectionResult &result) {
  if (!ready_) return SonarDbtTransmitStatus::NOT_READY;
  if (result.status != EchoDetectionStatus::FOUND)
    return SonarDbtTransmitStatus::NO_VALID_DEPTH;

  char sentence[64]{};
  if (!openEchoFormatDbt(result.depthCm, sentence, sizeof(sentence)))
    return SonarDbtTransmitStatus::FORMAT_ERROR;

  const size_t length = strlen(sentence);
  const size_t written = Serial1.write(
      reinterpret_cast<const uint8_t *>(sentence), length);
  Serial1.flush(); // Finish this bounded sentence before another sonar cycle.
  return written == length ? SonarDbtTransmitStatus::SENT
                           : SonarDbtTransmitStatus::UART_WRITE_ERROR;
}

bool Esp32DbtUartTransmitter::sendEchoProfile(uint8_t profileId,
                                               const uint8_t bins[128]) {
  return sendDataFrame(sonar_data_v1::ECHO_PROFILE, profileId, bins, 128U);
}

bool Esp32DbtUartTransmitter::sendWaterTemperature(int16_t temperatureCentiC,
                                                    uint8_t sample) {
  const uint16_t encoded = static_cast<uint16_t>(temperatureCentiC);
  const uint8_t payload[3] = {static_cast<uint8_t>(encoded >> 8U),
                              static_cast<uint8_t>(encoded), sample};
  return sendDataFrame(sonar_data_v1::WATER_TEMPERATURE, sample, payload, 3U);
}

bool Esp32DbtUartTransmitter::sendDataFrame(uint8_t type, uint8_t sequence,
                                            const uint8_t *payload,
                                            uint8_t length) {
  if (!ready_) return false;
  uint8_t bytes[sonar_data_v1::MAX_FRAME]{};
  uint8_t written = 0U;
  if (!sonar_data_v1::encode(type, sequence, payload, length, bytes,
                             sizeof(bytes), written)) return false;
  return Serial1.write(bytes, written) == written;
}

#endif // FLYTEC_SONAR_UART_DRIVER_COMPILE
