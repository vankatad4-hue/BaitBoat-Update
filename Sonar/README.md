# Sonar coprocessor

Standalone PlatformIO target for the owner's 4 MB ESP32-DevKitC V4, intended
to host an Open Echo TUSS4470 Shield 002. The current target is only a
compile-time scaffold. It starts disarmed, does not configure sensor GPIO,
does not enable the Boat UART, and does not report a depth value.

## Build target

From the repository root, use `pio run -d Sonar -e devkitc_v4_compile` to compile
the disarmed scaffold. `pio run -d Sonar -e devkitc_v4_adc_api_compile` also
compiles the ESP32 ADC continuous/DMA adapter against the pinned framework API;
it does not instantiate or start the adapter. PlatformIO Upload is intentionally
denied by the shared `Boat/tools/pio_deny_upload.py` guard.

## Candidate pins

See [`src/sonar_pin_map.h`](src/sonar_pin_map.h) and the detailed
[Boat coprocessor recommendation](../Boat/docs/BOAT_ESP32_SONAR_COPROCESSOR_RECOMMENDATION_R2.md).
The pin constants are not yet an instruction to wire the shield.

`src/dbt_sentence.h` contains a side-effect-free integer formatter for valid
depths in Boat's accepted range. `src/adc_capture_buffer.h` provides a fixed
5,224-sample staging buffer with complete-block checks, invalid-ADC-sample
rejection, overflow reporting, and elapsed capture timing. The optional
`src/esp32_adc_continuous_capture.*` adapter configures ADC1 DMA on the GPIO34
candidate, decodes only the expected channel, and stops on exactly one full
sample block. Its VOUT hardware audit is hard-set false, and neither adapter is
instantiated by the disarmed firmware; no DBT sentence is currently sent.

## Work required before sensor firmware

1. Identify the exact ESP32 module and Shield 002 revision, selected capacitor
   jumpers, transducer model, and drive voltage.
2. Measure TUSS logic outputs and VOUT against the ESP32 input limits; add
   level conversion or analog scaling if measurements require it.
3. Integrate the capture adapter with a validated timer burst and future echo
   detector. Keep Wi-Fi disabled during capture and propagate ADC pool
   overflow, invalid channel records, stop errors, or incomplete captures as
   invalid results.
4. Validate the capture interval, depth threshold, blind zone, and sound-speed
   compensation against known distances before sending DBT.
5. Add the one-way DBT transmitter only after the coprocessor has a validated
   depth source. The Boat-side receiver now has a compile-only UART0/USB-CDC
   profile on the GPIO21 candidate and bounded DBT parsing, but its physical
   pin audit is unsigned and the default firmware cannot start it. Keep the
   transmitter disabled until both boards' pins and logic levels are checked.

Boat keeps `HardwareSerial(2)` for GNSS and `HardwareSerial(1)` for E22. The
isolated `boat_open_echo_uart_compile` profile places diagnostics on native USB
CDC and reserves UART0 RX/GPIO21 for the sonar DBT stream. The default Boat
profile does not enable the reader, its GPIO audit flag is still zero, and the
optional nRF IRQ is omitted because the current radio driver polls. The selected
coprocessor-to-Boat software contract is 9600 baud, 8-N-1, one-way TX-to-RX,
with a shared logic ground; this is a candidate wiring plan, not approval to
connect it.

Both boards have integrated 2.4 GHz Wi-Fi, but the sonar target keeps Wi-Fi
disabled during ADC capture and uses the wired UART candidate for compact DBT
telemetry. Wi-Fi is not currently implemented as a sonar transport; any future
use must be assessed alongside the Boat nRF24 RF link and measured capture
quality.

The upstream Open Echo UNO sketch is a reference, not portable ESP32 firmware:
it uses AVR Timer1 registers and AVR ADC registers. The NMEA DBT sketch declares
an ADC sample buffer but does not populate it, so its reported depth index may
be stale or zero. The raw-data example does capture samples, but its timing and
depth detection still need an ESP32 implementation and bench validation.

The ADC adapter uses Espressif's `esp_adc/adc_continuous.h` interface available
with this repository's PlatformIO espressif32 8.0.0 framework package. It asks
for 75.8 ksample/s as a candidate rate; actual interval, sample loss and
voltage accuracy must be measured on the exact DevKitC and shield.

## Official API references

- [ESP-IDF 5.5.5 ADC continuous mode driver](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32/api-reference/peripherals/adc/adc_continuous.html)
- [PlatformIO Espressif32 8.0.0 release notes](https://github.com/platformio/platform-espressif32/releases/tag/v8.0.0) — Arduino-ESP32 3.3.12 based on ESP-IDF 5.5.5.
