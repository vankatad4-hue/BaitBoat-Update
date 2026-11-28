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
  Serial0.begin(9600, SERIAL_8N1, BOAT_OPEN_ECHO_UART_RX_PIN, -1);
  active_ = true;
#else
  (void)sensorOwnerReady;
#endif
  return active_;
}

void BoatOpenEchoUartPort::poll(uint32_t now) {
#if BOAT_OPEN_ECHO_UART_ENABLED
  if (active_) sensors_.pollOpenEchoDbt(Serial0, now);
#else
  (void)now;
#endif
}
