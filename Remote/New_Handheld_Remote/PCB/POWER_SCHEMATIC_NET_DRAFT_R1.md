# Power PCB — Electrical Schematic Net Draft R1

**Status:** Net-level electrical draft; not a native KiCad schematic.

## Design
- Pack: protected 1S4P 18650; charging schematic excluded.
- Display/controller: Waveshare ESP32-S3-Touch-LCD-4.3 powered from 5 V.
- Radio: nRF24L01+ PA/LNA on dedicated 3.3 V rail.
- Two converters from fused/protected battery: 5 V boost and 3.3 V buck-boost.
- Common ground plane; high-current return loops controlled by layout.

## Sheet A — BATTERY_INPUT
J1 battery input; F1 3 A time-delay provisional; reverse-polarity MOSFET ideal-diode circuit Q1 not yet specified; BAT_PROT downstream. Input bulk/ceramics and test points. Pack protection is not a replacement for wiring fuse.

## Sheet B — BOOST_5V (TPS61088)
BAT_PROT to TPS61088 boost topology following TI datasheet. Output +5V_MAIN, target 5.0 V for display/controller. EN has defined default state. FB divider, inductor, FSW, ILIM, compensation, SS, MODE, input/output capacitors and layout still require calculation. NC/thermal connections follow exact datasheet.

## Sheet C — RADIO_3V3 (TPS63070)
- VIN=BAT_PROT.
- L1/L2 through L_RADIO 1.5 µH.
- VOUT=RADIO_3V3; R1=470 kΩ VOUT→FB, R2=150 kΩ FB→GND (3.307 V calculated nominal).
- FB2=GND single-output; VSEL static LOW; PS/SYNC HIGH power-save (verify recommended pin connection/bias); EN active via proposed 10 kΩ BAT_PROT pull-up; PG optional 100 kΩ pull-up to RADIO_3V3.
- Input: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output: 3×22 µF 16 V X6S 0805 + 10 µF 0603 (76 µF nominal).
- VAUX: 100 nF X7R.
- Radio connector power/decoupling and SPI/control pins per connector drawing.
- 
## Sheet D — SENSE_CONTROL
Battery ADC divider/filter TBD; optional switched divider to reduce idle drain. Pack NTC input (open/short detection required). BOOST_EN/RADIO_EN defined defaults. Optional RADIO_PGOOD input. Test points BAT_PROT, +5V_MAIN, RADIO_3V3, GND, enable and ADC.

## Sheet E — CONNECTORS
J1 pack; J2 display 5 V/GND; J3 radio 3.3 V/GND/SPI/control; J4 NTC; J5 MCU control/status. Connector part numbers, pin order, wire gauges and keyed mating parts TBD.

## Budget
At 3.0 V battery, assumptions:
- Display 5 V × 450 mA, 85% boost efficiency → 882 mA input.
- Radio 3.307 V × 200 mA, 90% buck-boost efficiency → 245 mA input.
- Total ≈1.13 A.
This is not measured and excludes other loads/inrush. Fuse and pack wiring must be validated.

## Validation gates
Exact pin numbers, footprints, converter compensation/current limit, capacitor bias, fuse/connector/wire ratings, layout, ERC and bench validation remain outstanding.
