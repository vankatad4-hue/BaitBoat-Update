# ESP32 sonar coprocessor recommendation R2

**Date:** 2026-11-28
**Status:** use the owner's existing **ESP32-DevKitC V4** as the sonar-only controller candidate. Not yet wired or commissioned. This does not enable a sensor driver or change the active Boat pin map.

## Board fit

The linked Espressif board is ESP32-DevKitC V4 with an ESP32-WROOM-32E module; Espressif lists its 4 MB flash variant as ESP32-DevKitC-32E. The 4 MB flash is sufficient for this small dedicated acquisition firmware and a bounded echo buffer. The classic ESP32 supports ADC continuous/DMA acquisition. Open Echo specifies 13.2 μs between samples (about 75.8 ksample/s); Espressif's FAQ says 2 MHz is theoretical with Wi-Fi stopped and recommends lower practical rates. This makes the stated sample rate plausible, but actual interval, dropped samples, ADC quality and interference still need measurement.

Use the exact pin names from the owner's linked [ESP32-DevKitC V4 pinout](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html#pin-layout). The proposed allocation below avoids the documented SPI-flash pins GPIO6–11 and boot-strapping pins GPIO0, 2, 5, 12 and 15. It also leaves Wi-Fi/Bluetooth off during sampling. The board has enough resources for this sonar-only workload; there is no need to buy the previously proposed ESP32-S3 N8R8 for the prototype.

## Candidate Open Echo wiring

Open Echo Shield 002 is designed around the Arduino UNO pinout. This mapping is a direct-wire proposal for the ESP32-DevKitC V4; it is not plug-and-play. Verify the exact shield schematic, transducer and signal voltages before connection.

| Open Echo signal | ESP32-DevKitC V4 candidate | Board function | Notes |
|---|---:|---|---|
| VOUT | GPIO34 | ADC1_CH6, input-only | Suitable analog input. Measure the shield's VOUT maximum first; scale/condition it if needed. |
| IO1 | GPIO25 | Digital control | Confirm safe startup level and timing. |
| IO2 | GPIO26 | Digital control | Confirm safe startup level and timing. |
| OUT3 | GPIO27 | Digital input | Confirm output high voltage before connection. |
| OUT4 | GPIO32 | Digital input / ADC1_CH4 | Use as digital input. |
| TUSS CS | GPIO21 | Dedicated chip-select | Keep separate from SPI flash pins. |
| SPI SCK | GPIO18 | SPI clock | Keep at or below the TUSS4470 500 kHz limit. |
| SPI MISO | GPIO19 | SPI input | Verify shield logic-high voltage. |
| SPI MOSI | GPIO23 | SPI output | Configure SPI mode explicitly. |
| Coprocessor UART TX | GPIO22 | UART output | One-way 3.3 V serial data to Boat. |
| Signal ground | GND | Logic reference | Join logic grounds; keep transducer drive return away from MCU signal wiring. |

GPIO25–27 are ADC2-capable pins on the classic ESP32, but they are only used as digital GPIOs here. Keep Wi-Fi disabled during acquisition. Configure the SPI pins explicitly rather than relying on board defaults.

## Boat telemetry connection

Start by sending compact depth-only NMEA DBT over a one-way UART from coprocessor GPIO22 TX to Boat GPIO21 RX, with a shared logic ground. Boat GPIO21 is currently documented for optional nRF IRQ, but the current `NRF24Link` code polls the radio and does not consume the IRQ pin. Reassigning GPIO21 still requires updating the canonical Boat pin map, radio macros, crosswalk, source guards and resource checks as one reviewed change. Do not connect the wire until those records agree.

The current Boat DBT decoder can accept a bounded byte stream, but it is not yet attached to this UART. The Remote receives compact depth telemetry only; full 1,800-sample echo profiles and Lucky D+T water temperature need separate protocol work and are outside this depth-only first connection.

## Electrical and acquisition gates

1. Confirm Shield 002 revision, component selection for the 200 kHz transducer, transducer model/polarity and connector orientation.
2. Measure TUSS VDD, IO1/IO2, OUT3/OUT4 and VOUT across reset and capture. Do not connect any signal above ESP32 input limits; add level conversion or analog conditioning where required.
3. Power the shield's transducer-driver rail only from its documented VIN/XT30 input. Do not use the ESP32 3.3 V rail or GPIOs for transducer drive. Respect the shield's 28 V maximum and manage sonar return currents separately from motors.
4. Port acquisition to ESP32 ADC continuous/DMA with bounded buffering and overflow detection. Demonstrate the intended 13.2 μs interval and no dropped samples across 1,800 samples before calibrating depth.
5. Validate depth against known distances and account for water sound speed. DBT does not carry water temperature.
6. Bench-test the UART receive path with GNSS and radios scheduled; maintain STOP and control handling priority.

## References

- [Espressif ESP32-DevKitC V4 user guide and pinout](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html#pin-layout)
- [Espressif ADC continuous/DMA driver](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc/adc_continuous.html)
- [Espressif ADC sampling-rate guidance](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/adc.html)
- [Open Echo TUSS4470 Shield 002 guide](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/README.md)
- [Open Echo current sample timing](https://github.com/Neumi/open_echo)
- [TI TUSS4470 datasheet](https://www.ti.com/lit/ds/symlink/tuss4470.pdf)

## Software implementation status

The separate `Sonar/` PlatformIO target now records this DevKitC V4 pin
candidate and builds only a disarmed USB diagnostic scaffold. It deliberately
does not configure the shield pins, start the transducer, or emit DBT until the
electrical and acquisition gates above are cleared.

The upstream Open Echo UNO code is not a drop-in ESP32 implementation: it uses
AVR Timer1 and ADC registers. Also, `NMEA_DBT_OUT.ino` declares an analog sample
buffer but does not store ADC values into it; the depth output can therefore be
stale or zero. The raw-data sketch captures samples but still needs a timed ESP32
ADC/DMA port and validation before it can provide Boat telemetry.
