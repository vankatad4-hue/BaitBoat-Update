#include "esp32_tuss4470_transport.h"

#if FLYTEC_SONAR_SPI_DRIVER_COMPILE

#include "sonar_pin_map.h"

#ifndef FLYTEC_SONAR_HARDWARE_ENABLED
#define FLYTEC_SONAR_HARDWARE_ENABLED 0
#endif

bool Esp32Tuss4470Transport::begin(bool commissioningApproved) {
  end();

  // Do not even claim candidate pins until both explicit gates are signed.
  if (!commissioningApproved || !sonar_pins::TUSS_SPI_AUDIT_SIGNED ||
      FLYTEC_SONAR_HARDWARE_ENABLED != 1) {
    return false;
  }

  pinMode(sonar_pins::TUSS_CS, OUTPUT);
  digitalWrite(sonar_pins::TUSS_CS, HIGH);
  if (!SPI.begin(sonar_pins::SPI_SCK, sonar_pins::SPI_MISO,
                 sonar_pins::SPI_MOSI, -1)) {
    SPI.end();
    pinMode(sonar_pins::TUSS_CS, INPUT);
    return false;
  }
  ready_ = true;
  return true;
}

void Esp32Tuss4470Transport::end() {
  if (!ready_) return;
  digitalWrite(sonar_pins::TUSS_CS, HIGH);
  SPI.end();
  ready_ = false;
}

Tuss4470TransportStatus Esp32Tuss4470Transport::readRegister(
    uint8_t address, uint8_t &value, tuss4470_spi::Response &response) {
  uint8_t txFrame[2]{};
  if (!tuss4470_spi::encodeRead(address, txFrame))
    return Tuss4470TransportStatus::INVALID_ARGUMENT;

  const auto status = transfer(txFrame, response);
  if (status == Tuss4470TransportStatus::OK) value = response.data;
  return status;
}

Tuss4470TransportStatus Esp32Tuss4470Transport::writeRegister(
    uint8_t address, uint8_t value, tuss4470_spi::Response &response) {
  uint8_t txFrame[2]{};
  if (!tuss4470_spi::encodeWrite(address, value, txFrame))
    return Tuss4470TransportStatus::INVALID_ARGUMENT;
  const auto status = transfer(txFrame, response);
  if (status != Tuss4470TransportStatus::OK) return status;

  // The TUSS4470 echoes the write command's R/W and address in the response
  // data byte, with bit 0 cleared; a mismatch can indicate a lost frame.
  const uint8_t expectedEcho = static_cast<uint8_t>((address & 0x3FU) << 1U);
  return response.data == expectedEcho
             ? Tuss4470TransportStatus::OK
             : Tuss4470TransportStatus::WRITE_ADDRESS_ECHO_MISMATCH;
}

Tuss4470TransportStatus Esp32Tuss4470Transport::transfer(
    const uint8_t txFrame[2], tuss4470_spi::Response &response) {
  if (!ready_) return Tuss4470TransportStatus::NOT_READY;
  if (!txFrame) return Tuss4470TransportStatus::INVALID_ARGUMENT;

  const uint16_t txWord = static_cast<uint16_t>(
      (static_cast<uint16_t>(txFrame[0]) << 8U) | txFrame[1]);
  // Mode 1 follows the Open Echo sketch; verify the electrical edges on this shield.
  SPI.beginTransaction(SPISettings(SPI_RATE_HZ, MSBFIRST, SPI_MODE1));
  digitalWrite(sonar_pins::TUSS_CS, LOW);
  const uint16_t rxWord = SPI.transfer16(txWord);
  digitalWrite(sonar_pins::TUSS_CS, HIGH);
  SPI.endTransaction();

  // The TUSS4470 requires NCS high between frames. Hold it high for a short
  // guard interval instead of allowing adjacent register calls to merge.
  delayMicroseconds(NCS_HIGH_GUARD_US);

  const uint8_t rxFrame[2] = {
      static_cast<uint8_t>(rxWord >> 8U),
      static_cast<uint8_t>(rxWord & 0xFFU),
  };
  if (!tuss4470_spi::decode(rxFrame, response))
    return Tuss4470TransportStatus::RESPONSE_PARITY_ERROR;
  if (response.controllerParityError)
    return Tuss4470TransportStatus::PREVIOUS_CONTROLLER_PARITY_ERROR;
  if (response.pulseNumberFault || response.driverPulseFault ||
      response.eepromCrcFault)
    return Tuss4470TransportStatus::DEVICE_FAULT;
  return Tuss4470TransportStatus::OK;
}

#endif // FLYTEC_SONAR_SPI_DRIVER_COMPILE
