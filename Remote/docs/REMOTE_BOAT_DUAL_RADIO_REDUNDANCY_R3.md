# Dual-radio pairing decision R3 — identical E22 UART modules

## Decision
For the first test prototype, install the **same EBYTE E22-900T22S UART LoRa module at both remote and boat ends**. The Heltec Wireless Tracker candidate's integrated SX1262 remains unused for this backup link unless later firmware interop testing proves a viable alternative.

Reason: identical E22 modules use the same transparent UART/air protocol/configuration, avoiding the uncertain compatibility between E22's internal packet format and a native SX1262 raw packet implementation. The boat still has integrated LoRa hardware available, but it is not the default link in this prototype.

## Radio set
- Remote primary nRF24L01+ PA/LNA 2.4GHz.
- Boat primary nRF24L01+ PA/LNA 2.4GHz.
- Remote backup E22-900T22S UART/SX1262.
- Boat backup matching E22-900T22S UART/SX1262.
- Remote LEFT antenna = 2.4GHz; RIGHT = EU868.
- Boat antennas are separate matched 2.4GHz and EU868, mounted apart from GNSS and motor wiring.

## Pairing/configuration
Configure both E22 modules identically:
- Regional frequency/channel selected within EU863–870 after Bulgaria national rules check.
- Air data rate and packet settings matched.
- Network address/channel, transparent/fixed mode and UART baud/parity matched.
- Initial UART 9600 8N1 and short application packets.
- Output power configured legally; module maximum 22dBm is not the default.
- Use application frame with IDs, sequence, timestamp, CRC and authentication tag; E22 UART transparent data is not an ACK by itself.
- Test packets both directions, ACK timing, packet loss and duty-cycle enforcement.

## Boat controller interface
The Heltec Tracker has multiple UART resources, but exact pin assignment must avoid native GNSS, SX1262, USB, strapping and other board functions. Use direct hardware UART if a verified spare pair exists. If no spare UART pins, add SC16IS750 bridge or select an alternate controller carrier. Don't select pins from generic ESP32-S3 pin lists without the exact board schematic.

## Integrated SX1262 alternative
If later using the Tracker's onboard SX1262, the remote E22 and native SX1262 must be shown interoperable through measured/verified PHY and packet framing. It is an alternative—not an assumed drop-in.
