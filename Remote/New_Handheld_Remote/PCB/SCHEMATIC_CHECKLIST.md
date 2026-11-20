# Schematic Development Checklist

Native KiCad capture is pending. Component calculations and net-level drafts are design inputs, not completed sheets.

## Sheet plan
- [ ] 00_System_Block — battery, power rails, MCU/display, inputs and radio
- [ ] 01_Battery_Input — pack connector, BMS interface, fuse, reverse-polarity protection, pack sense/NTC and test points
- [ ] 02_5V_Boost — TPS61088; calculation document R2 added; compensation, exact capacitor bias, CAD pin mapping and ERC still pending
- [ ] 03_Radio_3V3 — TPS63070; datasheet checked, 1.5 µH inductor candidate, capacitor bank and pin/configuration reviewed; CAD capture and validation pending
- [ ] 04_Controller_Interface — Waveshare board, power, data/programming and control signals
- [ ] 05_Display_Interface — exact board connector and interface documentation
- [ ] 06_Joysticks_Buttons — two analog joysticks, 16 buttons and input scanning
- [ ] 07_nRF24_Radio — SPI/control, dedicated 3.3 V supply, RF keepout and antenna connector
- [ ] 08_Connector_Pinouts — freeze connector parts and pin assignments
- [ ] 09_Testpoints — battery, 5 V, 3.3 V, enable, PG, ADC, ground and debug

## Power-design documents
- TPS61088: TPS61088_COMPONENT_CALCULATIONS_R2.md; TPS61088_PIN_NET_MAP_R2.md
- TPS63070: TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md; TPS63070_PIN_NET_MAP_R2.md
- Battery: BATTERY_INPUT_PROTECTION_DESIGN_R1.md

## Current component baseline
- 5 V boost: TPS61088RHLR, 2.2 µH shielded inductor candidate, 177 kΩ/56 kΩ feedback, 150 kΩ FSW, 249 kΩ ILIM, 47 nF SS; compensation not frozen.
- 3.3 V radio buck-boost: TPS63070, 1.5 µH Coilcraft XFL4020-152MEC, R1 470 kΩ/R2 150 kΩ, TI capacitor bank, 100 nF VAUX.
- Battery protection: 1S4P pack, pack BMS, 4 A time-delay fuse candidate, high-side P-MOS reverse protection, voltage sensing and NTC; all component selection pending.

## Remaining capture/verification
- [ ] Exact symbols/footprints and pin numbers against datasheets.
- [ ] Finish TPS61088 compensation design.
- [ ] Verify capacitor DC-bias effective values.
- [ ] Finalize pack/BMS/fuse/reverse-protection component selection.
- [ ] Freeze connector part numbers and pinouts.
- [ ] Native KiCad sheets, ERC, independent review and PCB.
- [ ] Bench measurements logged before claiming validated.

## Scope note
Charging circuit remains excluded. Do not mark schematic or validation tasks complete until capture, exact symbol/footprint, ERC and review are checked. No fabrication-ready schematic or PCB is claimed.
