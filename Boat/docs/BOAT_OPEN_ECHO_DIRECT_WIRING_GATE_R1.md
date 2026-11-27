# Open Echo TUSS4470 direct-wire feasibility and gate R1

**Branch:** `platformio-unified-r30h20`  
**Scope:** software and wiring preparation only. No physical pin assignment, power-up, actuator access, or hardware acceptance is authorized by this document.

## Decision

The Open Echo TUSS4470 shield can be considered for a direct-wire Boat prototype. It does not need to be physically stacked on an ESP32 board: route its documented signal nets through a keyed adapter harness to the Boat controller after the exact shield and controller revisions are inspected. The shield was designed for Arduino UNO, and its current repository guide does not claim pin/software compatibility with other boards. Treat every Arduino pin number below as a shield-sketch reference only; it is not a Boat GPIO assignment.

The easiest depth-only bring-up remains an Arduino UNO running Open Echo's NMEA DBT sketch and a separate serial link into Boat. Direct wiring avoids that intermediate MCU but needs the acquisition firmware ported to the Boat target. The existing `OpenEchoDbtDecoder` only parses serial DBT; it does not configure or read TUSS4470 registers or sample `VOUT`.

## Signal list from the published UNO sketch

The NMEA DBT sketch declares CS on UNO D10, IO1 on D8, IO2 on D9, OUT3 on D3, OUT4 on D2, and analog input on A0. Standard UNO SPI also uses D11/MOSI, D12/MISO, and D13/SCK. These labels identify the shield's existing sketch mapping only.

| Shield function | UNO sketch reference | Boat-side interface to design | Gate |
|---|---:|---|---|
| SPI chip select | D10 | Dedicated output GPIO | Check conflicts and idle state on exact Boat board |
| SPI clock/data | D13/D11/D12 | SPI SCK/MOSI/MISO | Confirm SPI mode and run at or below the TI limit |
| TUSS4470 IO1 / IO2 | D8/D9 | Two controlled digital GPIOs | Verify direction, timing, and reset-safe levels |
| OUT3 / OUT4 | D3/D2 | Two digital inputs | Verify shield output voltage and any pull-up path |
| Echo envelope VOUT | A0 | ADC input, with attenuation only if required by measured range | Verify exact shield VDD/VOUT swing and ESP32 ADC limits |
| Signal reference | GND | Common signal ground to Boat controller | Define return path away from motor/transducer drive current |

No ESP32 GPIO numbers are assigned here. The current Boat connector and controller revision are not physically confirmed, and candidate board pin maps have changed across earlier design records. The selected pin map must come from inspection of the actual Boat carrier and the exact shield schematic, not from UNO labels or a generic ESP32-S3 example.

## Electrical and transducer boundaries

- The shield has an ultrasonic transducer drive supply path. Power it only through the supply input specified for the inspected shield revision; never connect that drive rail to an ESP32 3.3 V, ADC, or GPIO pin. Open Echo's current guide discusses external driver supply and warns against exceeding its stated board limit.
- Measure the shield's digital logic-high and analog VOUT range before connecting ESP32 inputs. The TUSS4470 datasheet bounds digital output and VOUT by its VDD rail. If the shield runs those signals above the Boat controller's safe input range, add a correctly designed level/attenuation stage before any powered test.
- Match the filter-capacitor configuration and firmware drive frequency to the exact transducer. Confirm polarity at the transducer connector and use the recommended shielded/coaxial routing for a longer lead.
- Keep transducer drive current and its return separate from motor PWM/current paths. Start with actuator power disconnected and a current-limited bench supply.
- The Lucky D+T water-temperature output is not documented as part of the shield's DBT serial sentence. Do not infer that the TUSS4470 VOUT waveform or DBT depth provides water temperature.

## Firmware and timing design

1. Keep the current DBT input path as a useful baseline: `pollOpenEchoDbt(Stream&, now)` reads a bounded number of bytes and the decoder publishes depth-only health.
2. For direct shield wiring, add a separate TUSS4470 acquisition adapter with injected pin/bus configuration. Do not add pin numbers to `TrackerSensorPort` or enable the driver by default.
3. Port only after reconciling register operations with the selected TUSS4470 datasheet and shield firmware revision. The current UNO sketch selects SPI mode 1 and uses `SPI_CLOCK_DIV16`; at a 16 MHz UNO clock that divider requests 1 MHz, while the TI datasheet specifies a 500 kHz maximum SPI rate. The direct ESP32 implementation must observe the datasheet limit.
4. The UNO sketch captures 5,224 ADC samples with a stated ~13.2 μs sample interval, about 69 ms of capture time before processing. Do not transplant this blocking loop into Boat's main application loop. Choose a bounded, hardware-timed capture approach and measure its effect on radio, watchdog, GNSS, and application deadlines before integration.
5. Keep sensor acquisition under the existing Boat hardware/application owner during initial development. Do not move SPI, ADC, I2C, or raw profile buffers into the dormant radio worker. Publish only a validated compact depth summary through `TrackerSensorPort::publishSonarSummary`; raw echo profiles are not sent over the current compact control telemetry.
6. Verify real depth against known distances at the intended 0.5–10 m range. Reject DBT's fixed sound-speed assumption for calibrated accuracy unless it is measured and corrected for water conditions.

## Required evidence before pin assignment or live integration

- Exact shield revision, populated component values, schematic and terminal orientation.
- Exact Boat controller/carrier revision, available SPI and ADC resources, board-level GPIO ownership, and connector mapping.
- Measured VDD, logic-high, OUT3/OUT4, and VOUT range under idle and echo operation.
- Confirmed transducer SKU, nominal frequency, drive-voltage/current requirements, cable, and waterproof mounting.
- Captured test waveforms/sample records and measured acquisition timing on the actual Boat target.
- Independent review that the adapter cannot affect motor, coil, STOP, watchdog, battery, or RF safety paths.

Until these gates are satisfied, the driver stays absent/inactive, Boat GPIO and actuator gates remain unchanged, and the sonar display represents only fresh compact summary samples received over telemetry.

## References

- [Open Echo TUSS4470 shield guide](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/README.md)
- [Open Echo NMEA DBT output sketch](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/arduino/NMEA_DBT_OUT/NMEA_DBT_OUT.ino)
- [TI TUSS4470 datasheet](https://www.ti.com/lit/ds/symlink/tuss4470.pdf)
- [Espressif ESP32-S3 ADC guidance](https://docs.espressif.com/projects/esp-faq/en/latest/hardware-related/development-board.html)
