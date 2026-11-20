# Flytec V080 Bait Boat Retrofit

**This GitHub repository is the canonical source of truth for the project.** Approved project decisions, documentation, firmware source, and versioned releases belong here.

## Project layout
- `Boat/` — boat-side firmware, configuration, docs and releases.
- `Remote/` — remote firmware, UI, docs and releases.
- `docs/` — shared architecture, decisions, safety, validation and repository workflow.

## Development workflow
1. Check the relevant repository files before starting work.
2. Implement approved changes in the relevant source files.
3. Update documentation when decisions, interfaces, hardware, safety or behavior change.
4. Keep Boat and Remote changes coordinated when protocol or shared requirements change.
5. Review the changes and verify them before committing.
6. Increment release versions for deliverable changes. Never overwrite prior release ZIPs.
7. Use this repository—not chat history—as the authoritative record. Chat is for planning, discussion and development assistance.

## Project baseline
- Retain the Flytec V080 hull, 12 V 550 brushed motors, battery indicator and main power switch; replace the control system.
- Boat controller target: Heltec Wireless Tracker V1.2 (ESP32-S3 + UC6580 GNSS).
- Remote target: Waveshare ESP32-S3-Touch-LCD-4.3 standard, 800×480, LVGL 9.6.0, Arduino IDE.
- Radio target: nRF24L01+ PA/LNA modules on both ends; 250 kbps initial test rate; 500 m range is a goal, not a guarantee.
- Motor driver target: Cytron MDD20A dual-channel module, subject to current measurements and validation.
- Sonar target: ready CHIRP-style module with echogram/fish-arch output; module and interface are not yet validated.

## Safety and validation
Motor running/stall currents, solenoid coil currents, battery/BMS limits, wiring, fuse sizing, and final pin assignments remain verification tasks. Outputs must default OFF on reset and invalid control. Return-home on link loss is permitted only if navigation and system health are valid.

## Release naming
- `Flytec_Boat_verXXX.zip`
- `Flytec_Remote_verXXX.zip`

Do not overwrite an existing release archive. Update the project documentation and version history when releasing changes.

## Remote power design progress (2026-11-20)
- Pack: protected 1S4P Li-ion; charging schematic excluded from this design phase.
- Radio rail: TPS63070 3.3 V buck-boost component review R2 and pin map R2 completed against TI datasheet. 1.5 µH inductor and TI capacitor baseline are candidates; effective capacitance and actual radio burst current still need verification.
- 5 V boost: TPS61088 calculations R2 exist, but a new selection review recommends evaluating TPS61023 instead. The current boost IC selection is **not frozen**. TPS61023 offers a simpler 5 V single-cell reference design and integrated compensation; expected rail requirement is 450 mA continuous with 1 A transient margin, pending load/thermal validation.
- Battery protection architecture drafted: pack BMS, fuse candidate, reverse-polarity MOSFET, battery voltage sense and NTC. Parts not selected.
- Estimated pack current at the conservative design point is about 2.6 A from a 3.0 V pack (5 V/1 A + radio 3.3 V/300 mA, with assumed converter efficiencies); actual load and efficiency must be measured.
- Native schematic, final boost selection, compensation, ERC, PCB and bench bring-up remain incomplete.

Documents:
- `Remote/New_Handheld_Remote/PCB/BOOST_CONVERTER_SELECTION_REVIEW_R1.md`
- `Remote/New_Handheld_Remote/PCB/TPS61088_COMPONENT_CALCULATIONS_R2.md`
- `Remote/New_Handheld_Remote/PCB/TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md`
- `Remote/New_Handheld_Remote/PCB/BATTERY_INPUT_PROTECTION_DESIGN_R1.md`
- `Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R2.md`
- `Remote/New_Handheld_Remote/PCB/POWER_DISTRIBUTION_DESIGN_R2.md`
