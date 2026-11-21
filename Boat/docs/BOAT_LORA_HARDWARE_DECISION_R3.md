> **SUPERSEDED for Boat LoRa by R8 (2026-11-21).** The original E22, I2C-UART, and 12-pin count below are historical. Active Boat uses integrated SX1262 with 10 external native signals. See Boat/docs/BOAT_INTEGRATED_SX1262_R8.md; Remote E22 remains incompatible until tested.

# Boat LoRa architecture R3 — paired E22 modules

Supersedes the R2 suggestion to use the Tracker's integrated SX1262 as the default boat backup. The integrated SX1262 on the Heltec Wireless Tracker remains available but is not used in the initial test pair.

## Baseline pairing
- Remote: EBYTE E22-900T22S UART LoRa backup.
- Boat: identical EBYTE E22-900T22S UART LoRa backup.
- Matching module firmware/configuration, channel/frequency, air data rate, address, UART settings, and packet mode required.
- This avoids relying on cross-compatibility between E22 transparent packet protocol and Heltec native raw SX1262 protocol.

## Hardware
- Boat E22 module supply from protected electronics rail; confirm voltage/current and TX transients for exact revision. Local bulk+ceramic bypass.
- Direct MCU UART (3.3V logic) on verified unused UART pair. GNSS, native SX1262, USB, strapping pins must remain reserved.
- M0/M1 defined and held in normal transparent mode; AUX status monitored; reset pin accessible.
- Dedicated EU868 antenna connector and antenna, physically separated from nRF24 and GNSS.
- The Tracker's onboard SX1262 may remain unused in this prototype; do not connect two RF transmitters to one antenna.

## Firmware
The boat LoRa driver is the same E22 transparent UART driver as remote, with identical application frame protocol. The controller UART assignment and transport driver are still not implemented.
