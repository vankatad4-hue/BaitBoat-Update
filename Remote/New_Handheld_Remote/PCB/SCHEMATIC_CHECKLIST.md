# Schematic Development Checklist

Native KiCad capture pending; component/net drafts are not completed schematics.

## Sheet plan
- [ ] 00_System_Block — battery, rails, controller, display, inputs, radio
- [ ] 01_Battery_Input — pack/BMS, fuse, reverse protection, ADC/NTC, testpoints
- [ ] 02_5V_Boost — TPS61023DRLR; reference components documented; pins/EN, bias, current and thermal pending
- [ ] 03_Radio_3V3 — TPS63070; candidate inductor/caps and config reviewed; capture/tests pending
- [ ] 04_Controller_Interface — Waveshare power/interface/programming
- [ ] 05_Display_Interface — exact board interface
- [ ] 06_Joysticks_Buttons — two analog joysticks and 16 buttons
- [ ] 07_nRF24_Radio — SPI/control, 3.3V, RF keepout/antenna
- [ ] 08_Connector_Pinouts — freeze part numbers/pinouts
- [ ] 09_Testpoints — battery, rails, EN/PG, ADC, GND/debug

## Power documents
- TPS61023: TPS61023_COMPONENT_CALCULATIONS_R1.md; TPS61023_PIN_NET_MAP_R1.md
- TPS63070: TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md; TPS63070_PIN_NET_MAP_R2.md
- Protection: BATTERY_INPUT_PROTECTION_DESIGN_R3.md
- Capture guide: POWER_SHEET_CAPTURE_SPEC_R1.md
- Candidate BOM: REMOTE_POWER_BOM_CANDIDATES_R1.md
- Net draft: POWER_SCHEMATIC_NET_DRAFT_R3.md; POWER_DISTRIBUTION_DESIGN_R3.md

## Current baseline
- 5V boost TPS61023DRLR: 1µH, 10µF input, 2×22µF output, 732k/100k FB; 450mA continuous plan/1A transient target. Thermal and capacitor-bias checks pending.
- Radio TPS63070: 1.5µH candidate, 470k/150k feedback, cap bank, 100nF VAUX.
- Pack: 1S4P; BMS; 4A fuse candidate; FDS6575 reverse P-MOS candidate; ADC/NTC; all protection coordination pending.

## Validation
- [ ] Datasheet symbol/footprint/pin numbers.
- [ ] Verify FB, EN and control defaults.
- [ ] Check effective capacitance under bias/temp.
- [ ] Calculate peak current and inductor margin.
- [ ] Finalize BMS, fuse, reverse MOS, connectors and wiring.
- [ ] Capture native KiCad sheets and run ERC.
- [ ] Independent schematic review.
- [ ] PCB placement, routing, DFM, thermal and RF review.
- [ ] Bench bring-up and logged measurements.

Charging is excluded. No fabrication-ready schematic/PCB claimed.
