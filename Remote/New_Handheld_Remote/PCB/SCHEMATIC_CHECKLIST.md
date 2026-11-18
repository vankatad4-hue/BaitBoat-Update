# Schematic Development Checklist

Native KiCad capture is pending. Component calculations and net-level drafts are design inputs, not completed sheets.

## Sheet plan
- [ ] 00_System_Block — battery, power rails, MCU/display, inputs and radio
- [ ] 01_Battery_Input — connector, fuse, reverse-polarity protection, pack sense and testpoints
- [ ] 02_5V_Boost — TPS61088; calculate/verify inductor, feedback, compensation, ILIM, FSW, SS and capacitor ratings
- [ ] 03_Radio_3V3 — TPS63070; feedback, 1.5 µH inductor candidate, capacitor bank, EN/PS-SYNC/VSEL/FB2/PG configuration
- [ ] 04_Controller_Interface — Waveshare board, power, data/programming and control signals
- [ ] 05_Display_Interface — exact board connector and interface documentation
- [ ] 06_Joysticks_Buttons — two analog joysticks, 16 buttons and input scanning
- [ ] 07_nRF24_Radio — SPI/control, dedicated 3.3 V supply, RF keepout and antenna connector
- [ ] 08_Connector_Pinouts — freeze connector parts and pin assignments
- [ ] 09_Testpoints — battery, 5 V, 3.3 V, enable, PG, ADC, ground and debug

## TPS63070 radio rail baseline
- R1 470 kΩ VOUT→FB; R2 150 kΩ FB→GND; 3.307 V nominal calculated.
- L_RADIO 1.5 µH Coilcraft XFL4020-152ME candidate.
- Input: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output: 3×22 µF 16 V X6S 0805 + 10 µF 0603; 76 µF nominal.
- VAUX: 100 nF.
- Configuration proposal: FB2 GND; VSEL static LOW; PS/SYNC HIGH power-save; EN defined with 10 kΩ pull-up; optional PG pull-up. Verify pin configuration and bias against TI datasheet before capture.
- Check effective capacitance at DC bias and temperature; component models must be verified.

## Scope note
Charging circuit is excluded from the current power schematic. Do not mark schematic or validation tasks complete until exact symbol/footprint, pin numbers, datasheets, ERC and review are checked. No fabrication-ready schematic or PCB is claimed.
