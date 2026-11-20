# Schematic Development Checklist

Native KiCad capture pending; calculation/net drafts are not completed schematics.

## Sheet plan
- [ ] 00_System_Block — battery, rails, controller, display, inputs, radio
- [ ] 01_Battery_Input — pack/BMS interface, fuse, reverse protection, ADC/NTC, testpoints
- [ ] 02_5V_Boost — TPS61023DRLR; reference components documented, pin/EN, bias and current verification pending
- [ ] 03_Radio_3V3 — TPS63070, 1.5µH candidate, cap bank and configuration review; capture/tests pending
- [ ] 04_Controller_Interface — Waveshare power/interface/programming
- [ ] 05_Display_Interface — exact board interface
- [ ] 06_Joysticks_Buttons — two analog joysticks, 16 buttons
- [ ] 07_nRF24_Radio — SPI/control, 3.3V rail, RF keepout/antenna
- [ ] 08_Connector_Pinouts — freeze part numbers/pinouts
- [ ] 09_Testpoints — battery, rails, EN/PG, ADC, GND/debug

## Design documents
- TPS61023: TPS61023_COMPONENT_CALCULATIONS_R1.md; TPS61023_PIN_NET_MAP_R1.md
- TPS63070: TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md; TPS63070_PIN_NET_MAP_R2.md
- Battery: BATTERY_INPUT_PROTECTION_DESIGN_R2.md
- Overall nets: POWER_SCHEMATIC_NET_DRAFT_R3.md; POWER_DISTRIBUTION_DESIGN_R3.md

## Baseline
- 5V boost: TPS61023DRLR, 1µH, 10µF input, 2×22µF output, 732k/100k divider; 450mA continuous plan, 1A transient target. Verify pinout/EN/cap bias/thermal.
- Radio rail: TPS63070, 1.5µH candidate, 470k/150k, capacitor bank, 100nF VAUX.
- Pack: 1S4P, BMS, 4A time-delay fuse candidate, reverse P-MOS, ADC, NTC; parts not frozen.

## Verification
- [ ] Datasheet symbol/footprint/pins
- [ ] Verify feedback and enable circuits
- [ ] Effective capacitance at bias/temp
- [ ] Peak-current and inductor review
- [ ] Final protection, connector and wiring parts
- [ ] Native KiCad schematic, ERC, independent review
- [ ] PCB layout/DFM and thermal/RF review
- [ ] Bench tests logged

Charging remains excluded. No fabrication-ready schematic/PCB claimed until capture, ERC, review and verification.
