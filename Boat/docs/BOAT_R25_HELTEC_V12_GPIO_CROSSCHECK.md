# R25 — Heltec Wireless Tracker v1.2 GPIO cross-check (2026-11-21)

## Evidence and limitations
1. Heltec official manufacturer v1.1 schematic: https://resource.heltec.cn/download/Wireless_Tracker/Wireless_Tacker1.1/HTIT-Tracker_V0.5.pdf
2. Heltec official hardware update log documents v1.0 and v1.1 only: https://wiki.heltec.org/docs/devices/open-source-hardware/esp32-series/lora-32/wireless-tracker/hardware-update-log
3. Independent v1.2-targeting ESP-IDF firmware pin definitions: https://github.com/jbjardine/HTIT-Tracker-v1.2-forIDF/blob/main/components/board/board_pins.h
4. Heltec product page presently identifies original Wireless Tracker as **phaseout** and recommends Wireless Tracker V2: https://heltec.org/project/wireless-tracker/

Independent v1.2-targeted software agrees with official v1.1 on the principal signals below. This is DOCUMENTARY CROSS-CHECK, not proof of exact v1.2 PCB copper or boot-time states. No v1.2-specific manufacturer schematic or physical board verification was obtained. Do not freeze harness or energize actuators.

## Cross-checked onboard assignments
| Function | ESP32-S3 GPIO | Evidence |
|---|---|---|
| SX1262 NSS/SCK/MOSI/MISO | 8/9/10/11 | Heltec v1.1 + third-party v1.2 |
| SX1262 RESET/BUSY/DIO1 | 12/13/14 | Same |
| UC6580 GNSS TX-to-MCU RX / RX-to-MCU TX | 33/34 | Same |
| GNSS RESET/PPS | 35/36 | Same |
| GNSS D_SEL / BOOT_MODE | 48/47 | Same |
| TFT CS/RESET/DC/SCK/MOSI | 38/39/40/41/42 | Same |
| TFT backlight | 21 | Same |
| VEXT power enable | 3 | Same; v1.1 GNSS control |
| Battery ADC/control | 1/2 | Same |
| User button / LED | 0/18 | Same |

## External signal resource audit
Provisional candidates GPIO4/5/6/7/17/26 = 6 pins, **not approved** until exact board and startup states verified. Do not assume GPIO15/16 (32k crystal), GPIO19/20 (USB), GPIO43/44 (console), GPIO45/46 (strapping), GPIO47/48 (GNSS controls), GPIO37 (power or PPS depending revision), or any onboard peripheral signal is free. The six candidates do not satisfy the R22 eight-direct-signal plan without changing the interface allocation.

Recommended engineering direction: nRF24 CE/CSN and two motor PWM pins on four verified native outputs; move DIR1/DIR2 and slow I2C requests to commercial I2C expander **only after** verifying safe reset states, update timing and independent motor power interlock. I2C SDA/SCL consume the remaining two candidate pins. This 6-pin budget is a *hypothesis*, not approved wiring. nRF24 SPI bus still requires a verified shared-bus design with SX1262 or alternate pins; CS exclusivity and TFT SPI separation must be tested. Motor safety must not depend on the expander's power-on state.

## Actionable blockers
- Request the exact **Wireless Tracker v1.2** schematic or written pin-function confirmation from Heltec.
- Verify physical silkscreen PCB revision, header positions, continuity, power rails and boot waveforms on actual v1.2 board.
- Verify nRF24 SPI-sharing with integrated SX1262 and coexistence with TFT.
- Verify MDD20A control interface and COTS independent STOP before enabling any actuator.
- Confirm availability of phased-out original Tracker before purchase; **do not silently substitute V2**, whose pin map differs.
