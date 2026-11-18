# New Handheld Remote Hardware Development

This folder contains the hardware design workspace for the new handheld bait-boat remote.

## Current concept
- 4.3-inch central touchscreen.
- Separate power button above screen; USB-C service connector.
- Two joysticks above two button banks; 16 buttons total, 2×4 per side.
- Ergonomic grips; no shoulder/rear buttons.
- nRF24L01+ PA/LNA control/telemetry radio with dedicated antenna.
- Waveshare ESP32-S3 touchscreen board uses onboard antenna; no separate ESP32 antenna path is required.
- 1S4P Li-ion pack, four matched 18650 cells in parallel; target 2500 mAh/cell is not yet selected/validated.

## Workspace
- CAD: shell, front/rear, internal mockup; preliminary only.
- PCB: power design, connector plan, schematic drafts.
- MECHANICAL: dimensions, component envelopes, placement.
- WIRING: cable and connector interface documents.
- docs: design decisions, RF, bring-up.

## Power design
- Display rail: TPS61088 candidate, 5.0 V.
- Radio rail: TPS63070 buck-boost, 3.3 V.
- Radio rail component baseline and calculations: `PCB/TPS63070_COMPONENT_CALCULATIONS_REVIEW_R1.md`.
- Power distribution and net-level draft: `PCB/POWER_DISTRIBUTION_DESIGN_R1.md`, `PCB/POWER_SCHEMATIC_NET_DRAFT_R1.md`.
- Main 3 A time-delay fuse is provisional; protection, reverse polarity, current budget and thermal design require verification.
- Charging schematic excluded from current design scope.

## TPS63070 radio rail — calculated baseline (R1)
- Feedback: 470 kΩ/150 kΩ, calculated 3.307 V.
- Inductor: 1.5 µH Coilcraft XFL4020-152ME candidate.
- Input capacitors: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output capacitors: 3×22 µF 16 V X6S 0805 + 10 µF 0603; 76 µF nominal.
- VAUX bypass: 100 nF.
- FB2 grounded, VSEL static LOW, PS/SYNC HIGH power-save; EN defined by proposed 10 kΩ pull-up; optional PG 100 kΩ pull-up. Verify all control pin states and biasing against exact datasheet before schematic capture.
- At 200 mA radio load, estimated battery current at 3.0 V is 245 mA assuming 90% efficiency.
- Whole remote preliminary battery estimate at 3.0 V is 1.13 A using display 450 mA at 5 V/85% and radio 200 mA/90%; assumptions only, excludes transient/other loads.

## Status / limitations
Documents are engineering planning and net-level design records, not fabrication-ready schematics or BOM. Native KiCad capture, symbol/footprint pin verification, ERC, PCB layout and bench testing have not been completed.
