# Sonar coprocessor

Standalone PlatformIO target for the owner's 4 MB ESP32-DevKitC V4, intended
to host an Open Echo TUSS4470 Shield 002. The current target is only a
compile-time scaffold. It starts disarmed, does not configure sensor GPIO,
does not enable the Boat UART, and does not report a depth value.

## Build target

From the repository root, run these commands to compile the disarmed scaffold
and its isolated peripheral API profiles:

```sh
pio run -d Sonar -e devkitc_v4_compile
pio run -d Sonar -e devkitc_v4_adc_api_compile
pio run -d Sonar -e devkitc_v4_spi_api_compile
pio run -d Sonar -e devkitc_v4_uart_api_compile
```

The ADC, SPI, and UART profiles compile their adapters but do not instantiate or
start them. PlatformIO Upload is intentionally denied by the shared
`Boat/tools/pio_deny_upload.py` guard.

## Candidate pins

See [`src/sonar_pin_map.h`](src/sonar_pin_map.h) and the detailed
[Boat coprocessor recommendation](../Boat/docs/BOAT_ESP32_SONAR_COPROCESSOR_RECOMMENDATION_R2.md).
The pin constants are not yet an instruction to wire the shield.
The current implementation state and next software/hardware gates are tracked
in [`CONTINUATION.md`](CONTINUATION.md).

### Provisional DevKitC V4 allocation

The software allocation is now explicit and compile-time checked for duplicate,
input-only, and boot-strapping GPIO conflicts:

| Function | ESP32-DevKitC V4 GPIO | Direction |
| --- | ---: | --- |
| TUSS4470 IO1 | 25 | Output |
| TUSS4470 IO2 | 26 | Output |
| Shield OUT3 | 27 | Input |
| Shield OUT4 | 32 | Input |
| TUSS4470 chip select | 21 | Output |
| VSPI SCK / MISO / MOSI | 18 / 19 / 23 | Output / input / output |
| Shield VOUT envelope | 34 (ADC1) | Input-only |
| Sonar-to-Boat UART1 TX | 22 | Output, 9600-8N1 |

On Boat, the matching candidate is GPIO21/J3-18 as UART0 RX, only in the
USB-CDC diagnostic profile. The coprocessor GPIO22 TX, Boat GPIO21 RX, and
logic ground form the proposed one-way link. The two boards' GPIO numbers are
independent namespaces. All physical pin, signal-level, shield-header, and
ground checks remain unsigned; the compile-time allocation does not authorize
connecting the shield or Boat UART.

The candidate [Lucky wired D+T transducer](https://www.luckylaker.com/products/luckylaker-sensor-718lic)
is listed by its manufacturer as a 200 kHz, 45-degree, fish-finder-only
transducer. The public product information does not specify a temperature
element's wires, resistance curve, interface, or data protocol. Therefore the
TUSS profile and water-temperature input remain disabled until the actual
transducer documentation and wiring are available. The [Open Echo Shield 002
guide](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/README.md)
also requires matching the capacitor jumpers to the selected transducer.

`src/dbt_sentence.h` contains a side-effect-free integer formatter for valid
depths in Boat's accepted range. `src/esp32_dbt_uart_transmitter.*` is a
compile-gated 9600-8N1 UART1 sender that accepts only a `FOUND` detector result;
its GPIO22 audit remains unsigned and it is not instantiated.
`src/adc_capture_buffer.h` provides a fixed 5,224-sample staging buffer with
complete-block checks, invalid-ADC-sample
rejection, overflow reporting, and elapsed capture timing. The optional
`src/esp32_adc_continuous_capture.*` adapter configures ADC1 DMA on the GPIO34
candidate, decodes only the expected channel, and stops on exactly one full
sample block. Its VOUT hardware audit is hard-set false, and neither adapter is
instantiated by the disarmed firmware; no DBT sentence is currently sent.

`src/echo_profile_detector.*` consumes only a complete capture and requires an
explicit calibrated ADC threshold, blanking sample count, measured sample
interval, sound-speed value, and measured acoustic burst-origin sample. It
reports the first stable crossing after that origin as a depth estimate; it does
not use a default calibration, read the TUSS OUT4 comparator, or publish
telemetry. The ADC raw threshold is a different value from the TUSS4470 OUT4
threshold register and must be calibrated independently. The caller must
determine the acoustic origin's sample coordinate; the ADC stream is not
automatically synchronized to the transmit event.

`src/tuss4470_spi_codec.*` is a side-effect-free 16-bit MSB-first frame helper.
It has separate read and write encoders; reads always send the required `0x00`
data byte. The response decoder validates odd parity and separates the raw
status byte into VDRV-ready, burst-fault, EEPROM-fault, and device-state fields.
It does not configure SPI, toggle chip select, observe the required high
interval between frames, or read/write TUSS4470 registers. The bus protocol and
all candidate chip-select/SPI pins remain uncommissioned.

`src/esp32_tuss4470_transport.*` wraps the codec with one `SPI.transfer16`
frame per NCS pulse, 400 kHz SPI mode 1, a 3 μs NCS-high guard, write-address
echo checking, and explicit parity/status error returns. Its compile-only API
profile is `devkitc_v4_spi_api_compile`. Runtime remains unavailable: hardware
enable is compile-locked to zero and `TUSS_SPI_AUDIT_SIGNED` is false. The mode and guard
are software candidates that still require a scope check on the exact board and
shield.

## Work required before sensor firmware

1. Identify the exact ESP32 module and Shield 002 revision, selected capacitor
   jumpers, transducer model, and drive voltage.
2. Measure TUSS logic outputs and VOUT against the ESP32 input limits; add
   level conversion or analog scaling if measurements require it.
3. Integrate the capture adapter and pure profile detector with a validated
   timer burst and measured sample-zero/transmit alignment. Keep Wi-Fi disabled
   during capture and propagate ADC pool overflow, invalid channel records, stop
   errors, or incomplete captures as invalid results.
4. Implement the TUSS SPI transport around the pure frame codec: select the
   SPI mode/rate from the exact shield documentation, keep the clock at or below
   500 kHz, deassert NCS between every 16-bit frame, and handle parity/status
   errors. Confirm the shield's actual interface and logic levels before any
   bus or chip-select pin is enabled.
5. Validate the capture interval, depth threshold, blind zone, and sound-speed
   compensation against known distances before sending DBT.
6. Validate the compile-gated DBT transmitter against Boat's bounded parser,
   then enable it only after the coprocessor has a validated depth source. The
   Boat-side receiver has a compile-only UART0/USB-CDC profile on the GPIO21
   candidate; its physical pin audit is unsigned and the default firmware
   cannot start it. Keep both UART gates unsigned until pin and logic levels on
   both boards are checked.

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
it uses AVR Timer1 registers and AVR ADC registers. Its current `settings.h`
defaults both transducer drive and band-pass filter to 40 kHz; the 200 kHz
filter value is only a commented option. Do not reuse that default for the
candidate 200 kHz transducer or change only the filter while leaving the 40 kHz
drive clock. Select both for the exact transducer and shield revision. The NMEA
DBT sketch declares an ADC sample buffer but does not populate it, so its
reported depth index may be stale or zero. The raw-data example does capture
samples, but its timing and depth detection still need an ESP32 implementation
and bench validation.

The ADC adapter uses Espressif's `esp_adc/adc_continuous.h` interface available
with this repository's PlatformIO espressif32 8.0.0 framework package. It asks
for 75.8 ksample/s as a candidate rate; actual interval, sample loss and
voltage accuracy must be measured on the exact DevKitC and shield.

## Official API references

- [ESP-IDF 5.5.5 ADC continuous mode driver](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32/api-reference/peripherals/adc/adc_continuous.html)
- [PlatformIO Espressif32 8.0.0 release notes](https://github.com/platformio/platform-espressif32/releases/tag/v8.0.0) — Arduino-ESP32 3.3.12 based on ESP-IDF 5.5.5.
