# ESP32 sonar coprocessor recommendation R2

**Date:** 2026-11-28
**Status:** use the owner's existing **ESP32-DevKitC V4** as the sonar-only controller candidate. Not yet wired or commissioned. Boat now has an unsigned UART0 RX candidate on GPIO21 in an isolated compile profile; the default firmware still cannot start the receiver.

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

The selected software candidate is compact depth-only NMEA DBT over a one-way UART from coprocessor GPIO22 TX to Boat GPIO21 RX, with a shared logic ground. Boat keeps UART2 for GNSS and UART1 for E22; the isolated `boat_open_echo_uart_compile` profile routes `Serial` to native USB CDC and uses UART0 for sonar receive. The default firmware leaves the sonar adapter disabled, and the adapter refuses to start while its separate GPIO audit flag is zero.

Boat GPIO21/J3-18 is now the unsigned UART0 RX candidate and the optional nRF IRQ is left unconnected. `NRF24Link` polls the radio and does not consume IRQ. The candidate is reflected in the canonical Boat pin map and GPIO crosswalk; inspect the exact board header and measure signal levels before signing it or connecting the wire.

Boat now has a bounded DBT decoder connected to a compile-gated UART0 receiver through `BoatOpenEchoUartPort`. Its isolated compile profile routes diagnostics to native USB CDC and reserves Boat GPIO21/J3-18 as UART RX; the default firmware remains disabled because the physical pin audit flag is unsigned. The coprocessor-side GPIO22 TX, Boat-side header mapping, TX voltage and shared-ground arrangement remain unverified, so this is a software interface candidate rather than a connection instruction. The Remote receives compact depth telemetry only; full 1,800-sample echo profiles and Lucky D+T water temperature need separate protocol work and are outside this depth-only first connection.

**D+T temperature is not yet electrically specified.** The [manufacturer's wired 718LIC D+T product page](https://www.luckylaker.com/products/luckylaker-sensor-718lic) lists a 200 kHz, 45-degree transducer and says it requires a compatible fish-finder monitor; it does not publish temperature-element pins, resistance curve, excitation limits, or data protocol. Do not connect the temperature element to an ESP32 ADC by assumption. Boat and Remote already carry a bounded water-temperature telemetry field, but no source driver publishes actual Lucky D+T measurements.

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
candidate and contains a disarmed USB diagnostic scaffold, fixed-size
5,224-sample capture buffer, optional ADC1 continuous/DMA adapter, and a pure
echo-profile threshold classifier. It also contains a side-effect-free
TUSS4470 SPI frame codec with separate read/write encoders, a fixed zero byte for
reads, whole-frame odd-parity validation, and named response status fields. It
also includes a compile-gated ESP32 transport at a candidate 400 kHz / SPI mode
1 with one 16-bit transfer per NCS pulse and an explicit NCS-high guard. The
transport is not instantiated; a compile-time hardware lock and unsigned SPI
pin audit prevent it from starting. Verify mode, timing and voltage levels on
the actual board/shield before signing the gate. The target deliberately does not
instantiate the capture or detector, configure shield pins, start the
transducer, read an ADC, or emit DBT until the electrical and acquisition gates
above are cleared. The classifier requires explicit calibrated ADC threshold,
blanking, measured acoustic burst-origin sample, sample timing, and sound-speed
inputs; it does not treat the TUSS OUT4 threshold register as an ADC code or
claim a calibrated depth. Any future
SPI use must verify the current mode 1, 400 kHz transport and NCS-high guard on
the exact shield; the TUSS4470 SPI clock must remain at or below 500 kHz.

The upstream Open Echo UNO code is not a drop-in ESP32 implementation: it uses
AVR Timer1 and ADC registers. Its current `settings.h` defaults both drive and
band-pass frequencies to 40 kHz even though the candidate Lucky D+T transducer
is 200 kHz; the 200 kHz filter value is commented out. Do not copy that
configuration or change only the filter. Also, `NMEA_DBT_OUT.ino` declares an
analog sample buffer but does not store ADC values into it; the depth output can
therefore be stale or zero. The raw-data sketch captures samples but still
needs a timed ESP32 ADC/DMA port and validation before it can provide Boat
telemetry.
