# R36 — Approved architecture change, uncommissioned hardware
Date: 2026-11-21. Owner requested update to HGLRC M100 PRO GPS, ESP32-S3 and E22.

## Selection
- Main boat processor: factory-built Espressif ESP32-S3-DevKitC-1, **exact N8/N8R8 or other variant unresolved**. Verify pin headers, boot strapping, flash/PSRAM reservations, supply.
- GNSS: HGLRC M100 PRO GPS, **verify exact compass-equipped variant and UART/I2C voltage levels**.
- Primary radio: nRF24L01+ PA/LNA, external SPI.
- Backup: EBYTE E22-900T22S **UART** LoRa candidate, to match remote candidate. Both ends must use compatible E22 UART configuration and real over-air test; the remote's current firmware and E22 physical interface are not yet proven compatible.
- Retain Cytron MDD20A, EKM014 (owner-frozen), two motors, four solenoids, 3S4P battery, modular independent STOP/contactor/pulse safety, GY-91 IMU only if needed after verifying M100 compass.
- No Heltec Tracker, no screenless TFT pin-sharing, no SC18IS606 bridge, no MCP23017 required solely for the former Tracker GPIO shortage. Existing R34/R35 expander code retained as experimental and **not integrated**.
- No custom PCB, no RP2040/second MCU. No sonar implementation until model chosen.

## Interfaces to engineer (no pins assigned)
nRF SPI SCK/MISO/MOSI/CE/CSN (5); GPS UART RX/TX (2); E22 UART RX/TX/M0/M1/AUX (5); MDD20A PWM2+DIR2 (4); IMU/GPS compass I2C SDA/SCL (2); EKM014 four requests (4); independent safety feedback/diagnostics (3+). Approx 25+ signals before optional sensor/ADC requirements. Board variant must expose sufficient safe GPIO; avoid USB, strap, flash/PSRAM and UART boot conflicts.

## Firmware migration blockers
The existing BoatDualRadioPort/HeltecSX1262Link and Boat sketch were written for the onboard SX1262. R36 **does not claim this compiles into an operational E22 boat firmware**. Implement E22 UART transport on boat, revise radio factory and tests, then compile and verify dual-radio sessions. Do not silently map E22 to SX1262 SPI.
M100 PRO GNSS driver must be verified for actual output sentence protocol and heading/compass source. Do not assume compass available on every SKU.

## Acceptance gates
1. Exact ESP32-S3 board SKU and manufacturer pin schematic.
2. Exact HGLRC M100 PRO variant and power/logic/compass pinout.
3. Exact E22 UART SKU and both ends RF mode/frequency/regulatory and framing interoperability.
4. Power supply and brownout/boot GPIO tests; nRF24 dedicated clean 3V3.
5. Factory-built independent STOP, contactor with feedback, watchdog and onehot solenoid pulse safety.
6. Non-actuating firmware build; software simulator tests; RF bench; physical STOP and fault tests.
No actuator energization or release publication before physical commissioning.
