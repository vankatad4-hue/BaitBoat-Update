> **R20 PROCUREMENT AUTHORITY:** The active Boat BOM, battery-sensing module and power-rail allocation are in [BOAT_HARDWARE_SELECTION_R20.md](BOAT_HARDWARE_SELECTION_R20.md). This R15 document is historical for its Heltec/older regulator candidate table; do not order from that table. The no-custom-PCB and ready-made-module-only policy remains active.

# R15 — Boat premade-modules-only implementation plan

> **R20 PROCUREMENT AUTHORITY:** Use [BOAT_HARDWARE_SELECTION_R20.md](BOAT_HARDWARE_SELECTION_R20.md) for all current Boat parts, including ADS1115 battery sensing. This R15 document and its Heltec/older regulator candidate table are historical; do not order from that table. The no-custom-PCB and ready-made-module-only constraint remains active.


> **R18 HISTORICAL UPDATE:** The old 77×79mm/30mm carrier note is superseded by the distributed-module decision and then by the active R20 BOM above. Do not treat this R18 note as a current procurement or mechanical constraint. The current modular installation plan is R18; the current component list is R19.

> **R18 MODULAR INSTALLATION POLICY (still active):** The old 77×79mm/30mm carrier envelope is retired; selected premade modules may be mounted separately along the hull. Manufacturer-supplied or factory-produced electronic modules only, no custom PCB. The R18 sourcing register is historical; the active BOM is R20. [Distributed mounting details](BOAT_DISTRIBUTED_MODULE_INSTALLATION_R18.md).
**Status:** Active architecture plan; no assembled electrical harness, no qualified STOP interlock

## Modules / supplies
| Slot | Function | Premade board/module | State |
|--|--|--|--|
| B01 | GNSS, boat application, telemetry | Heltec Wireless Tracker v1.2 ESP32-S3 | retained; physical pin mapping TBD |
| B02 | Two independent 550 brushed motors | Cytron MDD20A | retained; independently mounted along hull |
| B03 | LH RH LK RK 12V coil power switching | Electrokit EKM014 (4x10A published) | **OWNER SELECTED (R16)**; not physically installed, coil current and safety qualification pending |
| B04 | Safety logic 3.3V | RECOM R-78E3.3-0.5 preassembled SIP3 with commercial screw-terminal adapter | selected candidate; independently supervised |
| B05 | Heltec supply 5V | Pololu S13V20F5 5V/2A | candidate separate from VL, source/injection pins TBD |
| B06 | EKM014 VL supply 5V | separate Pololu S13V20F5 5V/2A | candidate, not paralleled with Heltec supply |
| B07 | Primary nRF supply 3.3V | Pololu D24V22F3 3.3V/2.6A | candidate; separate filtering from B04 |
| B08 | Hardware physical E-STOP and DC disconnect | manual latching safety switch + DC-rated contactor/interlock premade modules | **NOT SELECTED**; no safe actuator power |
| B09 | Hardware one-shot / one-hot / rearm | premade sequencer/timer/interlock controller | **NOT SELECTED**; no coil power |
| B10 | Independent power-on/brownout supervisor | premade relay/supervisor module with documented threshold and reset | **NOT SELECTED** |
| B11 | Coil/motor current monitoring | premade current monitor module(s) with rated current/shunt | **NOT SELECTED** |
| B12 | 3S power distribution | commercial covered fuse/terminal block and locking connectors | ratings and fit TBD |
| B13 | MPU9250/BMP280 heading/pressure | premade sensor breakout/module | pin logic and 3.3V bus TBD |
| B14 | Primary nRF24L01+ PA/LNA | premade 2.4GHz radio module | antenna and pin/rail budget TBD |
| B15 | Boat backup LoRa | Heltec integrated SX1262 | hardware onboard but software **DISABLED** pending Remote E22 PHY compatibility |

## Single effective high-current topology
```text
3S4P+BMS -> MASTER SWITCH + FUSES -> [physical latching E-STOP/DC cutoff (UNSELECTED)]
                                  +------ MDD20A motor power -> two brushed 550
                                  +------ EKM014 VH -> four independent solenoids
         -> fused LOW CURRENT branch -> RECOM safety 3.3V -> interlock logic
                                   -> Pololu 5V candidate B05 -> Heltec (approved input only)
                                   -> Pololu 5V candidate B06 -> EKM014 VL
                                   -> Pololu 3.3V B07 -> nRF24 PA/LNA

Heltec physical requests -> premade nonretriggerable, one-hot, release-qualified
  controller B09 (NOT SELECTED) -> EKM014 IN1..IN4
B08 + B10 + B11 -> independent fail-OFF cutoff; physical rearm REQUIRED.
```

The MDD20A remains a commercially manufactured independent module, not on a carrier PCB. Only cable harnesses connect it to the Heltec and separate battery/control branches. The EKM014 board's **VL 4.5–12V** is NOT the same as its **VH up to 50V**. Do not connect raw full 3S to VL. Both 3.3V logic and radio supplies must have separate noise filtering.

## No PCB fabrications, including "small safety helper board"
Archived: `Boat/hardware/Direct_MDD20A_R7`, `Coil_Drivers_R9`, `Safety_Cutoff_R10`, `Pulse_Limiter_R11`, `Release_Token_R12`, `Safety_Power_R13`. Their schematics are not manufacturing targets; there is no authority to fix their ERC and spin board layouts. Prior carrier PCB routing, native KiCad netlist and Gerber priorities are **canceled**.

## Important power/thermal blockers
- 20A BMS is a nominal rating; simultaneous motor startup and 6A coil allowance may trip it. Measure actual peak current, connector/fuse and motor/coil release power.
- A contactor rated 40A **AC** is NOT automatically rated 40A switching low-voltage **DC motor inductive loads**. Pololu #2813's approx 16A continuous is **below** the currently assumed 20A BMS rating and vendor explicitly warns against treating it as an emergency/safety disconnect. Neither is approved as B08 now.
- An INA226 generic breakout over I²C is useful for logging, but software readout is NOT independent fast current-zero/short protection. Don't convert telemetry to safety evidence without actual hardware thresholds.
- A generic Cebek I-33 non-retriggerable dual-timer 12V relay is a **research candidate** only; the spec starts at ~0.1s, has optional power-up mode and relay contacts; it does not natively prove four-way one-hot+release consumed, and actual coil energization time is unmeasured.
- If the modular components cannot fit and remain serviceable in the V080 hull, use a different modular enclosure/placement design or reconsider scope; do not covertly reintroduce a custom PCB.

See the R15 [commercial safety candidate matrix](../../docs/MODULE_ONLY_SAFETY_SHORTLIST_R15.md), [premade-only acceptance checklist](../../docs/MODULE_ONLY_ACCEPTANCE_R15.md) and [module-only pin map](../../docs/MODULE_ONLY_PIN_AND_BUS_MAP_R15.csv). These are procurement/bench documents, not permission to energize loads.


## R16 owner-confirmed four-channel module (2026-11-21)

**HARDWARE SELECTION FROZEN:** Electrokit **EKM014**, manufacturer/article **41024394**, is the final *model choice* for Boat LH/RH bait and LK/RK hook solenoid **low-side** switching. Exactly **ONE** four-channel preassembled module. It **replaces R9 custom-FET PCB permanently** in the active module-only project. This choice does **NOT** mean purchased, electrically verified, qualified, or safe to energize. No substitution to NCD or other coil driver without owner approval.

Read the dedicated [EKM014 R16 hardware and wiring contract](BOAT_EKM014_SELECTED_R16.md), [R16 channel/wiring CSV](BOAT_EKM014_PIN_WIRING_R16.csv) and [R16 commissioning test plan](BOAT_EKM014_ACCEPTANCE_R16.md) (relative paths from repository root). R15 module-only policy remains mandatory for both sides. A dedicated *commercial* hardware one-hot and no-repeat maximum pulse path, independent DC power cutoff/STOP, rail/brownout supervision and independent current-zero verification are still required before connecting coils. Sonar still planned only; no RP2040/rudder/custom PCB. Remote VER028 / Boat VER014 remain held.

## R17 — premade actuator safety procurement and wiring gate (2026-11-21)

**OWNER-CONFIRMED Electrokit EKM014 41024394 unchanged; no custom electronic PCBs.** New [R17 manufacturer-checked safety stack](BOAT_PREMADE_SAFETY_STACK_R17.md), [18-stage interface/handoff matrix](BOAT_R17_MODULE_SAFETY_HANDOFF.csv), [21 hard physical release gates](BOAT_R17_RELEASE_GATES.csv) and [11 named candidate/required-component decisions](../../docs/MODULE_ONLY_CANDIDATES_R17.csv). Status **ALL 20 applicable physical gates NOT VERIFIED**; one sonar item is NOT_APPLICABLE by prior decision. These are commercially preassembled module candidates, **not a safe/assembled installation**.

Vendor review: Duelco **NST-3.2 42042237 8–36VDC** stop **supervisor** CANDIDATE, manufacturer-rated only **3A at 24VDC resistive contacts** (not motor/coil rails); manual reset X1–X2 must not be bypassed (permanent bridge causes automatic restart). Albright **SW60C/SW60A 12V continuous-duty** DC inductive contactor with optional auxiliary feedback CANDIDATE, full order code/coil sag/feedback/thermal/fit unverified. **SW60P IP66 has NO available auxiliary contacts** per manufacturer datasheet, so it fails monitored-contact requirement unless a genuinely validated alternative feedback path is provided. Factory-built ShillehTek 4-channel programmable 0.01-s mode timer is only a **functional research candidate**, NOT proved to reject simultaneous requests/held-high repeats/brownout or safe pulse width, and NOT a qualified safety PLC. Independently hardware-timed, one-hot, no-retriggering coil gating, current-zero and qualified reset/POR are still UNSELECTED. Industrial safety modules appear too tall for original 30mm slot; measure another boat hull area before shopping.

**CI R17 module-only gate passed** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44935838255: checks frozen EKM014, 11 candidate statuses, 18 unbuilt handoff entries, 21 physical gates and pass-disabled safety without MCU. Prior archive KiCad workflows remain disabled; output GPIOs stay uncommissioned. Remote VER028, Boat VER014 held, CHIRP sonar not implemented.

