# Schematic Development Checklist

Native KiCad capture pending; component/net drafts are not completed schematics.

## Sheet plan
- [ ] 00_System_Block — battery, rails, controller, display, inputs, radio
- [ ] 01_Battery_Input — pack/BMS, fuse, reverse protection, ADC/NTC, testpoints
- [ ] 02_Controller_Power — Waveshare onboard CS8501 battery-to-5V boost; verify battery input and load capability
- [ ] 03_Radio_3V3 — TPS63070; candidate inductor/caps and config reviewed; capture/tests pending
- [ ] 04_Controller_Interface — Waveshare power/interface/programming
- [ ] 05_Display_Interface — exact board interface
- [ ] 06_Joysticks_Buttons — two analog joysticks and 16 buttons
- [ ] 07_nRF24_Radio — SPI/control, 3.3V, RF keepout/antenna
- [ ] 08_Connector_Pinouts — freeze part numbers/pinouts
- [ ] 09_Testpoints — battery, rails, EN/PG, ADC, GND/debug

## Power documents
- 5V controller power: 5V_CONTROLLER_POWER_ARCHITECTURE_REVIEW_R1.md; TPS61023 fallback study retained
- TPS63070: TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md; TPS63070_PIN_NET_MAP_R2.md
- Protection: BATTERY_INPUT_PROTECTION_DESIGN_R4.md
- Capture guide: POWER_SHEET_CAPTURE_SPEC_R3.md
- Candidate BOM: REMOTE_POWER_BOM_CANDIDATES_R3.md
- Net draft: POWER_SCHEMATIC_NET_DRAFT_R6.md; POWER_DISTRIBUTION_DESIGN_R4.md; 5V_CONTROLLER_POWER_ARCHITECTURE_REVIEW_R1.md; TPS63070_PIN_NET_MAP_R3.md

## Current baseline
- 5V controller supply: use Waveshare onboard CS8501 from single-cell battery header. Validate 450mA board load and transients; TPS61023 is fallback only.
- Radio TPS63070: 1.5µH candidate, 470k/150k feedback, cap bank, 100nF VAUX.
- Pack: 1S4P; BMS; 4A fast-acting fuse candidate; FDS6575 reverse P-MOS (Drain input, Source output) candidate; ADC/NTC; all protection coordination pending.

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
