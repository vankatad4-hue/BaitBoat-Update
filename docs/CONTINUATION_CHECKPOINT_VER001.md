# Flytec V080 retrofit — current continuation checkpoint

**Checkpoint:** 2026-11-22, R79/R20  
**Repository:** https://github.com/vankatad4-hue/BaitBoat-Update  
**Branch:** `hardware-ready-dual-radio-r1`

Read this file first. It supersedes the earlier Heltec/central-carrier/PCB-era continuation text. Current procurement and wiring authority is R20; current battery/radio failsafe authority is R79.

## Current state

- Boat software: local 3S battery monitor, low-battery outbound hold, validated Home diversion, critical STOP and stale-ADC STOP.
- Remote software: Boat battery voltage/estimated-percent indicator, green/yellow/red colors, Bulgarian recharge/RTH/critical STOP warnings and stale/red telemetry mission-preparation block.
- Candidate Boat GPIO/header map and full J1/J3 crosswalk are prepared. **They are not signed** because the ESP32-S3-DevKitC-1-N32R16V board has not been purchased/inspected. Firmware pin-audit flags remain 0 and the tracker sensor profile remains uncommissioned.
- Boat and Remote remain design/software targets, not physically commissioned. Keep actual motors and solenoids disconnected until dummy-load acceptance.
- Boat VER014 and Remote VER028 remain current version markers. No release bump or physical acceptance is authorized by CI.

## Active authority documents

1. [Boat Hardware Selection R20](../Boat/docs/BOAT_HARDWARE_SELECTION_R20.md) — current BOM and power rails.
2. [Boat GPIO/header map R20](../Boat/docs/BOAT_GPIO_HEADER_MAP_R20.md) — candidate J1/J3 pins, module wiring and reserved signals.
3. [R20 GPIO resource crosswalk](../Boat/docs/BOAT_R20_GPIO_RESOURCE_CROSSWALK.csv) — all 44 header positions.
4. [Boat R79 low-battery failsafe](../Boat/docs/BOAT_R79_LOW_BATTERY_FAILSAFE.md) — thresholds and mission behavior.
5. [Boat R79 battery/radio-loss dummy-load procedure](../Boat/docs/BOAT_R79_LOW_BATTERY_BENCH_TEST.md).
6. [Boat master status](../Boat/docs/PROJECT_MASTER_STATUS.md) and [Remote master status](../Remote/docs/PROJECT_MASTER_STATUS.md).
7. [Project progress review](PROJECT_PROGRESS_REVIEW_R1.md).
8. [Historical archive index](archive/2026-11-22/README.md).

## Selected hardware

### Boat

- ESP32-S3-DevKitC-1-N32R16V.
- HGLRC M100 PRO GNSS with integrated QMC5883L compass.
- GY-91 (MPU-9250 + BMP280).
- nRF24L01+ PA/LNA primary radio; EBYTE E22-900T22S backup radio.
- MCP23017 I/O expander, candidate address 0x27.
- ADS1115 16-bit ADC breakout at I²C 0x48 for pack voltage.
- Cytron MDD20A dual motor driver; EKM014 four-channel release driver.
- 3S4P 18650 pack, 11.1V nominal, ~10Ah, 20A BMS.

### Boat power allocation

- DCDC6-16-TO5 5V/2A main rail: ESP32-S3 DevKitC-1, M100 PRO and E22-900T22S VCC. E22 UART/M0/M1/AUX logic is 3.3V TTL; do not power its VCC from the 3.3V D24V10F3 rail if rated RF output power is required.
- TSR 2-2450 5V: EKM014 VL logic only.
- D24V10F3 #1 3.3V: nRF24 PA/LNA.
- D24V10F3 #2 3.3V: GY-91, ADS1115 and MCP23017.
- 3S battery through separate fused paths: MDD20A motor power and EKM014 VH.
- Do not parallel regulator outputs. Measure current, ripple and temperature before commissioning.

### Remote

- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6.
- nRF24L01+ PA/LNA primary and E22 backup radio.
- 1S4P 18650 pack, two joysticks, 16 physical button positions, USB-C.
- SD-backed offline maps and up to 16 waypoints; Bulgarian UI.
- Sonar model remains unselected/deferred.

No custom electronic PCB is in the active design. Use premade modules and removable, keyed wiring. Historical Heltec/SX1262 and central-carrier designs are archived.

## R79 low-battery policy

The Boat makes the decision locally; neither radio is required.

| Pack voltage | State/action |
|---|---|
| Above 11.4V | Green |
| 10.8–11.4V | Yellow; recharge recommended |
| At or below 10.8V | Red; hold outbound mission movement |
| At or below 10.8V for 3 seconds | Request validated Home diversion for an active mission |
| At or below 10.2V for 1 second | Critical STOP |
| At or below 9.9V | Immediate critical STOP |
| ADC sample stale/invalid for >1 second | Fail-closed STOP |

- Low-battery RTH aborts the outbound target and remaining release sequence.
- If a release pulse is active, deenergize it and wait for a newer physical release-idle/current-zero feedback sample before Home diversion. If the feedback is unavailable or fails, remain STOP.
- RTH requires matching CRC-valid Boat NVS Home, fresh GNSS/heading/geofence, battery above critical and valid local STOP/release interlocks.
- During RTH all four solenoid outputs remain OFF. Critical STOP overrides RTH and stops all outputs.
- If no autonomous mission is active, warn and leave manual return to the operator; do not synthesize a mission.
- Remote indicator: green above 50%, yellow 21–50%, red at 20% or lower/Boat low flag; stale data is grey/unavailable. Critical battery has a separate `КРИТ. БАТ. — STOP` warning. New mission preparation is blocked on stale/red/critical telemetry.
- Status bit 5 indicates low-battery/RTH; bit 7 indicates critical STOP.

Voltage percentage is an estimate from a generic 3S Li-ion curve, not coulomb counting. Calibrate the 100kΩ/33kΩ ADS1115 divider against a DMM and validate thresholds under actual load and temperature.

## R78 radio-loss policy retained

- Either authenticated radio fresh: existing link authority rules.
- Both radios stale: immediate STOP of manual motor and all solenoid outputs.
- At 10 seconds from the last accepted authenticated frame: RTH only if active eligible mission, matching CRC-valid Boat NVS Home, valid navigation and local safety gates.
- Detached RTH permits only local Home navigation; all release outputs remain OFF.
- Recovery before threshold can resume the held eligible mission after authentication/neutral qualification. Recovery after RTH begins does not resume the old outbound route.

## R20 candidate GPIO/header map — prepared, sign-off pending

The candidate map is in [BOAT_GPIO_HEADER_MAP_R20.md](../Boat/docs/BOAT_GPIO_HEADER_MAP_R20.md) and the complete J1/J3 crosswalk CSV.

| Function | Candidate |
|---|---|
| M100 PRO UART | GPIO4 TX / GPIO5 RX |
| Shared I²C | GPIO7 SCL / GPIO8 SDA |
| nRF24 SPI | GPIO9 CSN, GPIO10 CE, GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO, GPIO21 IRQ |
| E22 UART/control | GPIO17 TX, GPIO18 RX, GPIO15 M0, GPIO16 M1, GPIO14 AUX, GPIO6 NRST |
| MDD20A PWM | GPIO39 PWM1, GPIO40 PWM2 |
| MDD20A DIR | MCP23017 0x27 GPB0/GPB1 |
| EKM014 IN1–IN4 | MCP23017 0x27 GPA0–GPA3 |

The GPIO map reserves boot straps, USB, UART0, N32R16V memory pins and revision-dependent RGB LED pins. Candidate macros are in `boat_radio_hw_config.h`; `BOAT_NRF_PIN_AUDIT_SIGNED=0`, `BOAT_E22_UART_PIN_AUDIT_SIGNED=0`, and `trackerLocalProfile.pinAuditSigned=false`. Do not enable hardware profiles until the exact board revision and purchased module pinouts are inspected.

### Important unresolved physical safety gate

The current `BoatPhysicalDriverAdapter` refuses to initialize unless the independent STOP/watchdog contacts, motor/coil isolator feedback, independent EKM014 pulse-limiter ready signal and physical coil-current-zero feedback are commissioned. The current R20 BOM does not yet select all hardware that provides those signals. MCP23017 GPIO readback is not coil-current feedback. Do not bypass this gate. Determine whether the original Flytec board's safety contacts/feedback can be reused; otherwise a separate hardware decision is required before actual motor/solenoid outputs can be enabled.

## Next engineering steps

1. Purchase/inspect the exact ESP32-S3-DevKitC-1-N32R16V revision; verify J1/J3 numbering and sign the candidate GPIO map.
2. Verify exact E22-900T22S suffix/pad layout and use a premade carrier; verify M100 PRO connector pin labels and MCP23017 address straps.
3. Calibrate ADS1115 divider against a DMM; enable the sensor profile only after gain/offset and freshness checks pass.
4. Measure the DCDC6 5V, TSR 2-2450 and both D24V10F3 rails under simultaneous ESP32 boot, GNSS acquisition and nRF24/E22 transmission.
5. Measure both 550 motor startup/stall current and each solenoid pulse current; calculate fuses, wires and connectors against the 20A BMS.
6. Resolve physical STOP/watchdog/isolator/pulse-limiter/current-zero feedback. Until then the Boat physical driver remains intentionally uncommissioned.
7. Run R79 battery and R78 radio-loss dummy-load tests, then MDD20A and EKM014 dummy-load tests.
8. Only after all bench gates pass, connect real motors/solenoids and perform controlled water acceptance.
9. Sonar remains deferred until model selection.

## Safety boundary

R78/R79 watchdogs are software on the Boat ESP32-S3. ADS1115 is a measurement module, not an independent cutoff. Software cannot protect against MCU lockup, MCU power loss or a failed power transistor. Physical STOP, wiring protection and actual feedback must be validated separately.

No release version increment until target builds, host tests, physical commissioning and acceptance records are complete.
