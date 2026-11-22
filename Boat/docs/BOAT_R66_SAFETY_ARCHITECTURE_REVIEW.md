# R66 — Independent safety controller capacity and 3S-to-24V feasibility

**Date:** 2026-11-22. **Status:** design research only, **NO_ACTUATOR_ENABLE** and **NO APPROVED ORDERS**.
**Selected electronics remain unchanged:** ESP32-S3-DevKitC-1-N32R16V, MDD20A, EKM014 41024394, primary nRF24 PA/LNA, matched E22-900T22S (factory carrier unresolved), Waveshare ESP32-S3-Touch-LCD-4.3 Standard; no Boat SD; Remote integrated SD; sonar deferred. No custom electronic PCB.

## Decision: no single confirmed safe 4-channel + 2-disconnect controller yet

We need **18 independent input functions** and **six logical safe output functions** under the conservative architecture. Exact names and whether a physical sensor exists are in [R66 input/output contract](BOAT_R66_INTERLOCK_RESOURCE_REQUIREMENTS.csv). This is **functional accounting, not a verified terminal pinout**. Two of the six outputs are **high-current independent motor and coil load isolation**; four outputs are independently bounded one-hot EKM014 IN1..IN4 requests. This is an intentionally conservative count and is subject to the manufacturer-approved fault analysis.

| Device | Claimed safety-capable inputs | Safe outputs | Interpretation |
|---|---:|---:|---|
| [Pilz PNOZ m B0 **772100**](https://www.pilz.com/en-INT/eshop/product/772100) | 20 (=12 digital + 8 configurable) | 4 | **Best input-capacity research candidate** only if TWO external independently monitored physical load cutoffs are supplied by another verified safety chain. 4 outputs cannot also serve as two separate disconnect controls. Manufacturer notes 772100 firmware 2.6 limitations; newer revision **772714** has additional features. Do not assume exact pulse/edge blocks without checking selected firmware/configurator release. |
| [Pilz PNOZ m C0 **772105**](https://www.pilz.com/en-INT/eshop/product/772105) | 8 | 4 | Too few for all 18 independently observed inputs; cannot claim standalone complete interlock. Smaller physical unit ~101.4×22.5×120mm, no condensing moisture / IP20 housing. |
| [Omron G9SP-N10S](https://industrial.omron.eu/en/products/g9sp-n) | 10 | 4 | Too few independently observed inputs. 24V consumption specified 400mA (~9.6W), 4 output current limit 0.8A each. No 3.3V native output. |

**Important:** The input count is an engineering *upper-bound allocation*; individual devices may combine physical E-STOP, EDM and test-pulse functionality internally in ways that consume more (or fewer) resources. The 20-input B0 count does NOT prove that this exact logic program can be compiled, validated or certified. We have not installed the manufacturer's configurator or obtained board/PLC hardware. Manufacturer safety ratings apply only within the rated use and correctly validated whole system, not to a hobby boat automatically.

## Conditional architecture B — independent stop/load cutoff + four isolated coil gates

This is an unbuilt, **unapproved** block-level research hypothesis, not an installation wiring diagram:

1. **Upstream:** actual BMS/3S measurements, master manual service disconnect and proper DC fuses (ratings remain TBD); separate fused 5V/3V3 supplies for diagnostics, no actuator return currents through logic.
2. **Safety domain:** selected **factory-made** 24V converter and conditional programmable safety controller **only after** verified under full pack sag, start-up and wet-environment installation. Safety approval requires documented STOP and reset behavior and fail-off recovery without automatic restart.
3. **Independent physical STOP/domain cutoffs:** an appropriate dual-channel supervised STOP module (e.g. Duelco NST-3.2 42042237 candidate, 8–36V DC) and **independent rated NO DC disconnect(s)** for both motor supply and EKM014 VH, with true positive mechanically related feedback; manufacturer-verified coordination with software/PLC. A STOP relay's 3A DC resistive contacts are only for contactor coil/control, NOT load currents. An additional externally energized watchdog with qualified physical first-fault lockout is separate from the ESP32 internal watchdog.
4. **Controller release timing:** 4 conditioned request channels from an uncommissioned I²C expander (e.g. MCP23017) through a suitable **prebuilt 3.3V-to-24V signal interface** (exact product NOT selected); independently enforce all-requests-low baseline, edge consumption, all-off on simultaneous requests, nonretriggerable maximum pulse, current-zero and feedback before rearm, and power-on failure blocking.
5. **Output interface:** the four 24V safe outputs require a **factory-made and manufacturer-validated 24V-to-EKM014 3.3V active-high isolation/level interface** (not selected). **Never** connect a 24V PLC output directly to EKM014 INn or the ESP32 GPIO. The EKM014 VL logic rail is 4.5–12V and load VH 5–50V; EKM014 INn high level may be 3.3V but must remain at or below VL. EKM014 factory connector/header components ship unsoldered; prebuilt connectorized accessory or approved assembler is needed.
6. **Contactor feedback + current zero:** four coil-zero and two motor-zero physical sensors, monitored contactor auxiliaries and 24V supply-good need preassembled actual devices; INA260 I²C via MCU alone is **diagnostic only** and cannot be an independent disconnect guard.
7. **Pulse selection:** exact maximum milliseconds, pulse min, coil current zero threshold, feedback delay and RTH STOP priority must be set from measured hardware, not a generic safety PLC preset. No release pulse is electrically approved now.

**This architecture is not a claim that a particular Pilz controller can implement all functions.** A Pilz application engineer must confirm the exact supported timers / one-shot edge behavior, repeat/hold protections, reset logic and how to implement cross-channel output checks using actual certified blocks and revision. Also confirm safe output switching latency versus the yet-unknown required coil pulse duration. Without proof, the design remains rejected for powered actuation.

## 24 V module-only power sourcing, Bulgaria-first

- [MEAN WELL DDR-15G-24](https://meanwell.bg/product/ddr-15g-24): Bulgarian official distributor Omega Light lists 9–36V input, 24V **0.63A / 15W**, nonwaterproof DIN unit. Research only. It cannot be assumed sufficient for PLC, status loads and contactor coils.
- **New more promising supply candidate:** [MEAN WELL DDR-30G-24](https://meanwell.bg/product/ddr-30g-24), same verified Bulgarian specialist, **9–36V input, 24V 1.25A / 30W, 89% nominal efficiency**, 54.5×35×90mm, vendor shows **€24.00 excluding VAT / €28.80 including VAT** and listed *in stock* at time of lookup. Not checked out, reserved or tested; **power budget and EMI still unresolved**. A 24V output may power an industrial controller/interface *only within its exact rated load*; do not use it for two 550 motors or 12V coils.
- **Critical 3S voltage boundary:** 9–36V input minimum = 9V. Real 3S load sag or BMS undervoltage may fall **below 9V**. Independent safety must force STOP and prevent unexpected auto-restart across droop/rail recovery. Regulator hiccup or restart is not a latch.
- 30W/24V => 1.25A rated. Omron G9SP-N10S specified up to 400mA (9.6W) leaves *at most* 0.85A / 20.4W nominal for 24V accessories, before converter/surge/load reserve, **not** an approved budget. Pilz B0 controller nominal/peak wattage not verified; do NOT substitute its unknown power with Omron's number.
- The selected 3S 20A BMS rating is not sufficient information. Under a **provisional single-coil 6A** allowance, at most 14A remains against the nominal 20A current limit before motor, controller, coil-start transients, contactor, fuse margin and startup. If motors draw >14A combined during release, the reported BMS nominal limit is exceeded. **This is arithmetic planning, not evidence BMS can deliver 20A continuously or interrupt faults.**

## Manufacturer-backed evidence needed before purchase

**B0 exact firmware/version** and approved Configurator timer and edge/feedback blocks, input common/reference requirements and 3.3->24V interface; **24V safe output -> EKM014 high-level premade interface**, monitored external current-rated contactors and nonauto reset; independent heartbeat and low-voltage latch, current-zero sensor exact SKUs; 3S-under-load sag and worst-case PLC/regulator start sequence; 24V/5V/3V3 EMC. Obtain local Bulgaria authorised distributor and physical availability with complete order code (not a public search product thumbnail).

A [R66 source-only capacity test](../tests/test_r66_safety_plc_budget.py) checks the 18/6 function ledger and enforces the fact that B0 is **not standalone sufficient** and smaller candidates do not meet current assumed inputs. This test is not a Pilz compile, electrical acceptance, native schematic or proof of external PLC reliability.

**Physical acceptance and supply-gate:** No GPIO or MDD20A/EKM014 actuator power enabled, no PLC program installed, no parts purchased, no on-water tests. R65 N32R16V pin crosswalk remains tentative. The E22 SMD carrier is still unresolved. Keep source marker Boat VER014, Remote VER028, never claim an operational release.

### Worst-case 3S current sensitivity (engineering bound, NOT a measured load)

At the DDR-30G-24 published full **30W** output and published nominal **89%** efficiency, indicative 3S input demand is `Iin = 30W/(Vin * 0.89)`: approximately **3.75A at 9.0V**, **3.01A at 11.2V**, and **2.68A at 12.6V**, not including startup surge or derating. A provisional **6A single solenoid** at the 9.0V corner plus a 30W safety rail would consume **~9.75A** of the reported 20A BMS nominal output budget *before both motors, MCU, radio, contactor and margin*. This is not a claimed real operating consumption. It highlights why two 550 stall currents must be measured and why a 30W DC/DC rail cannot be casually added to a 20A pack assumption. A suitably sized fuse and safe low-voltage shutdown must be derived from actual current waveforms.


### R67 module choice downstream note (non-actuating)
The user confirmed **GY-91 MPU-9250 + BMP280** and exact **Waveshare MCP23017 IO Expansion Board SKU15391** for Boat. The MCP23017's **manufacturer default I²C 0x27** supersedes R65 generic 0x20 assumptions. This only establishes a low-voltage four-request expander model; it **does not supply safety-rated, independently timed 4-channel onehot control** or reduce the 18-input/6-output independent safety requirements. The actual I/O PLC interface, level conversion and physical DC isolation remain unresolved. See [R67 document](BOAT_R67_GY91_MCP23017_SELECTION.md).


## R77 firmware policy update — no new hardware

The user directed that no physical elements be added and that the boat stop all outputs when both radios are lost, then continue an eligible autonomous mission when an authenticated link returns. R77 implements this as a software watchdog over existing nRF24/E22 freshness. It removes automatic RTH on total link loss, holds eligible mission state, requires neutral/new manual input after primary recovery, and aborts rather than replaying an interrupted release. The authenticated session is retained across ordinary RF silence so backup E22 can restore after a long outage.

**This does not close R66 independent hardware requirements.** The watchdog runs on the main ESP32-S3 and cannot interrupt power on MCU freeze, stuck GPIO, shorted driver MOSFET or welded contactor. The physical STOP, motor/EKM014 VH contactors, independent watchdog, monitored feedback, one-hot bounded non-retriggerable pulse controller and current-zero sensors remain unselected/uncommissioned. No actuator power or physical acceptance is authorized. See [R77 implementation](BOAT_R77_DUAL_RADIO_WATCHDOG_IMPLEMENTATION.md).
