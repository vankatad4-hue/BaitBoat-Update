/*
 * R30A INA231-only Boat battery diagnostic.
 *
 * NO motor, EKM014, motor PWM, nRF24/E22 transmission or mission includes.
 * Default compiled form is ALWAYS LOCKED: the globally unsigned R27 hardware
 * commissioning flag is false. Merely copying a local header cannot bypass it.
 *
 * DO NOT connect 3S battery or INA231 BUS until actual Ardboard #1491 PCB
 * topology, 0.1-ohm R100 shunt bypass, pull-ups and 3.3V I2C are inspected.
 * This is for later current-limited logic-only bench operation after signoff.
 */
#include <Arduino.h>
#include <Wire.h>
#include "ina231_voltage_contract_r27.h"
#include "ina231_acquisition_r28.h"
#include "ina231_arduino_wire_r28.h"

#if __has_include("ina231_local_bench_config.h")
#include "ina231_local_bench_config.h"
#else
// These MUST remain false in the checked-in diagnostic.
static constexpr bool INA231_BENCH_BOARD_VERIFIED = false;
static constexpr bool INA231_BENCH_PIN_AUDIT_SIGNED = false;
static constexpr bool INA231_BENCH_DMM_CALIBRATED = false;
static constexpr int INA231_BENCH_SDA = -1;
static constexpr int INA231_BENCH_SCL = -1;
static constexpr uint8_t INA231_BENCH_ADDRESS = 0x40;
#endif

// Immutable design gate until a documented physical R27 acceptance revision.
// Do not change just to obtain a serial reading.
static_assert(!boat_r27::INA231_PHYSICAL_COMMISSIONED,
              "R30A checked-in INA231 diagnostic must remain uncommissioned");

static boat_r28::VoltageSampler sampler;
static Ina231ArduinoWireR28 bus(Wire);
static bool ready = false;
static uint32_t lastFreshAt = 0;
static bool previouslyValid = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Flytec R30A INA231 sensor-only; all actuators physically disconnected.");
  const bool eligible = boat_r27::INA231_PHYSICAL_COMMISSIONED &&
                        INA231_BENCH_BOARD_VERIFIED &&
                        INA231_BENCH_PIN_AUDIT_SIGNED &&
                        INA231_BENCH_DMM_CALIBRATED &&
                        INA231_BENCH_SDA >= 0 && INA231_BENCH_SCL >= 0 &&
                        INA231_BENCH_SDA != INA231_BENCH_SCL &&
                        INA231_BENCH_ADDRESS == boat_r27::I2C_CANDIDATE_ADDRESS;
  if (!eligible) {
    Serial.println("INA231: LOCKED; no I2C pins configured, no battery BUS read.");
    return;
  }
  // Never reached in the checked-in firmware; physically measured/approved
  // local profile and explicit R27 release required for future bench only.
  if (!Wire.begin(INA231_BENCH_SDA,INA231_BENCH_SCL,100000U)) {
    Serial.println("INA231: I2C init failed; remains disabled.");
    return;
  }
  Wire.setTimeOut(10U);
  ready = sampler.begin(true, INA231_BENCH_ADDRESS);
  Serial.println(ready ? "INA231: sensor-only diagnostic ready" :
                         "INA231: address or configuration rejected");
}

void loop() {
  if (!ready) { delay(250); return; }
  const uint32_t now = millis();
  const auto result = sampler.poll(bus,now);
  const auto sample = sampler.snapshot(now);
  if (result == boat_r28::PollResult::Fresh &&
      sample.valid && sample.acquiredAtMs != lastFreshAt) {
    lastFreshAt = sample.acquiredAtMs;
    Serial.printf("INA231,VOLTAGE_ONLY,%lu,%u,1\n",
                  static_cast<unsigned long>(sample.acquiredAtMs),
                  static_cast<unsigned>(sample.millivolts));
  } else if (result == boat_r28::PollResult::Failed ||
             (!sample.valid && previouslyValid)) {
    Serial.println("INA231,INVALID,0,0,0");
  }
  previouslyValid = sample.valid;
  delay(2);
}
