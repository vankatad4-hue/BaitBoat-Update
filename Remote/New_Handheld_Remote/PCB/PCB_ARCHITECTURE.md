# Modular PCB Architecture — Preliminary

Design intent: modular, serviceable boards with keyed connectors where useful. This document is not a finished schematic or PCB.

## Modules
1. Main/display controller interface for Waveshare ESP32-S3-Touch-LCD-4.3 Standard.
2. Power board: protected battery input, fuse/reverse-polarity, regulated rails. Charging circuitry is out of current scope.
3. Input boards: joystick ADC and button scanning.
4. Radio: nRF24L01+ PA/LNA with dedicated TPS63070 3.3 V buck-boost rail, SPI and control.
5. ESP32 RF: onboard antenna on Waveshare board; no external antenna path.
6. Display and connector interface: exact revision/pinout to be verified.
7. Optional accessories: camera/light/vibration only if boat hardware supports them.

## Power-flow baseline
Protected 1S4P pack → fused/protected BAT_PROT:
- TPS61088 boost candidate → 5 V display/controller rail.
- TPS63070 buck-boost → 3.3 V radio rail.

TPS63070 calculation baseline: R1=470 kΩ, R2=150 kΩ (3.307 V calculated), 1.5 µH inductor candidate, TI reference input/output capacitors (76 µF nominal output bank), VAUX 100 nF. Detailed component review is in PCB calculation document.

## Signals
Main MCU/display ↔ joystick/buttons; MCU ↔ nRF SPI/CE/CSN/IRQ; MCU ↔ battery sensing, enable and optional power-good. Power board connects display/controller and radio rails.

## RF and thermal
nRF24 and ESP32 Wi-Fi/Bluetooth share 2.4 GHz band; coexistence must be tested. Keep converter SW loops away from antenna feeds. Maintain continuous ground reference and RF keepouts; do not split ground plane under RF.

## Outstanding
Native schematics, verified pin numbers/footprints, connector pinouts, PCB layout, BOM, assembly drawings, ERC and bench tests are not completed.
