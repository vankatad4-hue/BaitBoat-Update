# Flytec Remote/Boat Archive Review — Reconciliation R6.4

**Reviewed:** 2026-11-20  
**Inputs inspected:** Boat and Remote versioned archives ver001–ver005, Remote ver009, GitHub Import ver004, and current branch engineering files. Archive contents were inventoried and the project-status, hardware, pin assignment, netlist, power, capture-sheet, safety, mechanical, firmware, radio test and release documents were reviewed.

## Version archive history

| Archive | Review finding |
|---|---|
| Boat ver001–005 | Progressive status/hardware/radio/PCB planning. ver005 adds release note; boat hardware remains untested and pin map is not frozen. |
| Remote ver001–004 | Progressive project master docs; firmware scaffold, offline-map and UI code; ver004 carrier plan and dual-radio strategy. |
| Remote ver005 | Verified Actions snapshot; checksum/build-status doc says archive integrity and SHA verified. Still not compiled/hardware tested. |
| GitHub Import ver004 | Source/docs import bundle with Remote and Boat plus ver004 ZIPs; useful historical baseline. |
| Remote ver009 | More recent consolidated development archive: dual-radio code, SC16IS750 UART bridge, button scanner, release guard, test sketches, R5 CAD and detailed electrical documents. It is a development bundle, not a validated hardware release. |

## Decisions already established by R5/R6 engineering records — do not reinvent

### I/O architecture
- MCP23017 @0x20 handles **16 independent physical button inputs** (8 left + 8 right); no 4×4 matrix.
- MCP23017 @0x21 handles radio/status/OK expansion. OK is a separate physical input on GPA4.
- ADS1115 @0x4A handles joystick L-X, L-Y, R-X, R-Y.
- ADS1115 @0x4B handles battery, NTC and optional radio rail sensing.
- SC16IS750 @0x48 is the I2C-to-UART bridge for the E22 backup radio.
- Shared I2C: ESP GPIO8 SDA / GPIO9 SCL. Existing onboard GT911, CH422G and RTC must be included in bus scan.
- nRF24 uses SPI GPIO11/12/13 shared with TF card; separate CS and serialized access.
- nRF CE GPIO6 and CSN GPIO15 are provisional direct GPIOs. GPIO6 is tied to the Sensor AD circuitry; GPIO15 is associated with RS485 TX. Those onboard paths must be disconnected/disabled and tested before use. CE/CSN remain direct GPIO, not the expander.
- E22 UART goes through SC16 bridge; do not wire directly to the ESP USB-UART or RS485 pins.
- PORT1 physical connector crosswalk remains provisional; no cable or PCB should use header pin numbers until board revision and orientation are physically verified.

### Power architecture
- Controller/display uses Waveshare onboard single-cell input/CS8501 boost only, after load/charge/backfeed/thermal verification.
- nRF PA/LNA uses separate TPS63070 BAT_PROT → 3.3V radio rail.
- E22 uses separate TPS61023 BAT_PROT → 5V rail.
- Converter outputs must never be paralleled.
- 1S4P battery, protection/fuse, reverse-polarity MOS candidate, cell pack, charge behavior and thermal design remain unverified.
- Power net draft R8 and BOM candidate R6 already exist and should be treated as current candidate documents, not replaced with a simplified architecture.

### Physical design
- R5 baseline: 310×209×69.5mm concept enclosure, 4.3-inch central display, dual sticks, 16 physical keys (two 2×4 banks), one separate OK/confirm key and one power key above the display, two antenna exits.
- Right-bank top four keys are independent guarded hopper-left/hopper-right/hook-left/hook-right release controls. Release confirmation is a separate OK button. No combined release.
- Current R6.3 concept adds front/rear/internal view refinement; keep the user-approved control layout. Do not introduce a bottom duplicate power switch.

## Corrections to the previous R6.4 brainstorming documents

The recent preliminary documents incorrectly proposed replacing the R5 16-button expander with a 4×4 matrix and ADS1015, and treated GPIO15/6 and TF SPI pins as unusable outright. These proposals are superseded by the established R5/R6.3 design:
- Keep discrete 16-input MCP23017 button bank and separate OK.
- Keep ADS1115, not ADS1015.
- Keep SC16IS750 for E22 UART.
- Keep the documented direct nRF CE/CSN allocation as a **candidate**, subject to the required board isolation and bench test.
- Shared TF/nRF SPI remains a design candidate with arbitration and validation.
- Do not make a new GPIO pin plan until these documents are reconciled and exact board revision is checked.

## Actual next engineering gate

The next step is **schematic consistency and design review**, not another conceptual I/O architecture:
1. Reconcile the current pin/netlist/CSV with R5 button mapping, R4 schematic capture sheets and R8 power net draft.
2. Review bridge address 0x48 and ADS1115/MCP23017 addresses against the board's I2C devices; perform scan when the Waveshare board is available.
3. Check nRF CE/CSN repurposed onboard paths, shared TF SPI arbitration, E22 SC16 bridge and IRQ routing for consistency.
4. Resolve all contradictory revisions and produce one authoritative REMOTE_SCHEMATIC_NETLIST_R6_4.md with each net, connector pin, GPIO/device pin, voltage, direction, and validation status.
5. Then capture native KiCad schematic (or a clearly labeled reviewable circuit drawing), run ERC when the project exists, and only after part footprints are verified proceed to PCB layout.

## Release/testing status remains unchanged
Neither boat nor remote firmware is compiled/hardware tested. Native KiCad schematics, ERC/DRC, PCB layout, key provisioning, replay-safe session management, application ACK, boat-side actuator integration and failsafe tests are incomplete. No actuator or field operation is authorized.
