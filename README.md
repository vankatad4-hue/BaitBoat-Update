# Flytec V080 modular bait-boat retrofit

Active development branch: `hardware-ready-dual-radio-r1`

This repository contains the Boat controller, handheld Remote, firmware, hardware decisions, tests and commissioning procedures for the Flytec V080 retrofit.

## Current hardware authority

- **Boat:** [Hardware Selection R20](Boat/docs/BOAT_HARDWARE_SELECTION_R20.md)
- **Remote:** [Remote project master status](Remote/docs/PROJECT_MASTER_STATUS.md)
- **Battery/RTH behavior:** [R79 low-battery failsafe](Boat/docs/BOAT_R79_LOW_BATTERY_FAILSAFE.md)
- **Bench procedure:** [R79 battery and radio-loss dummy-load test](Boat/docs/BOAT_R79_LOW_BATTERY_BENCH_TEST.md)
- **Continuation checkpoint:** [CONTINUATION_CHECKPOINT_VER001.md](docs/CONTINUATION_CHECKPOINT_VER001.md)
- **Progress review:** [PROJECT_PROGRESS_REVIEW_R1.md](docs/PROJECT_PROGRESS_REVIEW_R1.md)
- **Historical archive:** [docs/archive/2026-11-22](docs/archive/2026-11-22/README.md)

Older Heltec Wireless Tracker/SX1262 and central-carrier-PCB documents are archived. The active design uses ready-made modules and removable wiring; no custom PCB is required.

## Boat hardware baseline

| Function | Active selection |
|---|---|
| Main controller | ESP32-S3-DevKitC-1-N32R16V |
| GNSS and compass | HGLRC M100 PRO with QMC5883L |
| IMU/barometer | GY-91 (MPU-9250 + BMP280) |
| Primary radio | nRF24L01+ PA/LNA |
| Backup radio | EBYTE E22-900T22S |
| I/O expansion | MCP23017 |
| Battery voltage ADC | ADS1115 module, I²C address 0x48, 3.3V |
| Motor driver | Cytron MDD20A dual channel |
| Four-channel release driver | EKM014 |
| Main 5V rail | Olimex DCDC6-16-TO5 |
| EKM014 logic 5V | TRACO TSR 2-2450 |
| Sensor/RF 3.3V rails | 2× Pololu D24V10F3 #2830 |
| Boat battery | 3S4P 18650 Li-ion, 11.1V nominal, ~10Ah, 20A BMS |

Battery sensing uses a premade ADS1115 breakout and the **R21 corrected 150kΩ/33kΩ 1% divider** with 100nF input filtering. The 100kΩ/33kΩ R20 candidate failed the full-charge ADC input sanity limit. Check unpowered input/power sequencing and calibrate against a DMM; see [R21 sensor commissioning](Boat/docs/BOAT_R21_BATTERY_ADC_COMMISSIONING.md).

## Remote hardware baseline

- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6.
- nRF24L01+ PA/LNA primary link and E22 backup link.
- 1S4P 18650 remote battery pack.
- SD-backed offline maps and up to 16 saved waypoints.
- Bulgarian operating interface and joystick calibration.

The Remote shows the **Boat** battery voltage and estimated charge separately from the Remote's own battery. Green means OK, yellow means mid-level/recharge recommended, red means low. Stale or missing Boat battery telemetry is shown as unavailable, never as a fabricated percentage.

## R79 low-battery behavior

The Boat measures its own 3S pack locally; radio connectivity is not required for the decision.

- Above 11.4V: green.
- 10.8–11.4V: yellow.
- At or below 10.8V for 3 seconds: red; hold outbound mission motion and initiate validated Home diversion.
- At or below 10.2V for 1 second: critical STOP.
- At or below 9.9V: immediate critical STOP.
- **An invalid sample stops propulsion immediately**; a previously valid sample older than one second also fails closed.
- If an autonomous mission is active, low battery cancels the outbound target and remaining release sequence, then returns Home only with valid Boat NVS Home, navigation and local safety interlocks.
- If no autonomous mission is active, the Remote warns and manual return remains the operator's responsibility. A new mission is blocked at red.
- All hopper and hook outputs remain OFF during battery RTH. Critical battery overrides RTH and stops all outputs.

The voltage-to-percentage curve is an estimate, not coulomb counting. Thresholds are commissioning defaults and must be validated with the actual cell model, pack under load and calibrated divider before use on water.

## Radio-loss behavior

Both radio links lost: immediate STOP. After 10 seconds from the last accepted authenticated frame, local RTH is permitted only if an active eligible mission, matching CRC-valid Boat NVS Home, valid navigation and physical safety interlocks are available. Only Home navigation is allowed without radio; solenoids stay OFF.

## Software and hardware acceptance

Host tests and target builds are being run by GitHub Actions. A successful compile or host simulation does not prove physical sensor calibration, radio range, power integrity, motor-driver behavior or solenoid feedback.

The selected parts are not yet physically commissioned. The candidate GPIO map is documented, but all pin-audit flags remain unsigned until the purchased board revision is checked. Actual motors and solenoids must remain disconnected until the dummy-load procedures pass. Boat VER014 and Remote VER028 remain the current version markers; no physical acceptance or release bump is implied.

Sonar selection remains deferred.
