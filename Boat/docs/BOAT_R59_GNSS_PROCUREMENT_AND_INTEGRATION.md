# R59 — HGLRC M100 PRO GNSS verification (2026-11-22)

## Exact part and availability
- HGLRC M100 PRO GPS with QMC5883L compass (not M100 MINI). Official manufacturer: https://www.hglrc.com/products/hglrc-m100-pro-gps ; advertised USD 19.99, **out of stock** when checked.
- Baltic Drones exact M100 PRO SKU 6565 https://balticdrones.lt/en/products/gps-module-hglrc-m100-pro ; listed **€24.22 in stock** when checked. Shipping to Bulgaria not confirmed.
- Bulgarian specialist Drones.bg listing: https://drones.bg/en/magazine/racing/gps-compass/hglrc-m100-pro-gps-with-compass/ ; verify stock and cart price before purchase.
- Manufacturer specs: B101 GNSS, 115200 baud UBX, 10Hz, 3.6–5.5V input, 21.2 x 21.2 x 8.2mm, QMC5883L compass, supplied SH1.0 6-pin cable.

## Critical integration checks
- M100 PRO GNSS serial UART RX/TX at 115200, compass is **QMC5883L on I2C**; GNSS UART does not provide the compass heading by itself.
- Confirm actual cable pinout and I2C voltage levels on purchased board before wiring to ESP32-S3. Use manufacturer wiring diagram and multimeter, not connector wire colors alone.
- The GY-91 MPU9250 includes its own magnetometer AK8963. Validate whether both compasses are required, sensor offsets, orientation and mutual interference before figure-eight calibration. Code currently labels second calibration as M100.
- GNSS reception near motors and RF transmitters, cold-start, heading and 3m arrival radius require real field validation.
- **No hardware purchased or connected**. No software activation, no propulsion approval.

## Remaining purchase blockers
Exact E22 UART LoRa SKU and Remote-compatible PHY, nRF24 PA/LNA power, authentic GY-91 module, independent DC-rated STOP/contactor/watchdog/coil pulse limiter, current sensors, 5V/3.3V supplies and motor stall current. Do not order until exact ratings verified. Remote SD only; sonar deferred.
