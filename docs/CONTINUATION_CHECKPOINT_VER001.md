# Flytec V080 retrofit — current continuation checkpoint

**Checkpoint:** 2026-11-22, **R30A design-completion handoff** (R79 safety policy, R27 INA231, R29 common 3S bus)  
**Repository:** https://github.com/vankatad4-hue/BaitBoat-Update  
**Branch:** `hardware-ready-dual-radio-r1`

Read this file first. It supersedes the earlier Heltec/central-carrier/PCB-era continuation text. Current hardware baseline is R20, superseded by R27 INA231 battery sensing and R29 one-common-3S-load-bus wiring; battery/radio failsafe authority remains R79.

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
- **Ardboard INA231 product 1491**, I²C 0x40 candidate, for protected voltage-only BUS sensing; ADS1115 retired.
- Cytron MDD20A dual motor driver; EKM014 four-channel release driver.
- 3S4P 18650 pack, 11.1V nominal, ~10Ah, 20A BMS.

### Boat power allocation

- DCDC6-16-TO5 5V/2A main rail: ESP32-S3 DevKitC-1, M100 PRO and E22-900T22S VCC. E22 UART/M0/M1/AUX logic is 3.3V TTL; do not power its VCC from the 3.3V D24V10F3 rail if rated RF output power is required.
- TSR 2-2450 5V: EKM014 VL logic only.
- D24V10F3 #1 3.3V: nRF24 PA/LNA.
- D24V10F3 #2 3.3V: GY-91, INA231 and MCP23017.
- **ONE common protected 3S battery bus** feeds both MDD20A VM and EKM014 VH in parallel; wire/fuse/physical safe-STOP acceptance NOT RUN. No separately switched motor or coil power domain.
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

Voltage percentage is an estimate from a generic 3S Li-ion curve, not coulomb counting. Calibrate the R27 selected INA231 BUS-only module against a DMM and validate thresholds under load and temperature; R21 ADS1115 divider is historical.

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

### R27 replacement wiring matrix and consistency follow-up

Added [10-net R27 INA231 battery harness override](../Boat/docs/BOAT_R27_INA231_HARNESS_OVERRIDE.csv) to supersede historic R23 `H03_ADS` without rewriting the original 64-net audit evidence; all 10 candidate connections are unwired or explicitly NC. The [R27 static check](../Boat/tools/verify_r27_battery_selection.py) is included in Boat CI, asserting purchase selection, old ADS fail-closed polling, shunt exclusion, 3.3V/I²C candidate pins and 21 NOT_RUN physical acceptance records. CI outcome is not independently confirmed. No active motor/solenoid wiring has changed.

## R28 continuation — INA231 triggered acquisition integrated but electrically locked (2026-11-22)

**Scope:** Continued software integration with the already-selected premade Boat modules, focusing on INA231 voltage acquisition and simulated fault testing without touching motor/coil power. Added [R28 logic-only integration and fault report](../Boat/docs/BOAT_R28_INA231_LOGIC_SIMULATION.md), `Boat/source/ina231_acquisition_r28.h`, `Boat/source/ina231_arduino_wire_r28.h`, `Boat/tests/ina231_acquisition_faults_r28_test.cpp`, `Boat/tests/ina231_wire_r28_test.cpp`, `Boat/tests/r28_stubs/Wire.h`, and static lock verifier `Boat/tools/verify_r28_prototype_gate.py`. Both portable C++11 test executables compiled and passed **locally** using a local copy of reviewed headers, with `-Wall -Wextra -Wpedantic -Werror`. GitHub Boat workflow has a new test step, but the exact-source Actions result must still be independently checked before claiming a green CI run.

**Design improvement versus R27:** INA231 bus-only *triggered* mode `0x4122` (MODE=010) is used for each new sample, rather than merely reading the R27 continuous-mode `0x4126` register. R28 waits at least 2ms and checks mask register 0x06 CVRF bit 3, overflow bit 2, new BUS register 0x02, big-endian 1.25mV/LSB, range 9000–13000 mV and a hard 40ms conversion timeout. A cached register cannot refresh the time stamp; failed I²C, config mismatch, overrange and old samples fail closed. The existing R79 battery monitor is used in host simulation to confirm immediate critical STOP behavior at 9.9V, without claiming live sensors or actuators.

**Integration:** `TrackerSensorPort` now includes INA231 acquisition source and TwoWire adapter, and can propagate real sample voltage/age only after all three independent checks: global `INA231_PHYSICAL_COMMISSIONED`, local `pinAuditSigned`, and both `ina231BoardAndWiringSigned` and `ina231DmmCalibrationSigned`. All are explicitly **false** in the current project. The R27 old ADS1115 code remains inactive; Boat battery telemetry is **invalid/unavailable**, and the Boat physical output driver still requires separately commissioned independent hardware cutoff/feedback. No hardware selected/purchased/energized or release flags set.

**Verification boundary:** R27 INA231 21, R25 power 28 and R23 GPIO 32 physical acceptance checks all remain `NOT_RUN`; the R100 shunt cannot be placed in the 20A load path, and actual INA231 BUS/I²C pinout needs board inspection. Remote displays missing Boat battery as unavailable, never a guessed percentage. **Versions frozen: Boat VER014, Remote VER028.** Sonar deferred; independent rated STOP/watchdog/current-zero/one-hot-coil pulse limiter still required before actuator power.

**Next:** verify target compilation of the updated Boat sensor port and the GitHub workflow's R28 job; correct any build issues, then extend non-actuating radio+sensor+stop scenario tests and detailed physical bench steps. Actual motor, solenoid, telemetry, RF range and water acceptance remain pending.

## R29 continuation — user-selected common 3S motor/solenoid bus (2026-11-22)

**Explicit owner decision:** one common high-current 3S4P pack/distribution bus for BOTH Cytron MDD20A `VM` and Electrokit EKM014 `VH`; **NO** physically separate motor-vs-solenoid battery, isolation domain, independently switched actuator supply or custom PCB. The EKM014 VL 5V regulator and other 5V/3.3V **logic** regulators stay selected, powered from the same 3S battery. The power-path choice does NOT establish a real cutoff or prevent a driver stuck ON. Updated authority: [R29 shared bus / integrated faults](../Boat/docs/BOAT_R29_SHARED_BUS_FAULT_SIMULATION.md) and [eight unconnected common-bus nets](../Boat/docs/BOAT_R29_SHARED_3S_BUS_CONNECTIONS.csv), superseding only the R23 separately gated motor/coil `VM/VH` power paths. Other 18 GPIO candidates, sensor/radio harness and regulator selection remain.

**R29 software:** added `Boat/source/boat_r29_shared_bus_contract.h` (single 3S pack; `SEPARATE_MOTOR_SOLENOID_POWER=false`, `SHARED_BUS_PHYSICALLY_ACCEPTED=false`, pure software motor-vs-one-coil exclusivity, synthetic 20A BMS current budget screening). `BoatPhysicalDriverAdapter::apply` now rejects combined motor plus coil commands *before* any HAL callback; independent physical hardware remains uncommissioned and no output enabling was added. `Boat/tests/boat_r29_shared_bus_integration_test.cpp` combines actual R78 radio watchdog/timed RTH, R79 battery monitor, BoatActuatorSupervisor and R28 INA231 synthetic I²C sensor, exercising full link loss immediate STOP, backup-only nonmanual, 10-second conditional Home, stale safety data, invalid navigation, low voltage, critical STOP, invalid ADC, invalid merged load, coil/motor exclusivity and permanent hardware gate. Boat Actions CI compiles that test and runs `Boat/tools/verify_r29_shared_bus.py` to assert no shared bus wires actually connected and all 81 R23/R25/R27 physical checks NOT_RUN. **New CI result still requires independent verification**, no physical RF/I²C/actuator pass claimed.

**Current blockers, precisely stated:** all high-current conductor sizes, common input fuse, pack/BMS stall capability, motor and 6A coil pulsing current, ground return topology and startup transients still unmeasured; a 20A BMS is NOT an evaluated safety fuse. Existing physical driver requests distinct independent feedback contacts that are NOT provided by the shared-bus proposal; these remain false/unmet, not fabricated. INA231 remains `INA231_PHYSICAL_COMMISSIONED=false`, so live battery samples and actuator permission remain invalid. Boat VER014 / Remote VER028 frozen; sonar deferred.

**Next:** verify R29 host+target CI, fix build failures, then further non-actuating integration tests. For eventual physical acceptance, a single common DC-rated master disconnection could be evaluated separately if approved, but no new safety part was selected or ordered by R29.

### R29 final cross-module release and BOM consistency

Extended the R29 scenario to cover a manual EKM release pulse, simultaneous second-release denial and both-radio loss **during** a pulse: the software stops/rejects further release and latches STOP. Backup-only radio cannot start a fresh release. Corrected the active purchase register to remove the outdated phrase "VH remains separate 3S pack"; EKM VH and MDD VM are now explicitly fed from a single 3S bus. No real coil, motor, BMS current or independent STOP hardware has been measured. GitHub Actions R29 integrated scenario result remains unverified.

### R29A verified host and ESP32-S3 target results (2026-11-22)

**Observed actual GitHub Actions, not inferred from commits:**
- [Boat host run #45040175603](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45040175603), commit `ae230dd512c8b35195cc6031b6e71e7dffcbce49`: **SUCCESS**. R28 triggered INA231 sensor simulation, R29 integrated 3S shared-bus + radio/battery/mission/actuator sensor fault scenarios, R66 independent-safety review, R68/R68A power tests, R73 integrated compass and R75 regulator suite all passed. This is **host simulation and static audits only**.
- [Boat target compile run #45038989512](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45038989512), last Boat source-relevant commit `b9964105e41a34fa92433c33f2519bb8664677f1`: **SUCCESS**, selected ESP32-S3 DevKitC-1 diagnostic build with no actuating outputs. Firmware 408681/3145728 bytes flash, 21160/327680 global RAM. The historical, locked ADS1115-only bench sketch also compiled 302617 bytes flash / 18432 bytes global RAM. Since `b9964105`, changes relevant to R29 were to **tests, tools and documents**, not Boat source, so the successful target compile covers current executable Boat source.
- Earlier host runs #45039482082 (R29 C++ simulation **PASS**, Python checker FAIL) and #45040073182 (**FAIL**) identified a **checker-schema mistake**: R23 worksheet uses `Test_Status`, not R25/R27 `Status`, and its 14 `MODULE_INTERLOCK` rows intentionally leave `Continuity_Test` blank; the 18 `GPIO_PIN` rows require `NOT_RUN`. Corrected `Boat/tools/verify_r29_shared_bus.py` to accept **only** those documented N/A cells while requiring all R23 power-on, power-off and overall statuses to remain `NOT_RUN`. Fixed by commit `ae230dd5` and verified in successful full host workflow.

**Software verification reached for R29. Physical status unchanged:** R23 32, R25 28 and R27 INA231 21 checks are still entirely `NOT_RUN`; no high-current shared 3S wiring installed, no real sensors/radios tested, no independent physical cutoff/watchdog, no onboard key provisioning or water trial. The current Boat sensor module remains INA231 #1491, not ADS1115, and its physical commissioning is `false`. The one 3S pack powers VM and VH in parallel when eventually accepted, **not separately switched motor/solenoid power domains**. Boat VER014 / Remote VER028 unchanged.

**Next prototype-development priority:** add a non-actuating, source-tested shared-power system scenario that confirms release-rail brownout and battery sampling do not grant propulsion/RTH when sensor freshness or physical safety is invalid. Following that, stage a locked INA231-specific bench sketch only after actual module pin labels and 3.3V levels have been inspected. Do not enable outputs on CI evidence alone.

## R30A — Boat prototype assembly-design handoff and readiness gates (2026-11-22)

**User milestone:** "Complete Boat prototype design, documented and ready for assembly and physical testing." R30A produces a consolidated **engineering handoff** but deliberately does **NOT** claim a complete power-safe/commissioned vehicle while key electrical parts and physical tests remain unresolved.

- New **[R30A Boat assembly handoff](../Boat/docs/BOAT_R30A_PROTOTYPE_ASSEMBLY_HANDOFF.md)** gives current selected hardware, signals, common 3S distribution, regulated logic supplies, staged unpowered/logic-only assembly and clear actuator-energization hold.
- New **[73-row effective assembly netlist](../Boat/docs/BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv)**: **64 UNWIRED_PENDING, 7 NO_WIRE, 2 REFERENCE_ONLY**. Reconciles all **64** historical R23 rows by deleting **7** retired ADS1115/divider wires and **2** superseded VM/VH power feeds, then adds **10** R27 INA231 entries and **8** R29 common-bus entries. R29 INA231 BUS sense and EKM014 VL lines are **reference-only aliases**, not second conductors. Both motors and release solenoids use **one shared 3S4P pack bus**, never independently switched battery feeds.
- New **[24 open acceptance gates](../Boat/docs/BOAT_R30A_READINESS_GATES.csv)**: safety STOP/common DC protection, stuck-driver mitigation, one-hot bounded pulses and current zero, module pin levels, battery/BMS/stall currents, secure RF pairing, physical navigation, full physical application HAL, terminal ratings and waterproofing. **All 24 remain OPEN**, while the previous **32 R23 + 28 R25 + 21 R27 = 81 physical checks** remain NOT_RUN. Nothing physically assembled or measured.
- New **[17-subsystem Boat software integration inventory](../Boat/docs/BOAT_R30A_SOFTWARE_INTEGRATION_MATRIX.csv)** maps every reviewed subsystem to source and exact runtime readiness. **Actual `Flytec_Boat.ino` remains a non-actuating diagnostic**: the full live `BoatPhysicalDriver` is not attached. Source modules and their comments are documented; no misleading assertion that the full production application or source-comment audit is complete.
- New default-locked **[R30A INA231-only Arduino bench diagnostic](../Boat/hardware_test/INA231_Battery_Bench/INA231_Battery_Bench.ino)**, with no radio/actuator interfaces, and added ESP32-S3 target CI stage. `INA231_PHYSICAL_COMMISSIONED=false` plus unsigned local board/pin/DMM flags keep Wire inactive in checked-in firmware. Historic R22 ADS1115 bench remains compiled only as historical compatibility, **not** an active selected battery sensor.
- New `Boat/tools/verify_r30a_assembly_package.py` plus `Boat/tests/test_r30a_assembly_package.py` host CI prevent old ADS, split battery bus, duplicated conductors, R100 shunt-series wiring, altered signoffs, undocumented firmware wiring and missing source references from being labeled assembly authority.
- Updated the Boat README, root README and R23/R20 historical warnings so installers use the effective R30A schedule instead of previous conflicting versions. This is **COTS modules and removable harnesses only**, no custom PCB.
- **Verification:** R30A package host checks in GitHub runs [45042341438](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042341438) and [45042485790](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042485790) PASSED; final combined workflow run after latest trigger change must be checked separately. [R30A ESP32-S3 target workflow 45042370480](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042370480) compiled the INA231-only sketch successfully; final artifact/upload job status must be observed before claiming overall success.
- **Release status:** Boat VER014 / Remote VER028 remain frozen. R30A is ready as a **design review, procurement and unpowered preparation package**, not ready for actuator-power connection, fully operating Boat prototype, dummy-load acceptance or water use. Sonar explicitly deferred.

**Next:** verify final latest-source host and locked-target CI, then close physical component/connector/protection choices using real module receipts and measured evidence. Do not bypass unsigned interlocks to claim completion.

## R30A observed full CI verification — 2026-11-22

- [**Boat host Actions #45042597229**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042597229), head `308fafa10a1d506e4ae42179e2c563608a51ae38`: **SUCCESS**. The R29 shared-3S-fault C++17 integration, R30A 73-effective-net/24-gate package validator and its seven hostile-mutation cases, and all downstream Boat host steps completed successfully. The successful R30A package checker confirms **17 software source references**, **64 unwired + 7 NC + 2 aliases**, and **81 physical NOT_RUN** lines; no real wire or safety component was tested.
- [**Boat ESP32-S3 target Actions #45042370480**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042370480), head `36b5a8afe7343a744584a62f1e3c2a9441c51c2d`: **SUCCESS**. Compiled diagnostic Boat firmware **408681 bytes program / 21160 bytes global RAM**; historical locked R22 ADS bench **302617 bytes / 18432 bytes**; new **selected locked R30A INA231 bench 302553 bytes / 18440 bytes**. Firmware binaries and build logs uploaded as GitHub Actions artifacts.
- Both runs are **source compilation and document/fault simulation evidence only**. The new INA231-only binary has `INA231_PHYSICAL_COMMISSIONED=false`, does not configure I2C pins in default build, and has no motor/release API. Hardware application still has no `BoatPhysicalDriver` attached.
- This verifies the R30A design handoff package **for unpowered/inspection-only prototype preparation and future supervised logic-only bench**, not for motor/solenoid high-current connection or water use. All 24 R30A physical gates OPEN, old 81 physical checks NOT_RUN. No new regulator, breaker/contactor, BMS or fuse SKU selected and no physical electrical acceptance performed. Boat VER014 / Remote VER028 unchanged.

## R30B — Shared 3S power brownout and local sensor fault closure (2026-11-22)

**Completed in source:** `Boat/source/boat_shared_supply_fault_guard_r30b.h` adds a fail-closed local 3S voltage-freshness/critical evidence check. It is now called directly in `BoatApplicationPort::tick` after R79 battery monitoring, before the radio/mission/actuator decision. Invalid physical electrical health, invalid/implausible INA231 reading, stale/future timestamp or critical pack state sets `sample_.batteryHealthy=false` and triggers the already-existing STOP/disable path. Yellow/red battery alone does not create an RTH mission: the authenticated mission/Home/navigation gates remain authoritative.

**Host tests:** [run 45043402159](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043402159) **SUCCESS**; C++17 R30B battery/dual-radio/conditional-Home/freeze-gate test and source lock audit passed, together with R29/R30A regressions. Follow-on run [45043490071](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043490071) also passed after correcting an obsolete ADS1115 comment in the physical driver to INA231. Full [R30B fault test report](../Boat/docs/BOAT_R30B_SHARED_SUPPLY_FAULT_SIMULATION.md).

**ESP32-S3 target compile:** latest target build is tracked separately in GitHub Actions; earlier compile runs for the latest code were cancelled by subsequent source changes. Verify final completed run/conclusion before calling the target update PASS. These are non-actuating Boat diagnostics, not commissioned operating firmware.

**No hardware authority change:** single shared 3S4P battery pack powers MDD20A VM and EKM014 VH in parallel when eventually approved. No second battery, isolated motor/coil rails or new hardware selected. Physical independent STOP, conductor/fuse/BMS measured-current sizing, release pulse feedback and full physical HAL remain unresolved. R30A 24 gates OPEN; R23/R25/R27 81 physical checks NOT_RUN; INA231_PHYSICAL_COMMISSIONED remains false. Boat VER014 / Remote VER028 frozen. Sonar deferred.

**Next:** reconcile single-common-bus STOP/feedback expectations with the existing physical-driver contact gate on paper (do not invent contacts), then continue the target-build audit and testable sensor/radio fault scenarios while awaiting actual modules.

## R30B final verified CI (2026-11-22)

- [Boat ESP32-S3 target #45043490121](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043490121), source commit `7f5e2e11222c2b859113b5a4b8e610a5cd8ac016`: **SUCCESS**. Full non-actuating `Flytec_Boat.ino` diagnostic, historical locked ADS1115 bench and selected locked R30A INA231 bench all compiled; upload-artifact steps successful. No live motor/solenoid outputs.
- [Boat host #45043597807](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043597807), documentation commit `eb461d416db71fb2ca1d258553cf4adc7747791d`: **SUCCESS**, including R29 shared-bus fault simulation, R30A 73-net/24-readiness-gate consistency and R30B brownout/stale voltage tests + source-guard assertions.
- No Boat executable source changed after `7f5e2e` before these reviews. Firmware physical commissioning remains **DISABLED**, R30A 24 hardware gates OPEN and R23/R25/R27 physical tests **81 NOT_RUN**. These successes establish reproducible **software/locked-target readiness**, not proof of a safely energizable Boat or a working RF/autopilot/actuator assembly.

## R30C — one shared 3S bus, independent physical feedback contract (2026-11-22)

**No new battery or separately switched actuator supply.** Cytron MDD20A VM and EKM014 VH remain parallel on ONE 3S4P distribution. The current legacy R48/R61 `BoatSafetyContacts`/`BoatPhysicalDriverAdapter` requires **two physical contactor feedback inputs** (`motorContactorFeedbackClosed`, `coilContactorFeedbackClosed`), which the selected shared-bus hardware **does not provide**. Do not duplicate one bit, force logic read-back, or fabricate contacts to pass old tests.

**R30C source:** Added [common bus feedback contract](../Boat/source/boat_common_bus_feedback_r30c.h) and [engineering crosswalk](../Boat/docs/BOAT_R30C_ONE_BUS_SAFETY_FEEDBACK_CONTRACT.md). This prospective, transport-neutral interface models ONE common DC-rated load-disconnect feedback, real hardwired STOP status, independent watchdog, physically verified current-zero for both motor and all coils, and one-hot bounded coil pulse limiter. These are **separate safety functions on the same electrical bus**, not separate actuator batteries. Functions require fresh local evidence, 100ms max output lease, at most 500ms coil pulse, no simultaneously commanded motor/solenoid, and motor current zero before release. The new `COMMON_BUS_HARDWARE_COMMISSIONED=false` flag makes `mayEnergizeActuators(...)` **always deny outputs**, even when host-test synthetic evidence appears complete. Legacy adapter remains uncommissioned and unattached in default `Flytec_Boat.ino`.

**R30C verification:** New [host fault injection test](../Boat/tests/boat_r30c_common_bus_feedback_test.cpp) and [static authority check](../Boat/tools/verify_r30c_common_bus_contract.py) added to `.github/workflows/boat-host-tests.yml`. The test denies actuator energization for missing STOP/watchdog, missing/unsafe common-disconnect evidence, stale contacts, missing battery, missing independent pulse limiter/current-zero, illegal simultaneous motor+release, invalid lease/pulse and clock rollover. Observe a completed CI run before calling R30C verified. The compile-time lock and all physical G01–G24 acceptance gates remain unsatisfied; physical STOP/cutoff hardware, fuse and current ratings **not yet selected or tested**.

**Next engineering work:** qualify a suitable physically independent **single common actuator-bus** STOP/watchdog and monitored state (not two separate rails), update real HAL wiring only after device/revision selection and measured evidence. Current releases Boat VER014 / Remote VER028 frozen; sonar deferred.

## R30C verified CI — source and documentation (2026-11-22)

- [**Boat host Actions #45044409605**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044409605), commit `1d7fef011214277941dbfa6804dbc193fde98fe9`: **SUCCESS**. The dedicated R30C C++17 synthetic STOP/watchdog/common-disconnect/current-zero fault suite printed **"R30C common-bus STOP/watchdog/contact/current-zero model PASS; physical output permanently locked"**; accompanying static source/physical-gate check printed **PASS**.
- [**Latest R30C documentation-triggered Boat host Actions #45044485081**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044485081), commit `e7d7e1588e89ecc9463ce3f87fd6f638406ee0ea`: **SUCCESS**, including R29, R30A, R30B and R30C portable host checks.
- [**Boat ESP32-S3 target Actions #45044281786**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044281786), executable-source commit `c49f83cf8debcf7ce40776aed9e1338e65598d5b`: **SUCCESS**, both non-actuating main Boat diagnostic and default-locked INA231 bench compiled; all artifact uploads completed. Later R30C files updated tests/tools/docs/workflow, not executable Boat source.
- These are **software-only results**, not physical DC cutoff or feedback. `COMMON_BUS_HARDWARE_COMMISSIONED=false`, INA231 physical commissioning false, legacy dual-contactor driver unattached, shared 3S wiring uncommissioned, all 24 R30A physical gates OPEN and 81 older physical checks NOT_RUN. No hardware chosen, no motor/coil load energized or safety claims made; Boat VER014 / Remote VER028 frozen.

## R30D — Read-only single-bus safety feedback transport (2026-11-22)

**Owner's hardware choice unchanged:** exactly ONE 3S4P pack and common 3S high-current distribution for Cytron MDD20A VM and EKM014 VH. No separate motor/solenoid battery/contactor domains, no new modules or PCB.

**Implemented logic-only source:** [`Boat/source/boat_common_bus_readonly_port_r30d.h`](../Boat/source/boat_common_bus_readonly_port_r30d.h) introduces an input-only `ReadOnlySource::read(RawFrame&)` interface for prospective local STOP circuit, independent watchdog, one common DC-disconnect auxiliary, external coil pulse limiter, coil-current-zero, motor-current-zero and INA231 local battery readings. Every observation has its **own original acquisition timestamp and nonzero source ID**; missing/duplicated sources, expired 50ms safety evidence, expired 1000ms battery evidence, I/O failure and raw voltage out of 9000–13000mV are discarded without refreshing cached values. Source-ID checks identify only obvious mapping mistakes and **do not certify independence**.

**No hardware permission:** R30D carries raw observations for diagnosis, but `ACTUAL_READER_COMMISSIONED=false` and R30C's `COMMON_BUS_HARDWARE_COMMISSIONED=false`. Neither external DC rating/power-off test, physical one-hot limiter, independent current-zero nor INA231 calibration is automatically signed by a read. The actual `Flytec_Boat.ino` now includes and static-asserts these **disabled** interfaces during ESP32-S3 compilation, but it constructs no reader, drives no GPIO and attaches no `BoatPhysicalDriver`. Legacy two-contactor adapter remains deliberately uncommissioned and unattached.

**Host regression:** [`Boat/tests/boat_r30d_common_bus_readonly_faults_test.cpp`](../Boat/tests/boat_r30d_common_bus_readonly_faults_test.cpp) simulates seven raw observation channels, the read-only producer, duplicate or unsigned provenance, stale/future timestamps, I2C-like reader failure and cache clearing, STOP/watchdog/disconnect/current-zero/limiter faults, INA231 invalid/overrange/stale data and 32-bit timer rollover. [Static gate](../Boat/tools/verify_r30d_readonly_feedback.py) checks that all authorization fields remain false, one-pack bus unchanged, 24 physical R30A gates OPEN and diagnostic firmware driver unattached. [R30D report](../Boat/docs/BOAT_R30D_READONLY_FEEDBACK_SIMULATION.md).

**Verification:** R30D GitHub host and ESP32-S3 workflow runs were triggered. Confirm their actual completed success/failure independently before declaring this milestone verified. Historic R30C host/target evidence remains valid but is not proof of R30D compile.

**Still OPEN:** physical common DC-rated stop/watchdog/cutoff choice and true contact feedback, separate *functional* current-zero and pulse limiter, high-current fuse/BMS/wire/connector engineering and live INA231 and RF evaluation. No high-current wiring, no motor/solenoid enable, 24 R30A gates still OPEN, 81 historical physical checks NOT_RUN, Boat VER014 / Remote VER028 frozen. CHIRP sonar deferred.

**Next:** observe R30D host and target CI, correct any failures; then develop read-only fault-event telemetry with explicit unverified/STALE status, never an actuator permission or fake motor/coil contact.

## R30D verified results — 2026-11-22

- **ESP32-S3 target CI** [run 45044906755](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044906755) completed **SUCCESS** on Boat firmware-source commit `b8340cb74ebd78261bca474b9285c7abea1219ca`. It compiled the selected non-actuating diagnostic application including the new R30D read-only header/static commissioning guards, the historical locked ADS1115 bench, and selected locked INA231 bench. No reader constructed or actuator GPIO configured.
- **Boat host CI** [run 45045107306](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45045107306) completed **SUCCESS** on source-auditor correction `829bede975471a529103a41222e44a7da846f422`. The R30D simulated seven-source input test **PASS**, source-level no-write/unsigned gate audit **PASS**, and all existing Boat portable host tests **PASS**.
- Earlier intermediate runs [45044974018](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044974018) and [45045063567](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45045063567) failed **only** the static Python scanner: it mistook an explanatory comment `// Deliberately no write()` for an actual write API. The C++ fault test itself passed; the scanner now strips `//` comments before inspecting the concrete `ReadOnlySource` method declarations.
- **No physical signoff:** `ACTUAL_READER_COMMISSIONED=false`, `COMMON_BUS_HARDWARE_COMMISSIONED=false`, `INA231_PHYSICAL_COMMISSIONED=false`, and shared-bus physical acceptance remain false. Both real motor and solenoid power paths are unwired and the actual Boat actuator HAL remains unattached. R30A 24 OPEN gates and prior 81 NOT_RUN checks unchanged. No release changes (Boat VER014, Remote VER028).

## R30E — Read-only fault-status and local USB diagnostic telemetry (2026-11-22)

**Completed in Boat source:** Added `Boat/source/boat_r30e_fault_status.h` with stable 14-code `boat_r30e::Fault` reasons, multi-fault bitmask, mandatory `UNVERIFIED|COMMISSIONING_LOCK`, untrusted voltage sentinel `0xFFFF`, monotonic event-change counter and a versioned 8-byte **LOCAL ONLY** diagnostic sidecar (`D5 01 ...`). Covers R30D uninstalled reader, I/O error, unknown/duplicate input IDs, 50ms current/STOP-contact freshness, 1s local voltage freshness, invalid voltage, missing hardwired STOP/watchdog/common cutoff, limiter and current-zero evidence. Even an apparently healthy synthetic observation returns `UNCOMMISSIONED` and **never** actuator permission.

**Real ESP32-S3 diagnostic integration:** `Boat/source/firmware/Flytec_Boat.ino` compiles this source, constructs only an *empty* R30D observer with **NO installed hardware reader**, and reports `R30E,LOCAL_ONLY,NO_READER,...,outputs=LOCKED,frame=...` every 2000ms over USB Serial; local 8-byte envelope hex is visible for later offline analysis. The application still has **NO physical driver attached**. No new pin or module selected. Do not misinterpret the diagnostic status as a measured STOP contact or operational battery voltage.

**Protocol boundary:** R30E `D5` records are **NOT** a new `RadioPayloadKind`, authenticated frame or Remote battery/safety indicator; no radio packets or Remote software were changed. Any later authenticated telemetry needs a separate bilateral protocol design and tests. Physical hardware STOP/readback cannot be supplied by the ESP's own GPIO latch or an imagined dual-contactor circuit.

**Tests:** Added `Boat/tests/boat_r30e_fault_status_test.cpp` with fake R30D inputs and priority/flag combinations, source failures, missing/stale battery, cached-snapshot timeout, serial-envelope bytes, event transition tracking and physical driver disabled. `Boat/tools/verify_r30e_fault_status.py` verifies only local Serial integration, no safety RF subtype, all R27/R29/R30C/R30D commissioning locks false, all 24 R30A physical gates still OPEN. GitHub host and ESP32-S3 diagnostic target workflows triggered; inspect completed runs before recording PASS.

**Hardware invariant:** ONE 3S4P supply for MDD20A VM and EKM014 VH in parallel, **no separately switched motor/coil battery paths**. No DC-rated common safety cutoff, watchdog or motor/coil current-zero sensor installed. All high-current paths unwired, older 81 physical checks NOT_RUN. Boat VER014 / Remote VER028 frozen, sonar deferred.

**Next:** verify GitHub R30E host and target runs; if green, continue authenticated Remote telemetry protocol planning as a *separate* revision without assuming real input hardware exists.

## R30E CI — independently observed PASS (2026-11-22)

- [Boat host run **45047481082**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45047481082), code/workflow commit `bed9ce336beb6bf32f4ccf84ab6e1f463de90fad`: **SUCCESS**. The new C++17 `boat_r30e_fault_status_test.cpp` passed (all fault-code priorities, local 8-byte envelope, event tracking and permanently locked physical driver). The static `verify_r30e_fault_status.py` audit and all other Boat regression steps passed.
- [Boat ESP32-S3 target run **45047413922**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45047413922), latest executable-source commit `7f21a574a7bb21c3b8b277022d3915ecff104697`: **SUCCESS**. The non-actuating `Flytec_Boat.ino` including R30E local Serial telemetry compiled; the locked INA231 and historical locked ADS1115 bench sketches compiled; artifact steps completed.
- [Post-documentation Boat host run **45047573473**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45047573473), documentation commit `d129e71b4c90221671e15c1a0b2d8e0317693308`: **SUCCESS**. Confirms documentation-triggered host suite still passed. No Boat executable source changed after `7f21a574`.
- R30E's actual on-target expected diagnostic line is **`NO_READER / UNVERIFIED / outputs=LOCKED`** because the physical reader was not selected or attached. The tests do **NOT** certify real safety feedback or live Remote telemetry. The D5 diagnostic envelope remains USB local-only and not an authenticated radio command/payload. The 24 physical design gates remain OPEN, 81 older physical checks NOT_RUN; single common 3S VM/VH architecture and Boat VER014 / Remote VER028 unchanged.

## R30F — Optional authenticated Boat → Remote safety-fault telemetry (2026-11-22)

**Owner-selected hardware unchanged:** ONE common 3S4P pack powers MDD20A VM and EKM014 VH in parallel (separate regulated 5V/3V3 *logic* rails only), INA231 voltage-only sensing, nRF24L01+ primary and E22 UART backup, **NO** separate switched motor/coil supplies or custom PCB. No physical safety reader or independent STOP hardware installed.

**Boat and Remote sources updated on `hardware-ready-dual-radio-r1`:** Added identical `radio_safety_diagnostic_v1.h` on both endpoints with strict optional authenticated telemetry **subtype 8** and sub-version 1 in the existing 32-byte HMAC/session RadioFrame; legacy subtypes 1–7 and message types remain unchanged. Exact bytes: `08 01 reason flagsLo flagsHi millivoltsLo millivoltsHi sequence`. Mandatory bits `UNVERIFIED=0x0002` and `COMMISSIONING_LOCK=0x0800`; invalid, forged-commissioned, unobserved-voltage or unsupported-version packets are rejected.

**Actual source integration:** `Boat/source/firmware/Flytec_Boat.ino` sends R30E status as authenticated **primary-nRF-only** `RADIO_MSG_TELEMETRY` when a secure session and primary freshness permit, in addition to the existing local USB D5 diagnostic. The actual Boat R30D input reader remains absent, so the only honest report is `NO_READER / UNVERIFIED / LOCKED`; failure to send without provisioned RF is normal, not simulated success. Remote `boat_radio.cpp` decodes only after HMAC/session/sequence checks and **does not refresh link-control freshness** from diagnostic-only subtype 8. Backup link diagnostic reception is denied. `boat_telemetry.cpp` stores diagnostic fields separately; diagnostic never touches STOP confirmation, normal battery telemetry, ACKs, motor readiness or mission authority. New `BoatState` read-only fields expire after **6000 ms** and require active primary RF link. `boat_ui.cpp` shows Bulgarian separate footer line `ДИАГН.: НЕПОТВ. / ...` without hiding STOP or critical-battery warning.

**Tests:** Mirrored Boat and Remote codec regression vectors and malformed-frame tests; Remote telemetry-isolation test ensures diagnosis cannot clear STOP, refresh status/battery/link timers or grant actuators. `Boat/tools/verify_r30f_diagnostic_protocol.py` verifies cross-endpoint contract parity and 24 unsigned physical gates. [Full R30F protocol and risks](../Boat/docs/BOAT_R30F_DIAGNOSTIC_WIRE_PROTOCOL.md). [Boat host run 45048750474](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048750474) **PASS**. Remote host and full target build runs are pending observation; do not mark them PASS without checking results.

**Physical release unchanged:** `COMMON_BUS_HARDWARE_COMMISSIONED=false`, `ACTUAL_READER_COMMISSIONED=false`, `INA231_PHYSICAL_COMMISSIONED=false`, no commissioned actuator HAL. All R30A 24 physical gates OPEN, older 81 NOT_RUN, Boat VER014 / Remote VER028 frozen; sonar deferred.

**Next:** verify **both** Remote host and Waveshare target CI; correct compile/test failures. Then review RF cadence/diagnostic visibility under authenticated and dropped primary sessions without introducing STOP overrides. Physical motor/coil power is not approved.

## R30F confirmed verification — 2026-11-22

Actual GitHub Actions results were inspected after each job completed:

- **[Boat host #45048750474](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048750474): SUCCESS.** R30F strict subtype-8 mirrored wire-vector tests, static Boat/Remote security contract and all other Boat portable host tests passed.
- **[Remote host #45048847021](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048847021): SUCCESS.** R30F paired codec golden vectors, invalid/forged-commissioned packet rejection and telemetry-isolation test passed, including a test that diagnosis cannot clear an already confirmed STOP. All other Remote portable host tests passed.
- **[Boat ESP32-S3 target #45048442529](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048442529): SUCCESS.** Diagnostic-only Boat target includes R30F optional authenticated send route, plus locked INA231 and historical locked ADS1115 benchmarks. This is a compilation test, **not** real RF transmission or actuator work.
- **[Remote Waveshare ESP32-S3 4.3-inch target #45048618202](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048618202): SUCCESS.** Complete Remote LVGL/SD/UI/radio target compiled, **1,460,005 / 3,145,728 program bytes (46%)**, **179,512 / 327,680 global dynamic RAM bytes (54%)**, 148,168 bytes reported available for local variables. This is compile output, not a measured runtime memory budget or physical board acceptance.

Intermediate early R30F CI runs failed because their respective commit snapshots invoked `verify_r30f_diagnostic_protocol.py` before that file and its linked R30F document were committed; both strict C++ wire codecs already passed in those runs. The later complete host runs above are **SUCCESS**. No command type or radio-frame authentication contract was changed.

**Status at R30F:** Boat and Remote software paths, strict codecs, host tests and ESP32-S3 builds are verified. Physically uninstalled and unverified STOP/watchdog, shared DC cutoff, independent one-hot coil limiter/current-zero sensors and actual RF range remain **outstanding**. Both endpoints retain all motor/coil STOP locks; R30A 24 physical gates OPEN and prior 81 physical tests NOT_RUN. Single shared 3S pack, no separately switched motor/coil domains, no new module selection. Boat VER014 / Remote VER028 unchanged.

**Next engineering step:** R30G — review rate limiting and bounded-queue/backpressure behavior of diagnostic packets during manual control and deliberate link loss, then regression-test no control-freshness or priority inversion. Physical radio acceptance and motor power remain blocked.

## R30G — No-starvation optional radio diagnostics (2026-11-22)

**Completed and observed green:** [Boat R30G host Actions #45049989270](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45049989270) **PASS**, including old R29–R30F regressions and new R30G source/host priority tests. [Boat ESP32-S3 target #45049610592](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45049610592) **PASS** on the actual updated Boat firmware. Remote source was not changed, previous full Remote host #45048847021 and target #45048618202 both passed.

**Implemented:** [`boat_r30g_optional_diag_budget.h`](../Boat/source/boat_r30g_optional_diag_budget.h) and integrated into [`BoatDualRadioPort`](../Boat/source/boat_dual_radio_port.cpp). R30F optional subtype-8 diagnostic offers are a single replaceable in-memory eight-byte record, **not an immediate nRF send or unbounded FIFO**; stale after 500ms, minimum 2000ms between accepted transmissions. Radio `update()` now serves nRF/E22 RX, watchdog, 10-second conditional RTH, mission, 100ms nRF heartbeat and 3-second backup heartbeat **before** attempting a diagnostic. Diagnostic is denied with pending nRF TX, queued command, active mission, recent authenticated manual/mission-stick command (250ms holdoff), failsafe, detached Home, unauthenticated/stale primary or no recent successfully-started primary heartbeat. Only a 15–40ms post-heartbeat slot may be used, and a failed diagnostic send is discarded without retry. Busy nRF does not falsely advance the heartbeat schedule, leaving later update ticks to retry higher-priority heartbeat. Authentication epoch change drops all older diagnostic records.

**No control changes:** existing RF command types, HMAC/session/replay guards, primary command freshness, motor joystick lease, STOP ACK, Remote R30F diagnostic decode/UI and low-battery/dual-link RTH policy remain authoritative. R30G neither selects modules nor enables actuators. Full implementation/risk notes: [R30G document](../Boat/docs/BOAT_R30G_RADIO_DIAGNOSTIC_PRIORITY.md).

**Physical and release limits:** this is software arbitration, **not a measured hard realtime guarantee**. No hardware STOP, watchdog/contact, current-zero/pulse limiter, real nRF/E22 pairing or common high-current load test. One shared 3S4P pack feeds MDD20A VM and EKM014 VH in parallel (not two switched supplies), physical approval still 24 OPEN and prior 81 NOT_RUN, INA231 and common-bus commissioning false. Boat VER014 / Remote VER028 frozen, sonar model unselected.

**Next R30H priority:** preserve the selected hardware architecture and develop explicit real-RF bench verification procedures and a software trace for pending control, attempted heartbeat, dropped optional diagnostic and 10-second radio-loss transition; physical metrics cannot be claimed until paired modules and actual logic analyzer instrumentation are available.


## R30H — реални RF тестове: готов протокол, измервания все още NOT_RUN (2026-11-22)

**Етапът е подготвен и committed в `hardware-ready-dual-radio-r1`; физическите изпитвания още НЕ са проведени.** Текущият Boat firmware продължава да е неактуиращ diagnostic target и няма commissioned physical driver. Никакъв release bump (Boat VER014 / Remote VER028), няма позволени моторни/соленоидни load тестове или water trials.

R30H артефакти:

1. [Процедура за реалните nRF24L01+ PA/LNA и E22-900T22S](../Boat/docs/BOAT_R30H_RF_LATENCY_BENCH_PROCEDURE.md) — non-actuating RF first, STOP end-to-end/proc timing, controlled one-/two-link loss, conditional 10 s RTH, pre-/post-RTH restoration, staged dummy-load only after independent physical safety gates.
2. [22 сценария, всички NOT_RUN](../Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv) — 30 независими повторения за latency сценарии, негативни interlock проверки и изисквани evidence artifacts.
3. [Празен CSV event timeline](../Boat/docs/BOAT_R30H_RF_EVENTS_TEMPLATE.csv), и [Python read-only analyzer](../Boat/tools/analyze_r30h_rf_events.py) с host-only regressions [test_r30h_event_analyzer.py](../Boat/tests/test_r30h_event_analyzer.py), вграден в Boat host workflow. Празен timeline = NOT_RUN, а не PASS. Реални multi-device E2E времена изискват общ analyzer clock или доказана синхронизация.

**Кодово потвърдена особеност:** primary freshness 250ms, backup freshness 6000ms; nRF heartbeat 100ms, E22 heartbeat 3000ms. `both stale` означава immediate software STOP от момента на staleness, **НЕ** от момента на физическо RF прекъсване; при последен E22 heartbeat е възможен прозорец до 6s. 10s RTH се брои от **последен приет автентикиран frame**, само с активна eligible mission, matching CRC-valid Home и валидни локални safety/navigation условия. Backup-only не разрешава manual моторни команди или release. STOP по E22 може да бъде забавен от 3s pacing — конкретните worst-case измервания и приемливите физически граници са отворени. R30G optional diagnostics не може да refresh-ва основната връзка.

**R30H статус:** процедура/матрица/host evidence checker **PREPARED**; физически radio identity/level/interoperability, RF loss times, STOP output latency, actual 10s RTH motion, независими STOP/current-zero и common-3S hardware **NOT_RUN / BLOCKED**. R30A 24 OPEN gates; R23/R25/R27 81 NOT_RUN. Документацията и analyzer CI са софтуерни, не физически протокол от проведен тест.

**Следващо продължение:** R30H-1 inspect bought module revisions + legal antenna/logic rails; R30H-2 provision secure pairing and collect measured RF-only authenticated timestamps, keeping MDD20A VM and EKM014 VH disconnected; R30H-3 compare actual STOP/dual-link freshness timings and revise unacceptable 6s/3s safety exposures; R30H-4 independent physical signoff before dummy-load tests; R30H-5 accept/reject signed latency budget. Sonar still deferred.


### R30H host verification status (2026-11-22)

- [Boat host Actions #45050691271](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45050691271) completed **SUCCESS** after the initial R30H test+workflow integration, including the new R30H event checker step.
- Subsequent R30H analyzer change makes unmodeled matrix scenarios explicitly **MANUAL_REVIEW_REQUIRED**; an additional regression tests this fail-closed behavior. On [Boat host Actions #45050834071](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45050834071), the **R30H test step itself passed**; overall run was still in progress at last inspection. This is not actual RF measurement or signed STOP/RTH acceptance.
- Current authoritative references remain [R30H bench procedure](../Boat/docs/BOAT_R30H_RF_LATENCY_BENCH_PROCEDURE.md), [matrix 22 NOT_RUN](../Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv), [event template](../Boat/docs/BOAT_R30H_RF_EVENTS_TEMPLATE.csv), [analyzer](../Boat/tools/analyze_r30h_rf_events.py). The R79 legacy ADS1115/separate-VM-VH power worksheet must not be used for physical wiring; active choices are R27 INA231 BUS-only and R29 one common 3S load bus.
