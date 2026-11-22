# Boat source — current integration notes

Active Boat hardware decisions are in [BOAT_HARDWARE_SELECTION_R20.md](../docs/BOAT_HARDWARE_SELECTION_R20.md). Older source inventory is archived at [Boat_source_README_pre_R79.md](../../docs/archive/2026-11-22/Boat_source_README_pre_R79.md).

## Key modules

- `boat_application_port.*`: production application bridge, hardware sample ingestion, safety supervision, telemetry publication.
- `boat_battery_policy.h`: filtered 3S pack estimate, green/yellow/red state, low-battery RTH latch and critical STOP.
- `boat_dual_radio_port.*`: authenticated nRF24/E22 session management, radio-loss timer, Home NVS and local low-battery diversion.
- `boat_home_point_nvs.*`: two-slot CRC-verified Boat-local Home persistence.
- `mission_execution.h` / `mission_controller.h`: mission state, outbound hold on low battery, Home diversion and no-radio Home navigation.
- `boat_hardware_supervisor.h`: motor/release output policy, one-at-a-time releases, low-battery outbound inhibit and Home-only RTH gate.
- `tracker_sensor_port.*`: GNSS/IMU and selected R27/R28 INA231 BUS-only battery-voltage acquisition. Physical I²C access and calibration stay disabled pending board inspection; the retired ADS1115 fallback is inactive.

## R79 battery behavior

The Boat computes battery state locally from a fresh voltage sample. Red battery holds outbound mission movement; after 3 seconds the application requests Home diversion for an eligible active mission. Critical battery or stale ADC data causes STOP. RTH is blocked if Home, navigation, battery or local release/STOP interlocks are invalid. Release outputs remain OFF during RTH.

## Hardware boundary

The current firmware target is a diagnostic/non-actuating build. The [R20 GPIO/header map](../docs/BOAT_GPIO_HEADER_MAP_R20.md) is a candidate only; pin-audit flags remain false. Only a commissioned, calibrated INA231 BUS-only voltage sample may eventually supply battery-health validity; none exists yet, so the physical-driver gate stays closed. Independent STOP/watchdog/pulse-limiter/current-zero feedback and physical driver bindings still require sign-off. Host tests and target compilation do not commission actual ADC, GNSS, radio, MDD20A or EKM014 hardware.

## R22 read-only ADS1115 module

- `boat_ads1115_bench_math.h`: portable raw conversion to a plausibility-tagged, explicitly **uncalibrated** pack-voltage estimate; no Boat health or mission authorization.
- `../hardware_test/ADS1115_Battery_Bench/ADS1115_Battery_Bench.ino`: dedicated USB-serial ADC sketch, locked by default and devoid of radio/motor/solenoid control.
- `../docs/BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md`: pin audit, 3.3V supply, ADS1115 input, serial capture and physical evidence procedure.

The production Boat firmware is unchanged by this diagnostic sketch and remains non-actuating.

## R23 pin and connection contract

- `boat_r23_gpio_connection_contract.h`: 18 pinned J1/J3 candidates, reserved GPIO including 1.8V GPIO47/48, planned I²C addresses and physical-approval flags set **false**.
- `../tools/verify_gpio_connections_r23.py`: verifies all 64 harness nets against the 44-position R20 Espressif header crosswalk, radio macros and locked output states.
- `../tests/boat_r23_gpio_contract_test.cpp`: C++11 collision/reserved-pin/radio assignment regression.
- [R23 full wiring design](../docs/BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md).

The `boat_radio_hw_config.h` numeric GPIOs are unchanged and remain audit-locked. `NRF24Link::begin()` already calls explicit `SPIClass::begin(sck, miso, mosi, csn)`, so the selected GPIO11/12/13 SPI route is not relying on board-default SPI pins. GNSS/I²C sensor profile and all physical actuator drivers remain uncommissioned.

## Build inventory: retired implementation isolation (R30H-10 software pass)

- **Active radio transport:** nRF24L01+ plus E22 UART. The retired Heltec SX1262 implementation in `heltec_sx1262_link.{cpp,h}` is preserved solely for historical reference and old host regressions; it is excluded from both Boat PlatformIO and Arduino diagnostic target builds. RadioLib is no longer an active Boat target dependency. No removal of audit locks or adoption of a new radio is implied.
- The retired R21/R22 ADS1115 and R26 INA226 reference headers/tests remain available for historical regression only, not as active battery acquisition. R27/R28 INA231 BUS-only remains the selected (uncommissioned) path.
- Source-level inventory changes are not hardware acceptance. All R30A physical STOP, coil-current-zero, and R30H-10 RF/PPS measurements remain pending.
