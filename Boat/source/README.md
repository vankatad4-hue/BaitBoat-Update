# Boat source — current integration notes

Active Boat hardware decisions are in [BOAT_HARDWARE_SELECTION_R20.md](../docs/BOAT_HARDWARE_SELECTION_R20.md). Older source inventory is archived at [Boat_source_README_pre_R79.md](../../docs/archive/2026-11-22/Boat_source_README_pre_R79.md).

## Key modules

- `boat_application_port.*`: production application bridge, hardware sample ingestion, safety supervision, telemetry publication.
- `boat_battery_policy.h`: filtered 3S pack estimate, green/yellow/red state, low-battery RTH latch and critical STOP.
- `boat_dual_radio_port.*`: authenticated nRF24/E22 session management, radio-loss timer, Home NVS and local low-battery diversion.
- `boat_home_point_nvs.*`: two-slot CRC-verified Boat-local Home persistence.
- `mission_execution.h` / `mission_controller.h`: mission state, outbound hold on low battery, Home diversion and no-radio Home navigation.
- `boat_hardware_supervisor.h`: motor/release output policy, one-at-a-time releases, low-battery outbound inhibit and Home-only RTH gate.
- `tracker_sensor_port.*`: GNSS/IMU and ADS1115-compatible battery-voltage acquisition. Commissioned profile must remain disabled until exact pins and divider calibration are verified.

## R79 battery behavior

The Boat computes battery state locally from a fresh voltage sample. Red battery holds outbound mission movement; after 3 seconds the application requests Home diversion for an eligible active mission. Critical battery or stale ADC data causes STOP. RTH is blocked if Home, navigation, battery or local release/STOP interlocks are invalid. Release outputs remain OFF during RTH.

## Hardware boundary

The current firmware target is a diagnostic/non-actuating build. The [R20 GPIO/header map](../docs/BOAT_GPIO_HEADER_MAP_R20.md) is a candidate only; pin-audit flags remain false. The ADS1115 sample now supplies the battery-health validity used by the physical-driver gate. Independent STOP/watchdog/pulse-limiter/current-zero feedback and physical driver bindings still require sign-off. Host tests and target compilation do not commission actual ADC, GNSS, radio, MDD20A or EKM014 hardware.

## R22 read-only ADS1115 module

- `boat_ads1115_bench_math.h`: portable raw conversion to a plausibility-tagged, explicitly **uncalibrated** pack-voltage estimate; no Boat health or mission authorization.
- `../hardware_test/ADS1115_Battery_Bench/ADS1115_Battery_Bench.ino`: dedicated USB-serial ADC sketch, locked by default and devoid of radio/motor/solenoid control.
- `../docs/BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md`: pin audit, 3.3V supply, ADS1115 input, serial capture and physical evidence procedure.

The production Boat firmware is unchanged by this diagnostic sketch and remains non-actuating.
