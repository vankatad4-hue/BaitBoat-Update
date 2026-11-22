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

Voltage percentage is an estimate from a generic 3S Li-ion curve, not coulomb counting. Calibrate the R21 corrected 150kΩ/33kΩ ADS1115 divider against a DMM and validate thresholds under actual load and temperature.

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
3. Perform the [R21 150kΩ/33kΩ ADC/power-sequencing procedure](../Boat/docs/BOAT_R21_BATTERY_ADC_COMMISSIONING.md) and enter real measurements in the [R21 worksheet](../Boat/docs/BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv); leave the sensor profile uncommissioned until all checks pass.
4. Measure the DCDC6 5V, TSR 2-2450 and both D24V10F3 rails under simultaneous ESP32 boot, GNSS acquisition and nRF24/E22 transmission.
5. Measure both 550 motor startup/stall current and each solenoid pulse current; calculate fuses, wires and connectors against the 20A BMS.
6. Resolve physical STOP/watchdog/isolator/pulse-limiter/current-zero feedback. Until then the Boat physical driver remains intentionally uncommissioned.
7. Run R79 battery and R78 radio-loss dummy-load tests, then MDD20A and EKM014 dummy-load tests.
8. Only after all bench gates pass, connect real motors/solenoids and perform controlled water acceptance.
9. Sonar remains deferred until model selection.

## Latest R79 code and CI verification update (2026-11-22)

- The battery monitor now checks **raw** 3S pack voltage for an immediate critical event before smoothing; a 12.6V → 9.8V sudden collapse is covered by a regression test.
- The Boat actuator supervisor now **deenergizes an active hopper/hook release immediately** when a running mission hits low battery, without permanently blocking Home diversion. It verifies a newer physical release-idle/current-zero sample before releasing the RTH gate; after 500ms without confirmation it latches STOP.
- The Remote red warning explicitly instructs `ЗАРЕДИ/СМЕНИ` (recharge/replace), and appends RTH **only after** the Boat reports return-to-home mode. Critical voltage displays a distinct STOP warning.
- Boat portable host tests passed with this interrupted-release fix (GitHub Actions run 45012211843). Remote portable host tests passed with the enhanced warning (run 45012139922). Both target builds require a final result check after their latest edits; neither target compile constitutes physical acceptance.
- The E22-900T22S 5V rail is the **selected typical-voltage design**, not the only voltage the manufacturer permits. Keep the 5V main regulator current/ripple check and 3.3V TTL signal limits.

## Safety boundary

R78/R79 watchdogs are software on the Boat ESP32-S3. ADS1115 is a measurement module, not an independent cutoff. Software cannot protect against MCU lockup, MCU power loss or a failed power transistor. Physical STOP, wiring protection and actual feedback must be validated separately.

No release version increment until target builds, host tests, physical commissioning and acceptance records are complete.

## R21 checkpoint — battery ADC full-charge correction

- The old R20 100kΩ/33kΩ pack divider could produce 3.126V on ADS1115 AIN0 at a healthy full-charge 12.6V pack, while `carrierBatteryMv()` rejects ADC inputs greater than 3.0V. **Do not build the old divider.**
- R21 candidate: **150kΩ top from switched/protected pack sense feed, 33kΩ bottom, 100nF filter**; ratio 5.54545. Nominal AIN0 at 12.6V is 2.272V. Firmware example uses shared `BOAT_BATTERY_DIVIDER_RATIO` and remains `commissioned=false`.
- TI input limit relative to ADC VDD remains mandatory even during power-up/down. Same master switch is not enough to prove safe sequencing. Check for ADC back-powering with a scope before sensor commissioning.
- Software now fails closed **on the first invalid ADC sample**, in addition to aging out a formerly good sample after 1000ms.
- [R21 low-voltage commissioning](../Boat/docs/BOAT_R21_BATTERY_ADC_COMMISSIONING.md) and [blank test record](../Boat/docs/BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv) are ready. R21 Boat host and ESP32-S3 **non-actuating diagnostic** compile passed on commit `bd4199585dbd0ec48b3c745aa8b35612aa78e8f4` (CI #45013257861 and #45013257742). No physical measurements have been performed. Remote host and Waveshare target compilation passed on commit `12c5cfd0b29ab641b0acc184e6160335efc6b0b3`.
- **Next gate:** purchase/inspect actual board revisions, make DMM/scope recordings for voltage and rail sequencing, verify I²C bus address, then proceed to dummy-load-only tests. Physical STOP/watchdog/current-zero feedback remains unresolved. Boat VER014 / Remote VER028 unchanged.

## R22 continuation — isolated ADS1115 bench firmware (2026-11-22)

**Committed:** independent sensor-only [R22 ADS1115 serial bench](../Boat/hardware_test/ADS1115_Battery_Bench/ADS1115_Battery_Bench.ino) with default-off audited GPIO gate; private local config example, read-only raw-to-mV decoder, portable host tests, source isolation verification, and a separate target compile stage. It does **not** import Boat mission/radio/actuator code or authorize motor, solenoid or RTH operation. No physical ADC readings have been performed.

- [R22 sensor-only hardware bring-up](../Boat/docs/BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md)
- [R21 uncommissioned 150kΩ / 33kΩ divider design](../Boat/docs/BOAT_R21_BATTERY_ADC_COMMISSIONING.md)
- [Blank voltage/DMM/oscilloscope record](../Boat/docs/BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv)
- Boat host CI passed on `97b6a54e9562c4c2fd683bb399847d946930a61e`; [run #45021864048](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45021864048).
- Main Boat and separate locked R22 bench ESP32-S3 target compilation passed on `94750bb58ea5bbb9a501a82f4a78d03cd4824138`; [run #45021889338](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45021889338).
- The first host run immediately after adding the new CI check was transiently failing because its checker file had not yet been committed; the later complete R22 host run is passing.

**Next physical gate:** purchase/inspect the exact DevKitC-1-N32R16V revision and ADS1115 carrier; verify 3.3V regulator and pullups, GPIO header, sensor ground, divider 150k/33k, and AIN0 versus VDD through power-up/down with a scope. Record DMM and serial raw ADC codes at 9.0–12.6V into the blank R21 sheet. Only then privately enable I²C bench reads. No motor/solenoid supply connected; independent STOP/watchdog/coil current-zero physical hardware remains uncommissioned. Versions Boat VER014 / Remote VER028 remain unchanged.

## R23 continuation — GPIO assignment, vendor interfaces, harnesses (2026-11-22)

**Current status:** R23 module wiring DESIGN COMPLETE (18 ESP32-S3 GPIO candidates, 64 connection-matrix nets); physical hardware UNPURCHASED/UNCOMMISSIONED; Boat VER014 and Remote VER028 unchanged. **Use [R23 wiring guide](../Boat/docs/BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md) and [R23 64-net CSV](../Boat/docs/BOAT_R23_MODULE_CONNECTION_MATRIX.csv) for proposed module harness wiring; use [R20 hardware selection](../Boat/docs/BOAT_HARDWARE_SELECTION_R20.md) for selected parts and [R21/R22](../Boat/docs/BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md) for isolated ADC bench work.**

- **Espressif J1/J3 documented header positions match:** M100 UART GPIO4 TX/J1-4 and GPIO5 RX/J1-5; shared I²C GPIO7 SCL/J1-7 GPIO8 SDA/J1-12; nRF CSN9, CE10, MOSI11, SCK12, MISO13, IRQ21; E22 NRST6, AUX14, M0 15, M1 16, TX17, RX18; MDD20A PWM1 GPIO39/J3-9 and PWM2 GPIO40/J3-8. MCP23017 GPB0/1→MDD DIR1/2, GPA0–3→EKM014 IN1–IN4. GPIO47/48 are 1.8V and unused; 35–37 reserved memory; 39/40 repurposed JTAG; GPIO1/2/41/42 reserved safety **NO WIRE**.
- **Verified vendor interfaces:** EBYTE E22-900T22S exact 26-pad UART mapping, supply 5V main but 3.3V-control; Cytron MDD20A 3.3V PWM/DIR; Electrokit EKM014 input 3.3V with logic VL 5V and separate solenoid VH≥VL; Waveshare MCP23017 A0/A1/A2 jumpers *open/high* give 0x27.
- **Physical holds:** M100 PRO 5V-powered UART TX and I²C pull-up voltages/SH1.0 pin order, E22 TXD/AUX output high at 5V, actual nRF24 PA 2×4 header, I²C pull-ups, MCP address jumpers, independent STOP/watchdog/pulse limiter/current-zero feedback and protected motor/EKM VH high-current wiring.
- **Power register corrected:** two Pololu D24V10F3 3.3V regulators #1 nRF24 and #2 GY91+ADS1115+MCP23017, E22 on 5V DCDC6-16-TO5, EKM014 VL on 5V TSR 2-2450; Ardboard nRF socket **optional zero quantity** rather than an approved voltage converter.
- **Source/tests:** `Boat/source/boat_r23_gpio_connection_contract.h`, `Boat/tests/boat_r23_gpio_contract_test.cpp` and `Boat/tools/verify_gpio_connections_r23.py`; R23 host workflow was successful on commit `a5654ff009b94277a94cac938323900720adb647`, GitHub Actions [#45032793828](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45032793828). Updated R68/R68A historical test expectations now enforce the current R23 procurement rail roles without asserting old E22/Ardboard power as active. No target output commissioning.
- **Next next action:** physical module purchase/identity and connector photos; sign actual J1/J3 positions; measure isolated 5V-powered M100 TX/SDA/SCL and E22 TXD/AUX; scan I²C modules one-at-a-time; confirm both Pololu and both 5V supplies, ADC sense 150k/33k, and physical shutdown sequencing using a current-limited source; then define approved keyed harness cable cavity maps and separately resolve independent hardware safety. **Do not energize MDD20A motor power or EKM014 VH**.

Sonar remains deferred. All GPIO/RF/safety flags must remain zero/false until physical acceptance evidence exists.

### R23 physical pin/sign-off worksheet

[BOAT_R23_GPIO_AND_LEVEL_SIGNOFF.csv](../Boat/docs/BOAT_R23_GPIO_AND_LEVEL_SIGNOFF.csv) holds the 18 GPIO continuity/logic-level checks plus 14 module/power/safety checks (all **NOT_RUN**). Use it after purchasing parts; do not mark the design as electrically verified before recording actual measurements. Source and CI remain fail-closed.

## R24 continuation — boot/reset and MCP23017 joint disabled audit (2026-11-22)

**Current software deliverable:** [R24 module-only boot/reset safety review](../Boat/docs/BOAT_R24_BOOT_RESET_OUTPUT_SAFETY.md), [read-only combined register checker](../Boat/source/mcp23017_joint_boot_audit_r24.h), [R24 host regression](../Boat/tests/boat_r24_mcp_joint_boot_test.cpp), and a Boat portable-host CI step. The new helper validates both MCP23017 banks: IOCON=0, IODIRA=0xF0, IODIRB=0xFC, selected OLAT and GPIO bits all LOW. It composes existing low-latch-first initializers, with best-effort OFF on failure. It is **not integrated into an output-authorizing HAL** and must never be used as a physical safety gate.

**Vendor evidence:** EKM014 manufacturer documents 120kΩ pull-downs on the four active-high IN pins, helpful for an undriven input but **not independent safety**. Microchip MCP23017 POR returns pins to inputs; after a reset the joint output-enabled diagnostic audit becomes false. Cytron MDD20A includes manual on-board test buttons and has no power-reversal protection. No startup PWM, rail timing or coil-output voltage has been measured.

**Unresolved blocker unchanged:** independently rated physical STOP/watchdog/load isolation/coil-current-zero and bounded one-hot pulse protection are not yet selected, wired or accepted. An I²C/OLAT readback cannot replace them. The complete R23 32-item electrical signoff remains **NOT_RUN**. Keep both 3S high-current paths physically disconnected, existing pin audit flags 0/false, Boat VER014 and Remote VER028 unchanged. R24 CI regression is configured; its outcome must be checked from the workflow run rather than assumed.

**Next engineering gate:** obtain the exact DevKitC-1, Waveshare, EKM014 and MDD20A module revisions; photograph manufacturer silks/connectors, then measure idle EKM IN1–4 and MDD PWM/DIR boot/reset behavior on logic-only current-limited supply. Verify M100/E22 output-high levels and R21 ADS1115 power sequencing. Do not commission actual actuator power without separately validated electrical safety equipment.

## R25 continuation — unpowered ADC and isolated logic-rail acceptance (2026-11-22)

**Committed R25:** [power/logic bench acceptance plan](../Boat/docs/BOAT_R25_POWER_LEVEL_PREFLIGHT.md), [28-gate unsigned electrical evidence CSV](../Boat/docs/BOAT_R25_POWER_LEVEL_EVIDENCE.csv), [fail-closed physical-evidence checker](../Boat/tools/verify_r25_preflight.py), [synthetic host regressions](../Boat/tests/test_r25_preflight.py), and Boat host CI gate. Local equivalent checker and **13** simulated regression scenarios passed before push; the new GitHub CI run result has not been independently confirmed yet.

**Critical new STOP/HOLD:** R21's 150kΩ/33kΩ ADC divider is a valid nominal ratio at *powered* 3.3V, but it does not guarantee ADS1115 AIN0 remains within its **VDD-relative** input limit during power-up/down. At 12.6V pack the unloaded divider node is ~2.272V, whereas an unpowered ADC requires no input > VDD+0.3V. Until a manufacturer-compliant sequencing/protection path has been engineered for the actual breakout, **keep the 3S sense input disconnected**. Do not subject the ADC to known over-limit conditions in a proposed test. A common master switch alone is insufficient proof.

**R25 test coverage:** manufacturer PCB/signal cavity identity, four selected logic supply rails, isolated M100/E22 logic HIGH levels, 3.3V I²C pull-ups, ADC power-up/down AIN0−VDD timing, two MDD PWM startup pins, four EKM014 startup inputs, motor/solenoid supply isolation and unresolved independent STOP/watchdog/coil-zero/pulse-limit hardware. The evidence table remains **all 28 NOT_RUN** and the R23 physical sheet **all 32 NOT_RUN**. GitHub CI is designed to PASS only when the repository does **not** claim physical sign-off; successful CI is not physical acceptance.

No active regulator, radio, pin, motor or solenoid selection changed. No custom PCB, additional module, high-current connection, signed firmware flag or release version bump. **Boat VER014 / Remote VER028**, sonar deferred. **Next:** physical exact-module purchase/revision identification, safe isolated 5V/3.3V rail measurement, M100/E22 logic-level review, then solve ADS1115 power sequencing and independent safety hardware before any dummy-load acceptance. Real motor/coil leads must remain disconnected.

## R26 continuation — ADS1115 hold and INA226 alternative (2026-11-22)

**Engineering finding:** TI's INA226 Rev C datasheet (§6.3.1) explicitly permits bus voltage to be present when VS supply is off. The dedicated VBUS pin measures 0–36V with 1.25mV/LSB, compared with R21 ADS1115 whose AIN0 is **not** approved for battery-divider voltage during VDD-off. INA226 would be a manufacturer-designed solution to the specific sequencing hazard **if** a reputable factory-built module with validated exposed VBUS and safe I²C pull-ups is selected. Do not confuse the dedicated chip pin with arbitrary preinstalled shunt terminal routing. [R26 manufacturer-backed candidate review](../Boat/docs/BOAT_R26_INA226_VOLTAGE_ONLY_REVIEW.md) and [15-row unsigned selection worksheet](../Boat/docs/BOAT_R26_CANDIDATE_SIGNOFF.csv).

**Code:** added `Boat/source/ina226_voltage_candidate_r26.h` (host-only INA226 0x02 1.25mV/LSB, FE=0x5449 and FF=0x2260/0x2261 validation, conversion-newness, stale/invalid checks) with `INA226_HARDWARE_SELECTED=false` permanently locking the candidate out of actual Boat usage. Added `Boat/tests/ina226_candidate_voltage_r26_test.cpp` for calculated 3S thresholds and R79 battery policy compatibility, plus a CI step. Local C++11 candidate test also passed with a local copy of the reviewed R79 battery-policy header; GitHub Actions verification against the exact branch remains pending. No production sensor port, GPIO or actuator driver was modified.

**Hardware choice is NOT changed:** ADS1115 0x48 with R21 150kΩ/33kΩ remains on the selected R20 list, but the pack-sense lead must stay disconnected until its sequencing problem is resolved or the owner explicitly approves an exact INA226 preassembled replacement. INA226 nominal address 0x40 is a candidate needing A0/A1 inspection, not a signed bus address. TI specifies that INA226 SCL may not exceed VS+0.3V; VS-off I²C sequencing still requires real verification. Never route 20A BMS current through the shunt on an off-the-shelf INA226 module.

**Sourcing leads only:** TI INA226EVM appears in Comet/RS Bulgaria's catalog, with zero Comet local stock observed; HESTORE INA226-M exists as an EU listing, but the module's VBUS/shunt trace and delivery have not been approved. Neither is a purchase authorization. 15 R26 candidate physical signoffs, 28 R25 and 32 R23 physical measurements remain **NOT_RUN**. Independent DC STOP/watchdog/coil-zero/one-hot pulse limiter remains unresolved. No new hardware, custom PCB, signed GPIO, actual I²C/ADC measurement, motor/solenoid energizing, release or sonar integration. **Boat VER014 / Remote VER028 unchanged.**

**Next:** review/approve exact assembled INA226 board or a manufacturer-compliant ADS1115 sense-isolation solution, verify module pin/supply topology, then low-current bench evidence. Resolve independent physical power/STOP feedback before any MDD20A/EKM014 high-current commissioning.

## R27 continuation — Ardboard INA231 voltage-only module confirmed (2026-11-22)

**OWNER-AUTHORIZED SELECTED MODULE:** Ardboard Bulgaria [INA231 I²C monitoring module product **1491**](https://www.ardboard.com/index.php?route=product/product&product_id=578), listing €5.50 VAT included and 9 available at review. **Selected for Boat battery voltage-only sensing** instead of ADS1115 R20/R21 and INA226 R26. **No purchase, electrical inspection or physical commissioning performed.** Manufacturer TI INA231 Rev D permits bus voltage independent of VS while off (0–28V), 3.3V logic supply within 2.7–5.5V and 1.25mV/LSB at BUS register 0x02. Exact board identity, BUS connection and address 0x40 remain physical acceptance gates. See [R27 selected module/wiring plan](../Boat/docs/BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md) and [R27 all-NOT_RUN 21-point worksheet](../Boat/docs/BOAT_R27_INA231_ACCEPTANCE.csv).

**Critical constraint:** Ardboard module contains **0.1Ω R100 shunt**. NEVER put full 20A battery, MDD20A or EKM014 current through it (40W at 20A; ±81.92mV ADC shunt range is only ±0.819A). Connect only protected low-current voltage-sense lead to separate BUS pad once manufacturer module/trace is physically checked, sensor-ground and Pololu D24V10F3 #2 3.3V, SDA GPIO8/SCL GPIO7, I²C 0x40 only if address straps physically verify. Do not infer header pad order from stock images. ALE not an independent safety cutoff. No additional PCB selected.

**Source update:** `Boat/source/ina231_voltage_contract_r27.h` and `Boat/tests/ina231_voltage_contract_r27_test.cpp` implement pure 1.25mV decoder, explicit config 0x4126 BUS-only continuous, synthetic R79 threshold tests and permanent `INA231_PHYSICAL_COMMISSIONED=false` gate. Unlike INA226, TI's INA231 Rev D documented register map **does not establish FE/FF manufacturer/die ID**, so these are NOT used as an authenticity test. `TrackerSensorPort::pollAdc` now fails closed and does not operate the legacy ADS1115 while INA231 is selected. The physical INA231 I²C backend is **not integrated or commissioned**; until real hardware acceptance Boat battery telemetry remains invalid/unavailable, which intentionally forbids real propulsion. R68/R68A tests and procurement sensor-rail labels updated to INA231; Boat CI regression added, exact GitHub Actions outcome not yet verified. **Boat VER014 / Remote VER028 unchanged**; no live motors/solenoids, no physical tests or sonar integration.

**NEXT:** physically obtain Ardboard #1491; photograph PCB and confirm VBUS/shunt separation, safe 3.3V I²C pullups/voltage, exact address, protected low-current sense wiring and DMM against 9.0–12.6V; implement the isolated INA231 acquisition HAL only after safety review and provide independent STOP/watchdog/coil-current-zero/pulse-limit hardware before any actuator authorization.
