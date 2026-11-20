**SUPERSEDED by R3:** The initial prototype uses matching EBYTE E22-900T22S UART LoRa modules at remote and boat. Tracker integrated SX1262 is unused.

# Boat LoRa hardware decision R2 — use the Tracker's integrated SX1262

## Correction to R1
The previously proposed additional EBYTE LoRa module on the boat is **not required if the selected boat controller is confirmed as Heltec Wireless Tracker V1.1/V1.2 with onboard SX1262 and an EU863–870 RF variant**. The Heltec board already integrates SX1262 + GNSS + ESP32-S3, and its own IPEX antenna port can be used for the backup link. This reduces the boat carrier PCB and power load.

- Remote backup: EBYTE E22-900T22S UART SX1262 module candidate.
- Boat backup: integrated SX1262 on Heltec Wireless Tracker candidate.
- Both use LoRa PHY; compatibility is conditional on identical frequency, bandwidth, spreading factor, coding rate, sync word, preamble, packet format, CRC and IQ settings. E22's transparent UART framing and Heltec raw SX1262 packet mode are not inherently protocol-compatible; an explicit bridge/packet format must be implemented and verified.
- The Heltec Tracker module's RF antenna connector and band must be confirmed for the exact purchased revision. Do not assume a 915MHz SKU is EU868-ready or that its configured power is legal.
- If using an exact Heltec Tracker V1.2 board, its native SX1262 driver and pin mapping will be used; don't add another UART LoRa module unless the integrated radio is unsuitable or inaccessible.

## Boat radio electrical interface
- nRF24L01+ PA/LNA module: external 2.4GHz primary radio on carrier; SPI from controller, dedicated CSN/CE/IRQ GPIOs.
- LoRa: Tracker integrated SX1262; dedicated 868MHz antenna via its LoRa RF connector. No external E22 module/5V supply needed in this variant.
- Separate nRF antenna and LoRa antenna. Do not share one antenna or coax.
- Confirm power supply/current capacity for both radio TX peaks, GNSS and MCU/display loads. Native SX1262 TX peaks and carrier supply must be measured.
