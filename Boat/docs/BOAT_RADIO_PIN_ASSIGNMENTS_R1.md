# Boat Radio Pin Assignment R2 — logical only

Exact MCU pin numbers are intentionally TBD until the purchased Heltec Wireless Tracker board revision/schematic is checked. The board's integrated SX1262, GNSS, USB, strapping, and other resources are reserved; initial backup link uses a separate E22 module pair.

## nRF24 primary
- SPI MOSI/SCK/MISO from verified controller SPI bus.
- Direct CSN and CE MCU GPIOs; direct IRQ input preferred.
- Pin values must not overlap Tracker's integrated SX1262 SPI, GNSS UART, USB or boot strapping.
- External radio module on carrier with dedicated 3.3V rail and local decoupling.
- Initial RF profile: channel 76 (2476MHz), 250kbps, PA_LOW, fixed 32-byte frame, CRC16, auto-ack/retry. Validate coexistence and local EU 2.4GHz power limits.

## E22 backup UART
- UART TX/RX direct MCU pins, 9600 8N1 initial. GPIO numbers TBD.
- M0/M1 default low (normal transparent mode), AUX input, NRST accessible.
- 5V supply from electronics buck; local bypass.
- Dedicated 868MHz antenna port; separate from nRF and GNSS.
- Identical remote/boat config: channel18=868.125MHz, air data62.5kbps, 32-byte subpacket, output10dBm, transparent mode, address/network zero.

## Physical pin freeze
1. Exact Heltec Tracker version/PCB revision confirmed.
2. Schematic pin map crosschecked with board and continuity.
3. SPI/UART choices avoid onboard SX1262 and GNSS resources.
4. Voltage levels and boot straps verified.
5. Test harness connector crosswalk recorded and reviewed before carrier PCB/harness manufacture.
