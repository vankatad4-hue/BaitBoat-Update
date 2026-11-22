# Flytec V080 retrofit — current continuation checkpoint

**Checkpoint:** 2026-11-22, R79/R20  
**Repository:** https://github.com/vankatad4-hue/BaitBoat-Update  
**Branch:** `hardware-ready-dual-radio-r1`

Read this file first. It supersedes the earlier Heltec/central-carrier/PCB-era continuation text. Use the active R20 and R79 documents linked below.

## Current state

- Remote source has the Boat battery voltage/estimated-percentage indicator, green/yellow/red status, Bulgarian warnings and a mission-preparation gate for stale/red battery telemetry.
- Boat source has a local 3S battery monitor with filtered voltage, low-battery mission hold, validated Home diversion, critical STOP and stale-sample STOP.
- Boat and Remote remain design/software targets, not physically commissioned. Selected modules have not been bought/assembled; do not connect actual motors or solenoids before dummy-load acceptance.
- Boat VER014 and Remote VER028 remain the current version markers. No release bump or physical acceptance is authorized by CI.

## Active authority documents

1. [Boat Hardware Selection R20](../Boat/docs/BOAT_HARDWARE_SELECTION_R20.md) — current BOM, rails, ADC and remaining hardware decisions.
2. [Boat R79 low-battery failsafe](../Boat/docs/BOAT_R79_LOW_BATTERY_FAILSAFE.md) — thresholds and mission behavior.
3. [Boat R79 battery/radio-loss dummy-load procedure](../Boat/docs/BOAT_R79_LOW_BATTERY_BENCH_TEST.md) — physical bench sequence.
4. [Boat master status](../Boat/docs/PROJECT_MASTER_STATUS.md).
5. [Remote master status](../Remote/docs/PROJECT_MASTER_STATUS.md).
6. [Project progress review](PROJECT_PROGRESS_REVIEW_R1.md).
7. [Historical archive index](archive/2026-11-22/README.md).

## Selected hardware

### Boat

- ESP32-S3-DevKitC-1-N32R16V.
- HGLRC M100 PRO GNSS with QMC5883L compass.
- GY-91 (MPU-9250 + BMP280).
- nRF24L01+ PA/LNA primary radio; EBYTE E22-900T22S backup radio.
- MCP23017 I/O expander.
- ADS1115 16-bit ADC breakout at I²C 0x48 for pack voltage (module omitted from R19, added explicitly in R20).
- Cytron MDD20A dual motor driver; EKM014 four-channel release driver.
- Olimex DCDC6-16-TO5 main 5V; TRACO TSR 2-2450 EKM014 logic 5V; 2× Pololu D24V10F3 #2830 3.3V rails.
- 3S4P 18650 pack, 11.1V nominal, ~10Ah, 20A BMS.

### Remote

- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6.
- nRF24L01+ PA/LNA primary and E22 backup radio.
- 1S4P 18650 pack, two joysticks, 16 physical button positions, USB-C.
- SD-backed offline maps and up to 16 waypoints; Bulgarian UI.
- Sonar model remains unselected/deferred.

No custom electronic PCB is in the active design. Use premade modules and removable, keyed wiring. Historical Heltec/SX1262 and central-carrier designs are archived.

## R79 low-battery policy

The Boat makes the decision locally; it does not depend on either radio.

| Pack voltage | State/action |
|---|---|
| Above 11.4V | Green |
| 10.8–11.4V | Yellow; recharge recommended |
| At or below 10.8V | Red; hold outbound mission movement |
| At or below 10.8V for 3 seconds | Request validated Home diversion for an active mission |
| At or below 10.2V for 1 second | Critical STOP |
| At or below 9.9V | Immediate critical STOP |
| ADC sample stale/invalid for >1 second | Fail-closed STOP |

- A low-battery RTH request aborts the outbound target and remaining release sequence.
- If a release pulse is active, deenergize it and wait for a newer physical release-idle/current-zero feedback sample before Home diversion.
- RTH requires matching CRC-valid Boat NVS Home, fresh GNSS/heading/geofence, healthy battery above critical and valid local STOP/release interlocks.
- During RTH all four solenoid outputs remain OFF. If any RTH gate fails, STOP; do not guess.
- If no autonomous mission is active, warn and leave manual return to the operator; do not synthesize a mission.
- Remote indicator: green above 50%, yellow 21–50%, red at 20% or lower/Boat low flag; stale data is grey/unavailable. New mission preparation is blocked on stale or red telemetry.

Voltage percentage is an estimate from a generic 3S Li-ion curve, not coulomb counting. The actual pack cell model, load sag, temperature and divider gain/offset must be validated before operation.

## R78 radio-loss policy retained

- Either authenticated radio fresh: existing link authority rules.
- Both radios stale: immediate STOP of manual motor and all solenoid outputs.
- At 10 seconds from the last accepted authenticated frame: RTH only if active eligible mission, matching CRC-valid Boat NVS Home, valid navigation and local safety gates.
- Detached RTH permits only local Home navigation; all release outputs remain OFF.
- Recovery before the threshold can resume the held eligible mission after authentication/neutral qualification. Recovery after RTH begins does not resume the old outbound route.

## Next engineering step — exact Boat GPIO/header and wiring map

Proceed with the exact ESP32-S3-DevKitC-1-N32R16V board revision pin audit and publish a pin-by-pin wiring table. Keep every pin uncommissioned until the exact purchased board schematic is verified.

Required assignments to resolve:

1. GNSS UART and GNSS reset/enable (if used) for HGLRC M100 PRO.
2. nRF24 SPI, CE, CSN, IRQ and stable power/ground.
3. E22 UART, M0, M1, AUX, reset/enable and 3.3V power.
4. Shared sensor I²C for GY-91, ADS1115 (0x48) and MCP23017, including pull-up rail and bus recovery.
5. MDD20A two-channel PWM/DIR/enable and feedback connections.
6. EKM014 IN1–IN4 and all release-idle/current-zero feedback.
7. ADC battery divider AIN0, physical STOP/watchdog contacts and remaining safety feedback.
8. Exact GPIO boot-strapping, flash/PSRAM-reserved and input-only pin exclusions.

Do not assume a pin map from a generic ESP32-S3 module pinout. Verify the exact DevKitC-1 N32R16V board revision and module schematic before marking any pin final. The next deliverable is a GPIO map plus removable-harness table, power-distribution table and uncommissioned test checklist.

## Remaining acceptance work

1. Verify exact ESP32-S3 board/header revision and finish the GPIO/harness map.
2. Purchase/assemble selected modules; calibrate ADS1115 divider against a DMM.
3. Measure all regulator rails, current, ripple and temperature under simultaneous boot/GNSS/radio activity.
4. Measure both 550 motor startup/stall current and each solenoid pulse current; calculate fuse, wire and connector ratings against the 20A BMS.
5. Verify nRF24/E22 antennas, RF settings and range in the intended EU operating configuration.
6. Run the R79 battery and R78 radio-loss dummy-load tests.
7. Run MDD20A and EKM014 dummy-load tests; verify one-at-a-time release and physical current-zero feedback.
8. Only after all bench gates pass, connect real motors/solenoids and perform controlled water acceptance.
9. Sonar remains deferred until model selection.

## Safety boundary

The R78/R79 watchdogs are software on the Boat ESP32-S3. The ADS1115 is a voltage measurement module, not an independent hardware cutoff. Software cannot protect against MCU lockup, MCU power loss or a failed power transistor. Physical STOP, wiring protection and actual feedback must be validated separately.

No release version increment until target builds, host tests, physical commissioning and acceptance records are complete.
