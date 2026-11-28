# ESP32 sonar coprocessor recommendation R1

**Branch:** `platformio-unified-r30h20`
**Status:** board candidate for engineering evaluation; not ordered, wired, or commissioned. This recommendation does not enable a sensor driver or change the active pin map.

## Recommendation

Use a second **Espressif ESP32-S3-DevKitC-1-N8R8** as the dedicated sonar acquisition controller for the Open Echo TUSS4470 Shield 002. Its ESP32-S3-WROOM-1-N8R8 module has 8 MB flash, 8 MB octal PSRAM, and 3.3 V SPI. The dev board exposes the MCU signals on headers. The PSRAM is not required for the sonar buffer; this exact variant is recommended because it is a current documented 3.3 V SPI option and has ample header GPIO for the shield without taking Boat motor/radio/sensor pins.

Espressif documents PlatformIO board ID `esp32-s3-devkitc-1`; the N8R8 flash/PSRAM configuration still needs to be set and compile-checked for the chosen PlatformIO platform. TME's current listing identifies the exact N8R8 part at €16.15 including VAT, but shows zero stock, so treat that as indicative pricing rather than an orderable offer.

## Why this board fits the acquisition job

- The Open Echo shield needs SPI, four digital control/status signals, and an analog input. The shield guide targets Arduino UNO and says its pin/software mapping is not compatible with other Arduino boards, so the S3 needs a new board-specific firmware port; the shield is not plug-and-play.
- Open Echo's current project README describes 1,800 analog samples at 13.2 µs per sample. That interval is approximately 75.8 ksample/s. ESP-IDF provides continuous ADC acquisition with driver-managed conversion frames and callbacks, which is the right architecture to evaluate instead of a blocking `analogRead()` loop. The achieved interval, sample loss, ADC linearity, and interference still need bench measurements; the ESP-IDF driver requires checking the chip's supported sample-frequency range and reports pool-overflow/read-rate failures.
- Put VOUT on an ADC1 pin. Espressif maps GPIO1–10 to ADC1; use one pin for the envelope and leave the rest of the ADC unused during capture. Keep Wi-Fi/Bluetooth disabled in the first acquisition firmware to minimize unrelated timing and power noise.
- The TUSS4470's SPI limit is 500 kHz. Configure the coprocessor at or below that limit, even if the shield's current UNO sketch selects a faster clock.

## Candidate coprocessor wiring

This is a firmware/wiring proposal only. Confirm the exact shield schematic/revision and signal levels before connection.

| Open Echo signal | Coprocessor candidate | Notes |
|---|---:|---|
| VOUT | GPIO4 / ADC1_CH3 | Measure VOUT range first; add the required input conditioning if it can exceed the S3 limit. |
| IO1 | GPIO5 | Digital control; confirm reset-safe level and timing. |
| IO2 | GPIO6 | Digital control; confirm reset-safe level and timing. |
| OUT3 | GPIO7 | Digital input; verify shield output voltage. |
| OUT4 | GPIO8 | Digital input; verify shield output voltage. |
| TUSS CS | GPIO10 | Dedicated chip-select. |
| SPI MOSI | GPIO11 | Configure the required mode and clock. |
| SPI SCK | GPIO12 | Run at or below 500 kHz. |
| SPI MISO | GPIO13 | Confirm shield output logic level. |
| Coprocessor UART TX | GPIO17 / UART TX | 3.3 V serial output to Boat. |
| Signal ground | GND | Join logic reference; keep transducer drive return out of MCU signal paths. |

The ESP32-S3 pins are flexible through the GPIO matrix, but this particular allocation must still be checked against the selected module/board revision and startup behavior. Avoid boot/strapping GPIO0 and GPIO3. Do not connect the shield's transducer-drive rail to an ESP32 supply, ADC, or GPIO.

## Boat data path

For the current Boat firmware, start with compact depth-only NMEA DBT output over a 3.3 V UART from the coprocessor to Boat. The existing Boat DBT decoder already accepts a bounded serial stream. A one-way UART needs coprocessor TX, Boat RX, and a shared logic ground; it does not need a second Boat UART pin if the boat never configures the sonar controller.

GPIO21 is the best Boat receive candidate for a later pin-map revision: current docs assign it to optional nRF IRQ, but `NRF24Link` polls the radio and its constructor does not consume the IRQ pin. Reassigning GPIO21 requires changing the canonical pin maps, hardware crosswalk, source guards, and pin-resource checks together, and physically confirming the header. Keep nRF polling enabled. Do not wire the UART until that revision is reviewed.

The existing DBT format carries depth only. Water temperature from the Lucky D+T transducer and full echo-profile data require separate validated output fields/protocol changes; the present Remote UI receives compact depth history and does not consume 1,800-byte raw profiles.

## Electrical and timing gates

1. Confirm the exact Open Echo shield revision, 200 kHz component selection, signal reference, and transducer part/polarity.
2. Measure TUSS VDD, IO1/IO2, OUT3/OUT4, and VOUT levels through startup and capture. Add level translation or attenuation before connecting any signal outside ESP32 limits.
3. Power the transducer driver only from the shield's documented driver supply input. Open Echo specifies an external VIN/XT30 path and a 28 V maximum; do not source that rail from the ESP32 board. Keep the sonar rail/return away from motor switching noise.
4. Implement hardware-timed ADC capture at the intended interval, bounded DMA buffering, and overflow detection. Measure the actual interval and dropped-sample rate at 1,800 samples per ping before tuning depth accuracy.
5. Test sonar bursts while Boat GNSS, both radios, and control scheduling run. Verify the new serial path does not delay STOP, heartbeat, radio, or sensor handling.
6. Validate depth against known distances in water and calibrate sound speed; do not infer water temperature from DBT.

## Decision boundary

The ESP32-S3-DevKitC-1-N8R8 is the recommended coprocessor candidate. It is not yet a locked purchase. No source pin assignment, UART connection, GPIO audit flag, firmware target, or hardware gate has changed. Select/lock it only after checking vendor availability, exact Open Echo board revision and transducer interface, and the physical Boat GPIO21 header.

## References

- [Espressif ESP32-S3-DevKitC-1 hardware guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html)
- [Espressif ESP32-S3 ADC continuous-mode guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32s3/api-reference/peripherals/adc_continuous.html)
- [Espressif ADC/GPIO mapping](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html)
- [PlatformIO ESP32-S3-DevKitC-1 board configuration](https://docs.platformio.org/en/latest/boards/espressif32/esp32-s3-devkitc-1.html)
- [Open Echo TUSS4470 Shield 002 guide](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/README.md)
- [Open Echo project README and raw sample timing](https://github.com/Neumi/open_echo)
- [TI TUSS4470 datasheet](https://www.ti.com/lit/ds/symlink/tuss4470.pdf)
- [TME ESP32-S3-DevKitC-1-N8R8 listing](https://www.tme.eu/ie/en/details/esp32s3devkc-8r8/development-kits-for-data-transmission/espressif/esp32-s3-devkitc-1-n8r8/)
