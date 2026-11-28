#include <Arduino.h>
#include "driver/uart.h"
#include "esp_sleep.h"
#include "dbt_sentence.h"
#include "sonar_pin_map.h"
#include "sonar_power_protocol.h"

// This target is intentionally a diagnostic placeholder until the exact shield
// revision, transducer, signal levels, and sampling implementation are reviewed.
#ifndef FLYTEC_SONAR_HARDWARE_ENABLED
#define FLYTEC_SONAR_HARDWARE_ENABLED 0
#endif

static_assert(FLYTEC_SONAR_HARDWARE_ENABLED == 0,
              "Sonar hardware activation requires a separately reviewed change");

#ifndef FLYTEC_SONAR_POWER_UART_COMPILE
#define FLYTEC_SONAR_POWER_UART_COMPILE 0
#endif
#ifndef FLYTEC_SONAR_POWER_UART_ENABLED
#define FLYTEC_SONAR_POWER_UART_ENABLED 0
#endif
static_assert(FLYTEC_SONAR_POWER_UART_ENABLED == 0 ||
              (sonar_pins::BOAT_UART_RX_AUDIT_SIGNED &&
               sonar_pins::BOAT_UART_TX_AUDIT_SIGNED),
              "UART sleep/wake requires both physical UART pin audits");

#if FLYTEC_SONAR_POWER_UART_COMPILE
static sonar_power_v1::Parser powerParser;
static bool sonarEnabled = true;

static void enterSonarLightSleep() {
  // The acquisition stack is currently compile-time disarmed, so there are no
  // ADC/TUSS tasks to stop. Future sensor drivers must add their stop hook here.
  Serial1.flush();
  delay(5U); // Let the authenticated state ACK clear the UART wire before sleep.
  uart_set_wakeup_threshold(UART_NUM_1, 3U);
  if (esp_sleep_enable_uart_wakeup(UART_NUM_1) == ESP_OK)
    (void)esp_light_sleep_start();
}

static void servicePowerControl() {
  if (FLYTEC_SONAR_POWER_UART_ENABLED != 1 ||
      !sonar_pins::BOAT_UART_RX_AUDIT_SIGNED ||
      !sonar_pins::BOAT_UART_TX_AUDIT_SIGNED) return;
  while (Serial1.available() > 0) {
    const int input = Serial1.read();
    if (input < 0) continue;
    sonar_power_v1::Frame request{};
    if (!powerParser.feed(static_cast<uint8_t>(input), request) ||
        request.type != sonar_power_v1::REQUEST) continue;
    sonarEnabled = request.state == sonar_power_v1::ON;
    uint8_t ack[sonar_power_v1::FRAME_SIZE]{};
    if (!sonar_power_v1::encode(sonar_power_v1::ACK, request.state,
                                 request.commandId, ack)) continue;
    Serial1.write(ack, sizeof(ack));
    if (!sonarEnabled) enterSonarLightSleep();
  }
}
#endif

void setup() {
  // USB serial is the default interface. The optional command UART is separate
  // from sensor activation and still requires an explicit, signed pin audit.
  Serial.begin(115200);
  Serial.println("FLYTEC_SONAR_COPROCESSOR,DISARMED,NO_SENSOR_DATA");
#if FLYTEC_SONAR_POWER_UART_COMPILE
  if (FLYTEC_SONAR_POWER_UART_ENABLED == 1 &&
      sonar_pins::BOAT_UART_RX_AUDIT_SIGNED &&
      sonar_pins::BOAT_UART_TX_AUDIT_SIGNED) {
    Serial1.begin(9600U, SERIAL_8N1, sonar_pins::BOAT_UART_RX,
                  sonar_pins::BOAT_UART_TX);
  }
#endif
}

void loop() {
  // A power request only changes the coprocessor sleep state; sensor acquisition
  // remains inactive until its independent hardware commissioning gate passes.
#if FLYTEC_SONAR_POWER_UART_COMPILE
  servicePowerControl();
  // Keep the optional control UART responsive. Sensor acquisition still
  // remains disarmed in every compile-only profile.
  delay(5U);
#else
  delay(1000U);
#endif
}
