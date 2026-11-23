> **CURRENT R30H-13 PlatformIO source migration (2026-11-23):** All **208 active** Boat/Remote source files covered by [repository-wide audit](../../docs/R30H_13_PLATFORMIO_SOURCE_MIGRATION.md); 17 exact shared protocol pair checks. Remote obsolete direct-SPI RF24 code archived, dead Remote RF24 dependency removed, Boat nRF TX 120 ms software watchdog/fail-closed transport handling added. VS Code multi-root tasks and optional local licensed LVGL font staging available. Both endpoints remain PlatformIO **compile-only**, independent physical STOP unverified; all 18 R30H-10 + 12 R30H-12 physical evidence gates unsigned. Versions Boat **014** / Remote **028** frozen. CI results must be read per last commit; target build PASS is not hardware acceptance.

> **Current R30H-10 GPIO resource note:** [Boat J1/J3 provisional pin map](../docs/BOAT_R30H_10_PIN_MAP.csv) is the test ledger. GNSS4/5, I²C7/8, nRF9/10/11/12/13/21, E22 6/14–18 remain candidate only; PWM39/40 output/driver unpowered; GPIO1/2/41/42 for future safety, PPS=-1; hardware STOP unresolved. `r30h10_gpio_test_profile.h` contains compile-time assertions only. [Source preflight](../tools/verify_r30h10_gpio_map.py) and [R30H-10 physical lock](../tools/verify_r30h10_physical_lock.py) both apply. Boat VER014, upload denied.

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

- **Active radio transport:** nRF24L01+ plus E22 UART. The retired Heltec SX1262 implementation `heltec_sx1262_link.{cpp,h}` was moved out of the live source tree to `docs/archive/2026-11-22/source_cleanup_r30h10/Boat/source/`. Neither PlatformIO nor Arduino now needs a legacy exclusion or RadioLib. The historic airtime header/tests remain for traceability. No audit locks were relaxed.
- The retired R21/R22 ADS1115 and R26 INA226 reference headers/tests remain available for historical regression only, not as active battery acquisition. R27/R28 INA231 BUS-only remains the selected (uncommissioned) path.
- Source-level inventory changes are not hardware acceptance. All R30A physical STOP, coil-current-zero, and R30H-10 RF/PPS measurements remain pending.

- The unreferenced R36/R02 candidate-only `config/boat_config.h` and `config/radio_config.h` were archived under the same R30H-10 source-cleanup archive. Current selected hardware and radio macros remain in the live `boat_radio_hw_config.h` and R20/R27/R29 documents.

## VS Code + PlatformIO R30H-10 (R2)
The `Boat/source/platformio_entry.cpp` adapter is guarded by `FLYTEC_PLATFORMIO_BUILD` and includes the nested `firmware/Flytec_Boat.ino` exactly once. PlatformIO excludes that firmware subdirectory from source discovery. R30H-10 physical safety gates, motor/solenoid power, GPS/PPS and upload remain blocked. Use `../platformio.ini` as compile-only entry.
