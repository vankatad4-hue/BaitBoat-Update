# R39 — ESP32-S3 modular wiring and physical acceptance
2026-11-21. NOT-YET-FROZEN wiring. Do not connect actuator supply.

## Exact hardware evidence required
- ESP32-S3-DevKitC-1: module variant, headers, strapping, USB, flash/PSRAM exclusions.
- HGLRC M100 PRO: actual SKU, power voltage, UART logic, baud and observed NMEA. Compass interface is variant-dependent. R39 GNSS profile denies unverified operation.
- E22-900T22S: UART SKU, 3V3 logic, supply peak, M0/M1/AUX and RF profile matching the remote. Existing fixed test-profile and estimated airtime are NOT certified.
- nRF24 PA/LNA: SPI SCK/MISO/MOSI/CE/CSN and dedicated clean 3V3 regulator.
- MDD20A: two native PWM, two DIR, independent motor supply isolation.
- EKM014: four request lines plus independent hardware onehot/nonretriggerable coil pulse limiter.
- I2C IMU/compass: check exact breakout voltage, pull-ups and address conflicts.

## Factory-made independent safety stack procurement gates
1. DC-rated normally-open 12V continuous-duty contactor, DC interrupt capacity for measured stall current, mechanically linked feedback.
2. Manual-reset dual-channel emergency stop monitor suitable for DC control circuit.
3. Independent watchdog/monostable contactor cutoff during crash, boot, brownout, STOP.
4. Hardware onehot coil driver and maximum non-retriggerable pulse effective even with stuck GPIO.
5. Fuses, transient protection, rated waterproof wiring and measured battery/BMS capacity.
Prior SW60C/A and Duelco NST-3.2 are candidates only, not approved exact SKUs. Software cannot replace hardware safety.

## Physical tests requiring assembled equipment
Measure rails and boot states with no actuator load; capture GPS NMEA; E22 bidirectional remote bench test; nRF primary link; stop/contactors fault injection; EKM014 LED-only onehot tests; then controlled isolated motor/coil current tests with appropriate safety equipment and operator. No physical testing has been performed here. No pin freeze or safety certification.
