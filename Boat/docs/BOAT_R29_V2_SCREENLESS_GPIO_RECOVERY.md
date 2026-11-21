# R29 — Wireless Tracker V2, screenless GPIO recovery
Date: 2026-11-21. Design candidate, NOT wiring release.

## Manufacturer evidence
Official V2 datasheet, page 8 pin map: https://resource.heltec.cn/download/Wireless_Tracker_V2/Wireless_Tracker_v2_Datasheet/Wireless%20Tracker%20v2.pdf
V2 schematic: https://resource.heltec.cn/download/Wireless_Tracker_V2/Schematic/HTIT-Tracker-FEM_V2.2.pdf

User will NOT use onboard TFT. Display is still physically wired; not using its software driver does not electrically disconnect it.

## Provisional allocation (nine native GPIO)
| Signal | GPIO | Critical constraint |
|---|---|---|
| nRF SPI SCK | 41 | Also TFT SCLK; display CS must be inactive |
| nRF SPI MOSI | 42 | Also TFT SDIN |
| nRF SPI MISO | 40 | Also TFT RS/DC; verify TFT input high impedance and signal integrity |
| nRF CE | 43 | UART0 TX reassigned; move diagnostics to USB CDC |
| nRF CSN | 44 | UART0 RX reassigned; check boot ROM and strap behavior |
| I2C SDA | 6 | external pull-up to 3.3V |
| I2C SCL | 17 | external pull-up to 3.3V |
| MDD20A left PWM | 21 | TFT backlight control attached: LED current/drive and startup state must be evaluated |
| MDD20A right PWM | 37 | Verify no hidden onboard circuit/USB or flash conflict |
| TFT CS hold inactive | 38 | Must be actively safe at boot, verify pull-up / screen never selected |
| TFT RESET hold inactive | 39 | Verify reset polarity and boot default; screen remains connected |

All header mappings and boot states require exact V2 board revision verification. nRF24 SPI MISO on GPIO40 is a software-mapped SPI input, but GPIO40 is also TFT DC input, not TFT output. TFT SPI clock/data may toggle, so only TFT CS held inactive prevents display transactions. GPIO21 PWM also drives TFT backlight circuitry: **not approved for motor PWM until load and electrical coupling are characterized**.

## Remaining controls
- MDD20A DIR1/DIR2 via vetted commercial I2C expander; safe boot defaults and independent motor power contactor mandatory.
- EKM014 four requests via vetted I2C expansion and independent hardware onehot / pulse limiter; no coil power until qualified.
- I2C compass / BMP280 coexistence and addresses need verification.
- LoRa integrated SX1262 reserved GPIO8–14 and RF PA control GPIO4/5/7 unchanged.
- GNSS reserved GPIO33–36, VEXT control GPIO3 unchanged.
- GPIO43/44 console removed; require USB-CDC logging, bootloader behavior test, no unexpected boot-time toggles on nRF CE/CSN.
- No direct 3S power to board; commercial 3S-to-5V supply, radio 3.3V separate filtered rail.

## Fail-closed acceptance tests
1. Official V2.2 schematic net trace for TFT CS pull resistor and display supply, TFT reset default, TFT backlight driver and GPIO21 current.
2. Confirm GPIO43/44 are safe during ROM boot and reset. External nRF CE pulldown and CSN pullup (using commercially assembled module/cable, no custom PCB), or different pins.
3. Scope GPIO21/37 PWM with TFT attached and all other peripherals enabled.
4. Check native SPI signal integrity and nRF packet reliability under concurrent GNSS + LoRa + I2C operation.
5. Validate I2C expander POR defaults and MDD20A direction logic, plus independent actuator power cut-off.
6. Confirm exact ready-made wiring adapters and safety hardware; no second MCU or custom PCB.

## Decision
**Conditional recovery of R26's 9-pin shortage appears possible without I2C-to-SPI bridge** if TFT pins and UART0 pins are reusable electrically. Not verified physically, not final GPIO assignment. Screenless V2 is now the preferred migration candidate for validation; R28 conservative pin estimate superseded only for the screenless configuration. Sonar deferred, official releases held.
