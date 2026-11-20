# Remote Controller — R6.4 Engineering Status

## Work completed in this continuation
- Reviewed Waveshare official documentation and schematic for SKU 25948 touch version.
- Updated GPIO audit: most GPIOs are occupied by RGB display, touch, USB, TF-card, UART and RS485/CAN circuitry.
- Added provisional MCP23017 button/radio I/O allocation.
- Added ADS1015 four-axis joystick ADC logical mapping.
- Added provisional wiring plan for matrix, joystick ADC, nRF and E22.
- Identified potential conflicts on TF SPI pins and USB-UART pins.
- No component footprints, GPIOs, connector pin numbers, or electrical ratings are frozen.

## Status matrix

| Area | Status | Blocking issue |
|---|---|---|
| Front mechanical concept | In progress | Joystick/radio antenna collision; exact joystick selection |
| Waveshare board | Candidate chosen, not purchased | Need confirm exact SKU/revision on purchase |
| GPIO audit | Reviewed | Need physical header/access and pin conflict verification |
| I2C architecture | Proposed | Bench bus scan; address compatibility |
| Button matrix | Proposed | Exact switches/diodes and harness |
| Joystick ADC | Proposed | ADC noise, sample rate, calibration |
| nRF24 radio | Selected family | Exact PA/LNA module and clean SPI/GPIO allocation |
| E22 backup | Candidate | UART isolation/mux plan, variant and antenna |
| Battery pack | Concept 1S4P | Cells, protection, fuse, dimensions, restraint |
| Charging | Not selected | Verify onboard CS8501 circuit/power-path and charging behavior |
| DC/DC | Not selected | Current, efficiency, ripple and thermal tests |
| Schematic | Not released | Pin and component decisions still open |
| PCB | Not started | Wait for schematic and footprints |
| Firmware | UI concept underway | Hardware pin map and control input interfaces needed |

## Board revision warning

Pin tables and schematic reviewed are for Waveshare ESP32-S3-Touch-LCD-4.3 touch model SKU 25948. The 4.3B and 4.3C versions differ in mapping. When the board is purchased, verify SKU and revision against its markings and repeat the audit if different.

## Recommended next decision

Keep the Waveshare board as the display/UI processor. Implement buttons and joystick analog inputs using external I2C expander and ADC. For radios, reuse the TF SPI pins only if TF storage is dropped and verified; use a UART isolation/mux for E22 to preserve USB debug. If no safe accessible GPIO allocation can be proven, add a small dedicated control/radio MCU instead of forcing an unsafe pin map.

## Links
- [Waveshare 4.3 board documentation](https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3)
- [Official schematic and resources](https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3/Resources-And-Documents)
- [Pin audit R6.4](REMOTE_PIN_AUDIT_R6_4.md)
- [I/O architecture R6.4](REMOTE_IO_ARCHITECTURE_PRELIM_R6_4.md)
- [Wiring plan R6.4](REMOTE_WIRING_PLAN_R6_4.md)
- [Component shortlist R6.4](COMPONENT_SELECTION_SHORTLIST_R6_4.md)
- [Power architecture R6.3](REMOTE_POWER_ARCHITECTURE_PRELIM_R6_3.md)
