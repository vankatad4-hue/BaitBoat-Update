# Schematic Development Checklist

This folder will contain KiCad schematics when component selection and interfaces are validated.

## Sheet plan
- [ ] 00_System_Block — power, MCU, display, inputs, radios
- [ ] 01_USB_C_Charging — USB-C receptacle, CC, ESD, charger, power-path/load-sharing
- [ ] 02_Battery_Protection — pack protection, fuse, temperature sense, battery monitoring
- [ ] 03_Regulators — 5V boost and 3V3 buck/LDO rail calculations and parts
- [ ] 04_ESP32_S3_Main — module, boot/reset, USB data/programming, debug
- [ ] 05_Display_Interface — exact display connector and required signals
- [ ] 06_Joysticks_Buttons — two sticks, 16 switches, debounce/matrix or GPIO expander
- [ ] 07_nRF24_Radio — SPI, CE/CSN/IRQ, PA/LNA supply filtering and RF layout
- [ ] 08_ESP_RF — external antenna-supported ESP32 RF path/module details
- [ ] 09_Expansion — optional lights/camera/vibration/accessories
- [ ] 10_Connector_Pinouts — cable and board interfaces
- [ ] 11_Testpoints — battery, rails, USB, SPI, UART, RF and reset signals

Do not mark a sheet complete until electrical rules, component datasheets, connector pinout, and review have been checked. No schematic or PCB is fabrication-ready at this stage.

# Schematic Development Checklist

Native KiCad capture is pending. Component calculations and net-level drafts are design inputs, not completed sheets.

## Sheet plan
- [ ] 00_System_Block — battery, rails, display, inputs and radios
- [ ] 01_Battery_Input — connector, fuse, reverse-polarity protection, pack sense and testpoints
- [ ] 02_5V_Boost — TPS61088; calculate/verify inductor, feedback, compensation, ILIM, FSW, SS and capacitor ratings
- [ ] 03_Radio_3V3 — TPS63070; 470 kΩ/150 kΩ feedback, 1.5 µH inductor candidate, corrected TI capacitor baseline, EN/PS-SYNC/VSEL/FB2/PG
- [ ] 04_ESP32_S3_Main — Waveshare board interface, USB/programming and control signals
- [ ] 05_Display_Interface — exact board connector and interface documentation
- [ ] 06_Joysticks_Buttons — two analog joysticks, 16 buttons and input scanning
- [ ] 07_nRF24_Radio — SPI/control, dedicated filtered 3.3 V rail, RF keepout
- [ ] 08_Connector_Pinouts — freeze connector parts and pin assignments
- [ ] 09_Testpoints — battery, 5 V, 3.3 V, enable, PG, ADC, ground and debug

## TPS63070 current baseline
- R1 470 kΩ VOUT→FB; R2 150 kΩ FB→GND; nominal 3.307 V.
- L_RADIO 1.5 µH candidate.
- Input: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output: 3×22 µF 16 V X6S 0805 + 10 µF 0603; 76 µF nominal.
- VAUX: 100 nF.
- FB2 GND; VSEL static LOW; PS/SYNC HIGH power-save; EN defined by proposed 10 kΩ pull-up; optional PG pull-up. Verify datasheet pin configuration before capture.
- Previous 98 µF proposal superseded; check effective capacitance at DC bias and temperature.

Do not mark schematic or validation tasks complete until exact symbol/footprint, pin numbers, datasheets, ERC and review are checked. No fabrication-ready schematic or PCB is currently claimed.
