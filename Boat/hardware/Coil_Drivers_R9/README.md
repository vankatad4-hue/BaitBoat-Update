# R9 native KiCad: four independent low-side solenoid drivers (2026-11-21)

**DRAFT / NO FABRICATION OR ENERGIZATION**. This is a standalone, editable native KiCad schematic and local symbol library, **not** a completed power-and-safety design.

Each of LH/RH bait hoppers (FW0630 009, 12V) and LK/RK bait hooks (FW0630 001, 12V) has separate fuse F1..F4, keyed two-wire coil connector J3..J6, individual low-side Q1..Q4 MOSFET, diode D1..D4 **cathode to fused coil positive** and anode to switched coil negative, series gate R1/R3/R5/R7 (100 ohm preliminary), gate-to-return pulldown R2/R4/R6/R8 (100 kohm preliminary). Source is returned to ACT_RETURN. Diode orientation and MOS pin mapping are provisional generic symbols; must match selected actual devices.

J1 is **not an onboard power gate**: it ONLY imports a rail named ACT_SOL_POS_AFTER_LATCH_TBD. The independent physical STOP, TPS3431 watchdog fault latch, battery UVLO, current/thermal fault gate and real actuator power-disconnect are *not yet implemented* on this sheet or elsewhere as qualified hardware. J2 is ONLY an interface for a not-yet-designed four-channel independent hardware one-hot/max-pulse limiter. **Never connect J2 directly to ESP32 GPIO or an I2C expander.** Without that physical limiter a stuck high gate could overheat a solenoid. Firmware one-shot is insufficient. No current-zero feedback is designed yet.

Zero footprints are frozen. Actual FET VGS/SOA/heat and diode clamp voltage/energy vs solenoid release speed, fuse curves, actual current and connector sealing depend on physical measurements. The pack 3S BMS 20A sticker is not independent short-circuit/thermal proof. For testing, no motor/coil power may be connected before the high-side fault latch and pulses have been physically verified.

*Topological* netlist testing (once KiCad CLI runs) can check pin-to-net pairing, not safety qualification. The **separate R7 MDD20A/TPS3431 preliminary schematic still has 15 ERC errors**; these are not fixed by the standalone R9 file. Physical STOP/fault latch, gate timer, feedback, actual MDD20A power, approved Heltec V1.2 pins and PCB remain open. No RP2040, rudder, Boat E22 or sonar. Keep Remote VER028 / Boat VER014 unchanged.

| Branch | Fuse | Output plug | Suppressor | MOSFET | Gate series | Gate pulldown |
|---|---|---|---|---|---|---|
| Left hopper LH | F1 | J3 | D1 | Q1 | R1 | R2 |
| Right hopper RH | F2 | J4 | D2 | Q2 | R3 | R4 |
| Left hook LK | F3 | J5 | D3 | Q3 | R5 | R6 |
| Right hook RK | F4 | J6 | D4 | Q4 | R7 | R8 |

All four branches share only the supply/return and intentionally independent gate pulses; no software has been enabled by this change. Vendor sources: TI TPS3431 https://www.ti.com/lit/ds/symlink/tps3431.pdf ; Cytron MDD20A https://cdn.robotshop.com/media/c/cyt/rb-cyt-287/pdf/cytron-20a-6v-30v-dual-dc-motor-driver-datasheet.pdf .
