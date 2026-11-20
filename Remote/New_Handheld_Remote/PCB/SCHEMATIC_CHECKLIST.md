# Schematic Development Checklist R2

Native KiCad capture pending. Net assignments are drafts, not completed schematics.

## Sheet plan
- [ ] 00_System_Block — pack, rails, controller, display, inputs, radio
- [ ] 01_Battery_Input — pack/BMS, fuse, reverse protection, ADC/NTC, testpoints
- [ ] 02_Controller_Power — onboard CS8501 battery-to-5V; verify connector/load performance
- [ ] 03_Radio_3V3 — TPS63070 dedicated rail; verify pin map/capacitance/config
- [ ] 04_Analog_Controls — ADS1115 joystick ADC
- [ ] 05_I2C_Button_Expander — MCP23017 buttons @0x20
- [ ] 06_I2C_Radio_Control — MCP23017 CE/CSN/IRQ candidate @0x21; validate timing
- [ ] 07_Controller_Interface — Waveshare PORT1/shared buses
- [ ] 08_Joysticks_Buttons — 2 joysticks, 16 buttons
- [ ] 09_nRF24_Radio — shared SPI, controls, power, RF keepout
- [ ] 10_Connector_Pinouts — parts/keying/pinout
- [ ] 11_Testpoints — battery, rails, ADC/NTC, debug

## Verified facts
Waveshare ESP32-S3-Touch-LCD-4.3 Standard schematic PORT1 mapping reviewed. GPIO11/12/13 TF SPI; GPIO8/9 I2C; GPIO6 Sensor AD; GPIO15/16 RS485/CAN; GPIO19/20 USB/CAN mux; GPIO43/44 UART0. LCD/touch pins reserved. GPIO35–37 unused pending module memory-routing confirmation.

## Proposed peripheral assignments
See WAVESHARE_HEADER_MAPPING_AND_PIN_ASSIGNMENTS_R2.md. ADS1115 @0x48 axes; MCP23017 @0x20 buttons; MCP23017 @0x21 radio control candidate; shared SPI GPIO11/12/13; I2C GPIO8/9. Radio CE/CSN/IRQ via expander timing is not verified.

## Power baseline
Protected 1S pack feeds Waveshare battery header; onboard CS8501 boosts to 5V. Test startup, load, ripple/droop and thermal; external boost fallback only. Radio TPS63070 rail remains dedicated. Protection candidates: BMS, 4A fast fuse, FDS6575 reverse P-MOS (Drain input, Source protected output), ADC, NTC; all require validation.

## Before PCB
- [ ] Verify physical board revision/header orientation.
- [ ] Verify GPIO35–37 memory usage.
- [ ] I2C scan/pullups/capacitance/rise time.
- [ ] Shared SPI test with radio + TF card, CS isolation and bus arbitration.
- [ ] MCP radio control timing/throughput test; redesign if needed.
- [ ] ADS1115 axis sampling/latency and analog filters.
- [ ] Radio-rail transient and thermal test.
- [ ] Battery-monitor circuit/assignment.
- [ ] Connector and harness pin-1/keying finalized.
- [ ] Native KiCad capture/ERC/footprint checks/independent review.
