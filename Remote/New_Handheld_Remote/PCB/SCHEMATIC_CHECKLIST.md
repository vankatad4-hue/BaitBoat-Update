# Schematic Development Checklist

Native KiCad capture pending; net-level drafts are not completed schematics.

## Sheet plan
- [ ] 00_System_Block — pack, rails, controller, display, inputs, radio
- [ ] 01_Battery_Input — pack/BMS, fuse, reverse protection, ADC/NTC, testpoints
- [ ] 02_Controller_Power — Waveshare onboard CS8501 battery-to-5V boost; verify battery connector and load performance
- [ ] 03_Radio_3V3 — TPS63070 dedicated rail; verify pin map/capacitance/config
- [ ] 04_Analog_Controls — ADS1115 joystick ADC and MCP23017 button expander
- [ ] 05_Controller_Interface — Waveshare module interfaces/programming
- [ ] 06_Display/Touch — board interface documentation
- [ ] 07_Joysticks_Buttons — two joysticks, 16 buttons (ADC/expander hardware) 
- [ ] 08_nRF24_Radio — SPI/control, power, RF keepout/antenna
- [ ] 09_Connector_Pinouts — exact connectors and pin assignments
- [ ] 10_Testpoints — pack, rails, ADC/NTC, debug

## Power documents
- Controller power architecture: 5V_CONTROLLER_POWER_ARCHITECTURE_REVIEW_R1.md
- External TPS61023 fallback: BOOST_CONVERTER_SELECTION_REVIEW_R3.md and TPS61023 calculation/pin map files
- Radio rail: TPS63070 calculations R2; pin map R3
- Battery protection: BATTERY_INPUT_PROTECTION_DESIGN_R4.md
- Capture guide: POWER_SHEET_CAPTURE_SPEC_R4.md; REMOTE_POWER_AND_CONTROL_BLOCK_DIAGRAM_R1.md
- BOM: REMOTE_POWER_BOM_CANDIDATES_R4.md
- Net draft: POWER_SCHEMATIC_NET_DRAFT_R6.md; POWER_DISTRIBUTION_DESIGN_R4.md; REMOTE_PERIPHERAL_INTERFACE_PINOUT_DRAFT_R1.md; WAVESHARE_GPIO_RESOURCE_AUDIT_R1.md

## Baseline
- Controller/display: Waveshare battery header fed from protected 1S pack; onboard CS8501 boost provides 5V. Test 450mA board load and transients. External boost is fallback only.
- Radio: TPS63070, 1.5µH candidate, 470k/150k, cap bank, 100nF VAUX; EN reset-safe.
- Protection: BMS, 4A fast fuse candidate, FDS6575 reverse P-MOS (Drain=input, Source=protected output), ADC, NTC. All need review/validation.

## Verification
- [ ] Confirm exact board revision and battery connector polarity/power path.
- [ ] Validate onboard boost startup, 5V rail droop/ripple, load and thermal.
- [ ] Verify converter symbols, footprints and pins.
- [ ] Check feedback/enable/default states.
- [ ] Effective capacitor bias and inductor current.
- [ ] Final pack/BMS/fuse/MOSFET/wire/connector ratings.
- [ ] Native KiCad capture and ERC.
- [ ] Independent schematic review.
- [ ] PCB layout, DFM, thermal/RF review.
- [ ] Bench bring-up and logs.

Charging is excluded. No fabrication-ready schematic/PCB claimed.
