# Boat Logical Pin Assignments R3

**R2 is superseded by this resource-conflict audit.** GPIO numbers are still intentionally not frozen.

See [Controller Pin Audit R1](BOAT_CONTROLLER_PIN_AUDIT_R1.md) for onboard resource reservations and official references.

Controller candidate: Heltec Wireless Tracker ESP32-S3+UC6580. The exact PCB revision, pin map and connector crosswalk must be confirmed before wiring. Reserve onboard GNSS, SX1262, USB, strapping and board-support GPIOs.

## Primary nRF24
- SPI MOSI/SCK/MISO from verified SPI bus; direct CE and CSN GPIO; IRQ input.
- Channel76,250kbps, fixed32-byte,PA_LOW initial.
- GPIO numbers remain unset in source macros until board pin audit.

## Backup E22
- Matching E22-900T22S UART module, 5V VCC from electronics buck.
- UART TX/RX at3.3V logic,9600 8N1; M0/M1 defined, AUX input, NRST accessible.
- Channel18=868.125MHz, air62.5kbps,subpacket32,10dBm,transparent.
- 50-ohm 868MHz antenna separate from nRF/GNSS.
- All UART and mode pins TBD until exact Tracker schematic.

## Motor and solenoids
- Motor driver PWM/DIR and high-current harness. Controller GPIO assignment TBD; default PWM low, DIR defined.
- Bait/hook MOSFET gates and output pins TBD; outputs must default off. Flyback/fuse/pulse limits required.
- Sensor I2C/UART/ADC pin assignments must not overlap radio, GNSS or strapping pins.

## Pin-freeze gate
Confirm exact SKU/revision, obtain schematic, trace board, continuity-check, reserve native SX1262/GNSS, verify boot strap levels and logic voltage, then update source macros and connector drawings.
