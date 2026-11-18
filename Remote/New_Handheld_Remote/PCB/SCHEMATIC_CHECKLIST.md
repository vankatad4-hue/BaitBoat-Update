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
