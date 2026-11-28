#include "boat_open_echo_uart_port.h"

#ifndef BOAT_OPEN_ECHO_UART_ENABLED
#define BOAT_OPEN_ECHO_UART_ENABLED 0
#endif

#if BOAT_OPEN_ECHO_UART_ENABLED
#if !defined(ARDUINO_USB_CDC_ON_BOOT) || ARDUINO_USB_CDC_ON_BOOT != 1
#error "Sonar UART0 requires native USB CDC for the diagnostic Serial console"
#endif
#endif

bool BoatOpenEchoUartPort::begin(bool sensorOwnerReady) {
  active_ = false;
#if BOAT_OPEN_ECHO_UART_ENABLED
  // A compile-only profile is not physical pin approval. Refuse to configure
  // UART0 unless both the sensor owner and separate GPIO audit are approved.
  if (!sensorOwnerReady || BOAT_OPEN_ECHO_UART_PIN_AUDIT_SIGNED != 1) return false;
  Serial0.begin(9600, SERIAL_8N1, BOAT_OPEN_ECHO_UART_RX_PIN,
                BOAT_OPEN_ECHO_UART_TX_PIN);
  active_ = true;
#else
  (void)sensorOwnerReady;
#endif
  return active_;
}

void BoatOpenEchoUartPort::poll(uint32_t now) {
#if BOAT_OPEN_ECHO_UART_ENABLED
  if (!active_) return;
  if (powerPending_ && !powerFrameSent_ &&
      static_cast<int32_t>(now - powerSendAtMs_) >= 0) {
    Serial0.write(powerFrame_, sizeof(powerFrame_));
    Serial0.flush();
    powerFrameSent_ = true;
    powerDeadlineMs_ = now + 1200U;
  }
  for (uint8_t budget = 0U; budget < 96U && Serial0.available() > 0; ++budget) {
    const int readByte = Serial0.read();
    if (readByte < 0) continue;
    sonar_power_v1::Frame frame{};
    const uint8_t byte = static_cast<uint8_t>(readByte);
    if (powerParser_.feed(byte, frame)) {
      if (powerPending_ && powerFrameSent_ && frame.type == sonar_power_v1::ACK &&
          frame.commandId == powerCommandId_) {
        powerResultId_ = frame.commandId;
        powerResultSuccess_ = frame.state == (requestedEnabled_ ? sonar_power_v1::ON : sonar_power_v1::OFF);
        powerResultReady_ = true;
        powerPending_ = false;
      }
    } else {
      // Binary power frames are ignored by the NMEA DBT parser; valid ASCII
      // DBT bytes continue through the same existing sonar telemetry path.
      sensors_.feedOpenEchoDbtByte(static_cast<char>(byte), now);
    }
  }
  if (powerPending_ && powerFrameSent_ &&
      static_cast<int32_t>(now - powerDeadlineMs_) >= 0) {
    powerResultId_ = powerCommandId_;
    powerResultSuccess_ = false;
    powerResultReady_ = true;
    powerPending_ = false;
  }
#else
  (void)now;
#endif
}

bool BoatOpenEchoUartPort::requestPower(uint16_t commandId, bool enabled, uint32_t now) {
#if BOAT_OPEN_ECHO_UART_ENABLED
  if (!active_ || !commandId || powerPending_ || powerResultReady_) return false;
  if (!sonar_power_v1::encode(sonar_power_v1::REQUEST,
        enabled ? sonar_power_v1::ON : sonar_power_v1::OFF, commandId, powerFrame_))
    return false;
  // UART wake loses its first character, so send a short 0x55 edge preamble,
  // then send the framed idempotent request after the Sonar CPU wakes.
  static const uint8_t wakeEdges[4] = {0x55U, 0x55U, 0x55U, 0x55U};
  if (Serial0.write(wakeEdges, sizeof(wakeEdges)) != sizeof(wakeEdges)) return false;
  Serial0.flush();
  powerPending_ = true;
  powerFrameSent_ = false;
  requestedEnabled_ = enabled;
  powerCommandId_ = commandId;
  powerSendAtMs_ = now + 25U;
  powerDeadlineMs_ = 0U;
  return true;
#else
  (void)commandId; (void)enabled; (void)now;
  return false;
#endif
}

bool BoatOpenEchoUartPort::takePowerResult(uint16_t &commandId, bool &success) {
  if (!powerResultReady_) return false;
  commandId = powerResultId_;
  success = powerResultSuccess_;
  powerResultReady_ = false;
  return true;
}
