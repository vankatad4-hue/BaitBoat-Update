# R45 selected controller and provisional pin/power engineering schedule

**Frozen part:** Espressif ESP32-S3-DevKitC-1-N32R16V, 32 MB octal flash, 16 MB octal PSRAM, on-board PCB antenna. **Do not substitute** an N16 board or bare WROOM-2 without approval.

Sources: [Espressif DevKitC-1 v1.1 user guide](https://documentation.espressif.com/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html), [HGLRC M100 PRO](https://www.hglrc.com/products/hglrc-m100-pro-gps). Manufacturer pin header is J1/J3, **GPIO number is not header position**.

## Provisional functional GPIO map — not a signed wiring release

| GPIO | Function | Board header | Notes |
|---|---|---|---|
| 4 | M100 UBX UART RX (GPS TX) | J1-4 | GPS voltage and TX levels to verify |
| 5 | M100 UART TX (GPS RX) | J1-5 | Only if GNSS configuration needed |
| 7 | I2C SCL | J1-7 | GY-91, M100 compass, ADS1115; pullups to 3.3V only |
| 8 | I2C SDA | J1-12 | Confirm address map and total bus capacitance |
| 9 | nRF24 CSN | J1-15 | 3.3V RF supply |
| 10 | nRF24 CE | J1-16 | Default low via external pulldown |
| 11 | nRF24 SPI MOSI | J1-17 | Separate from external UART LoRa |
| 12 | nRF24 SPI SCK | J1-18 | |
| 13 | nRF24 SPI MISO | J1-19 | |
| 14 | E22 AUX | J1-20 | Confirm module output voltage |
| 15 | E22 M0 | J1-8 | Mode selection |
| 16 | E22 M1 | J1-9 | Mode selection |
| 17 | E22 RX (MCU TX) | J1-10 | External E22 UART |
| 18 | E22 TX (MCU RX) | J1-11 | External E22 UART |
| 39 | MDD20A PWM left | J3-9 | Hardware output DISABLED pending safety |
| 40 | MDD20A PWM right | J3-8 | Hardware output DISABLED pending safety |
| 41 | MDD20A DIR left | J3-7 | External safe default required |
| 42 | MDD20A DIR right | J3-6 | External safe default required |
| 47 | Watchdog heartbeat REQUEST | J3-17 | **Not** a replacement for independent watchdog |
| 21 | Motor permit REQUEST | J3-18 | **Not** a replacement for physical STOP |
| 6 | Physical STOP monitor INPUT | J1-6 | Independent STOP contactor still required |
| 1 | DC contactor auxiliary feedback INPUT | J3-4 | Must read actual contacts |
| 2 | Solenoid power feedback INPUT | J3-5 | External sensor needed |

GPIO0, GPIO3, GPIO45, GPIO46: reserve for boot/strapping and avoid safety outputs. GPIO19/20: reserve USB. GPIO43/44: reserve UART0 programming/diagnostics. GPIO35/36/37: avoid because octal memory variants may reserve them. GPIO38/48: reserve for board-revision-dependent RGB LED. Pins not listed remain unassigned.

Four EKM014 coil commands: **not assigned to bare GPIO**. Provisional factory-built MCP23017 I2C expander channels with hardware one-hot/pulse/no-repeat protection are required. This is NOT an approved direct connection: external independent safety gate is unresolved.

## Power distribution

3S4P Li-ion + 20A BMS -> appropriately DC-rated master fuse, isolation and protection -> (A) independently cuttable motor branch to MDD20A and motors, (B) independently cuttable coil branch to EKM014 VH/solenoids, (C) protected regulated 5V logic branch to DevKitC-1 5V, M100 (manufacturer 3.6–5.5V), and EKM014 VL if 5V is confirmed. Separate adequately rated regulated 3.3V RF branch for nRF PA/LNA and E22 only if exact E22 SKU is 3.3V-compatible. Common ground reference with star/high-current routing; do not backfeed USB and external 5V simultaneously. EKM014 power and coil return current must never pass through ESP32 ground traces.

Power, fuse and thermal current values cannot be frozen without measured motors' stall current, solenoid current/pulse width, E22 exact model, 5V module rating, and DC contactor ratings.

## Hold points
- Confirm **physical board revision** and RGB pin, USB behaviour, accessible header pins, boot strapping, voltage and octal-memory GPIO reservations.
- Confirm exact E22 model, pin voltage, LoRa modulation/protocol with remote, M100 QMC5883L vs QMC5883P, I2C 3.3V levels, GPS UBX NAV-PVT stream.
- Select independently certified DC-rated motor and coil STOP, watchdog, pulse one-hot, POR/reset and current-zero modules. No custom PCB.
- Do not wire actuator power or enable GPIO output on an uncommissioned boat.
