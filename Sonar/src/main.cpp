#include <Arduino.h>
#include "dbt_sentence.h"
#include "sonar_pin_map.h"

// This target is intentionally a diagnostic placeholder until the exact shield
// revision, transducer, signal levels, and sampling implementation are reviewed.
#ifndef FLYTEC_SONAR_HARDWARE_ENABLED
#define FLYTEC_SONAR_HARDWARE_ENABLED 0
#endif

static_assert(FLYTEC_SONAR_HARDWARE_ENABLED == 0,
              "Sonar hardware activation requires a separately reviewed change");

void setup() {
  // USB serial is the only active interface. No shield pins or Boat UART are
  // configured, so this firmware cannot trigger a transducer or publish depth.
  Serial.begin(115200);
  Serial.println("FLYTEC_SONAR_COPROCESSOR,DISARMED,NO_SENSOR_DATA");
}

void loop() {
  // Idle without touching the sensor or filling Boat's depth telemetry path.
  delay(1000);
}
