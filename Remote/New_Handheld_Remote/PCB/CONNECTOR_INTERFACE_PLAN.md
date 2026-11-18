# Connector Interface Plan — Preliminary / Pinouts TBD

Use keyed, locking connectors wherever practical. Pin numbers below are logical names, not assigned physical pin numbers. Pin assignment will be frozen only after selected MCU/module, display, connector family and board routing are reviewed.

## Main board ↔ Power board
| Signal | Direction | Notes |
|---|---|---|
| BAT+ | Power | Protected pack positive |
| GND | Return | Common ground |
| SYS_5V | Power | Boost rail, if needed by display |
| SYS_3V3 | Power | Regulated logic/radio rail |
| PWR_EN | Logic | Power control / latch signal, architecture TBD |
| CHG_STAT | Logic | Charger status output |
| BAT_SENSE | Analog | Divider/fuel gauge signal |

## Main board ↔ Input board(s)
| Signal | Direction | Notes |
|---|---|---|
| GND | Return | Common |
| 3V3 | Power | Logic only; do not power high-current loads |
| JOY_L_X / JOY_L_Y | Analog | Filtered joystick signals |
| JOY_R_X / JOY_R_Y | Analog | Filtered joystick signals |
| BTN matrix rows/columns or GPIO | Logic | Scan/interrupt design TBD |
| INPUT_ID | Analog/logic | Optional board identification |

## Main board ↔ nRF24L01+ PA/LNA
| Signal | Direction | Notes |
|---|---|---|
| 3V3_RF | Power | Dedicated filtered supply sized for PA burst current |
| GND | Return | RF return |
| SPI_SCK / MOSI / MISO | Logic | SPI bus |
| NRF_CE | Logic | Chip enable |
| NRF_CSN | Logic | SPI chip select |
| NRF_IRQ | Logic | Optional interrupt |

## Main board ↔ Display
Exact connector and signals depend on display module. For RGB displays, interface may be high-pin-count and cannot be represented by a generic low-pin connector. Select the screen first, then create a dedicated display interconnect/adapter and mechanical support.

## USB-C
- VBUS: charging/input power (5V)
- GND: return
- CC1/CC2: USB-C sink configuration/charger control
- D+/D−: ESP programming/data if supported
- Shield: chassis/ESD strategy TBD

## Antenna connections
- ANT_NRF: SMA/u.FL or selected radio connector to nRF24 module only.
- ANT_ESP: supported external antenna path for selected ESP32-S3 module only.
- Never connect the two antenna paths together.

## Status
No connector pin numbers, wire colors, connector part numbers, current ratings or pinout are approved yet. These will be documented before PCB fabrication.

# Connector Interface Plan — Preliminary / Pinouts TBD

Use keyed locking connectors where practical. Logical signal names below are not physical pin numbers; freeze pinout only after connector, module and PCB review.

## Power board interfaces
- BAT+ / GND: protected pack input.
- SYS_5V: TPS61088 boost output for display/controller.
- SYS_3V3_RF: TPS63070 radio output; dedicated nRF PA/LNA rail.
- PWR_EN: board-level power control, architecture TBD.
- BAT_SENSE: battery measurement.
- PG_RADIO: optional radio regulator power-good.
- NTC: battery temperature input if pack provides it.

## Main ↔ input controls
GND, 3V3 logic, four joystick ADC signals, button matrix rows/columns, optional input identification.

## Main ↔ nRF24L01+ PA/LNA
3V3_RF, GND, SPI_SCK, MOSI, MISO, NRF_CE, NRF_CSN, NRF_IRQ. Exact connector pin order TBD. Place 100 nF and 10–47 µF local decoupling near radio/module.

## Display/controller
Exact connector/interface depends on the Waveshare board revision. Verify connector, pinout and mechanical details before capture.

## USB-C
Charging and USB data/programming require CC, ESD, power-path and data routing design. Charging schematic is currently excluded from this work.

## RF/antenna
- ANT_NRF: SMA/u.FL or module's actual antenna connector, nRF24 PA/LNA only.
- ESP32: Waveshare board onboard antenna; no external antenna connector/path planned.
- Keep nRF and ESP32 RF systems electrically independent. Both use 2.4 GHz, so coexistence testing remains required.

## Status
Connector part numbers, physical pin numbers, wire colors, current ratings and finalized pinouts remain unapproved. Do not fabricate cable assemblies until these are documented.
