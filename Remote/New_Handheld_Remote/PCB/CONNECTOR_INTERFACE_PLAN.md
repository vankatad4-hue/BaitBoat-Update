# Connector Interface Plan — Preliminary / Pinouts TBD

Connector pin numbers and part numbers are not assigned. Use keyed/locking connectors where practical; finalize after exact board/module review.

## Power-board interfaces
- BAT+ / GND: protected pack input.
- SYS_5V: TPS61088 boost output for display/controller.
- SYS_3V3_RF: TPS63070 output for nRF PA/LNA radio.
- PWR_EN: control, architecture TBD.
- BAT_SENSE: battery measurement.
- PG_RADIO: optional radio regulator power-good.
- PACK_NTC: temperature input if pack provides it.

## Main ↔ inputs
GND, logic 3V3, joystick ADC X/Y pairs, button matrix rows/columns, optional board identification.

## Main ↔ nRF24L01+ PA/LNA
SYS_3V3_RF, GND, SPI_SCK, MOSI, MISO, NRF_CE, NRF_CSN, NRF_IRQ. Connector order TBD. Local radio decoupling: 100 nF plus 10–47 µF near module.

## Display/controller
Connector/interface depends on Waveshare board revision. Verify pinout, cable, mechanical support and connector clearance before schematic capture.

## USB-C
CC/ESD/charger/power-path and data routing require design review. Charging circuit is currently excluded.

## RF
- ANT_NRF: selected SMA/u.FL or module-specific external antenna connection, nRF radio only.
- ESP32 RF: Waveshare onboard antenna. No external ESP32 RF path.
- RF paths are independent; never connect together. 2.4 GHz coexistence testing remains necessary.

## Status
Physical connector part numbers, pin order, wire colors, current ratings and finalized cable definitions remain pending. Do not build cable assemblies or fabricate PCB until pinouts are reviewed.
