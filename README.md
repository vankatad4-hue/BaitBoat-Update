# Flytec V080 Bait Boat Retrofit

**This GitHub repository is the canonical source of truth for the project.**

## Project layout
- `Boat/` — boat firmware, configuration, docs and releases.
- `Remote/` — remote firmware, UI, docs and releases.
- `docs/` — shared architecture, decisions, safety, validation and workflow.

## Development workflow
Check repository files first; implement approved changes; update documentation; coordinate shared Boat/Remote interfaces; review and verify; increment releases without overwriting prior ZIPs. GitHub is authoritative; chat is the working interface.

## Baseline
- Retain Flytec V080 hull, 12 V 550 brushed motors, battery indicator and main switch; replace control electronics.
- Boat controller target: Heltec Wireless Tracker V1.2 (ESP32-S3 + UC6580 GNSS).
- Remote: Waveshare ESP32-S3-Touch-LCD-4.3 standard, 800×480, LVGL 9.6.0, Arduino IDE.
- Radio: nRF24L01+ PA/LNA both ends; initial 250kbps; 500m range goal, not guarantee.
- Motor driver: Cytron MDD20A candidate, current validation required.
- Sonar: CHIRP echogram/fish-arch module; interface/module selection pending.

## Safety
Motor/solenoid currents, battery/BMS, wiring/fuses, connector and pin assignments require measurement/validation. Outputs default OFF on reset/invalid control. Return-home on link loss only if navigation/system health are valid.

## Release names
`Flytec_Boat_verXXX.zip`, `Flytec_Remote_verXXX.zip`. Never overwrite releases.

## Remote power progress (2026-11-20)
- Protected 1S4P Li-ion pack; charger excluded.
- **5 V rail candidate adopted: TPS61023DRLR.** TI reference values: 1µH, 10µF input, 2×22µF output, 732k/100k feedback. Target 450mA continuous and 1A transient only; verify current, capacitance under bias, thermal, pinout and EN. No 1A continuous rating claimed.
- **3.3 V radio rail:** TPS63070, 1.5µH candidate, 470k/150k feedback (3.3067V nominal), capacitors per TI baseline; radio bursts and effective C require tests.
- Battery protection: BMS, 4A fast-acting fuse candidate, reverse P-MOS, ADC sense and NTC; exact parts pending.
- At 3V loaded, estimates: about 1.20A combined continuous (5V/450mA and 3.3V/300mA at assumed 90% efficiencies); about 2.33A simultaneous transient (5V/1A at assumed 85% plus radio). Estimates only.
- Native schematic, ERC, PCB, review and hardware bring-up remain incomplete.

Documents:
- `Remote/New_Handheld_Remote/PCB/BOOST_CONVERTER_SELECTION_REVIEW_R2.md`
- `Remote/New_Handheld_Remote/PCB/TPS61023_COMPONENT_CALCULATIONS_R1.md`
- `Remote/New_Handheld_Remote/PCB/TPS61023_PIN_NET_MAP_R1.md`
- `Remote/New_Handheld_Remote/PCB/BATTERY_INPUT_PROTECTION_DESIGN_R3.md`
- `Remote/New_Handheld_Remote/PCB/POWER_SHEET_CAPTURE_SPEC_R1.md`
- `Remote/New_Handheld_Remote/PCB/REMOTE_POWER_BOM_CANDIDATES_R1.md`
- `Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R4.md`
- `Remote/New_Handheld_Remote/PCB/TPS63070_PIN_NET_MAP_R3.md
- `Remote/New_Handheld_Remote/PCB/TPS61023_COMPONENT_CALCULATIONS_R1.md`
- `Remote/New_Handheld_Remote/PCB/TPS61023_PIN_NET_MAP_R1.md`
- `Remote/New_Handheld_Remote/PCB/TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md`
- `Remote/New_Handheld_Remote/PCB/BATTERY_INPUT_PROTECTION_DESIGN_R3.md`
- `Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R4.md`
- `Remote/New_Handheld_Remote/PCB/POWER_DISTRIBUTION_DESIGN_R3.md`
