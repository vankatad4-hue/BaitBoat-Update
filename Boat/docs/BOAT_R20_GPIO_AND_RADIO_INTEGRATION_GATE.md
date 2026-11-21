# R20 — GPIO resource and LoRa interoperability integration gate
Date: 2026-11-21. Design review only. Applies R15-R19 canonical policy. NO physical output enabled.

## GPIO resource allocation by function, not invented pin numbers
| Signal | Quantity | Proposed path | Gate |
|---|---:|---|---|
| MDD20A PWM1/PWM2 | 2 | Direct Heltec ESP32-S3 output with hardware-safe enable downstream | Must verify exposed safe PWM-capable GPIO |
| MDD20A DIR1/DIR2 | 2 | Direct GPIO or vetted COTS expander if safe latency/reset behavior proven | Verify output state at reset |
| nRF24 SPI SCK/MOSI/MISO | 3 | Shared integrated SX1262 SPI only if schematic shows accessible bus and arbitration tested; otherwise separate SPI pins | Verify board wiring and CS exclusivity |
| nRF24 CE/CSN | 2 | Direct safe GPIO | CE default low; CSN default high, no boot glitch |
| nRF24 IRQ | 1 optional | Poll if unavailable, with latency tests | No claim of IRQ pin availability |
| I2C SDA/SCL | 2 | V1.2 verified pins only | Onboard conflict and voltage audit |
| EKM014 requests 1..4 | 4 | Off-the-shelf MCP23017/TCA9555 I2C expander to *independent COTS safety gating*, never directly as safety guarantee | Reset default off, safety module unresolved |
| Safety feedback/status | TBD | Vetted slow expander inputs | No dependency on MCU for physical STOP |
| Onboard SX1262/GNSS/TFT/USB | reserved | Keep all manufacturer-designated resources | Must use exact V1.2 board schematic |

No numeric GPIO freeze possible from the currently reviewed revision-ambiguous audit. The R7 best-case resource study reported only 11 tentative pins for at least 12 native functions with two external radios; removing boat E22 helps but does NOT by itself prove a conflict-free v1.2 pinout. GPIO macro defaults stay disabled.

## Remote E22 ↔ Boat integrated SX1262
E22-900T22S is an EBYTE UART modem, not a raw UART-to-SX1262 transparent cable. Even when its RF silicon is compatible, its firmware's packet header, fixed/transparent addressing, modulation and framing must be matched on the boat.
Qualification matrix:
1. Exact E22 hardware variant and EU868 legal channels, TX power and duty-cycle restrictions.
2. Frequency, bandwidth, spreading factor, coding rate, preamble, sync word, explicit/implicit header, payload length, CRC, IQ polarity, packet whitening and FEC (if present).
3. E22 UART transparent vs fixed mode, address/channel and auxiliary pin timing; manufacturer configuration readback.
4. Boat SX1262 radio HAL mode, RF switch, DIO1/BUSY/reset and coexistence with nRF SPI.
5. Two-way bench packet exchange with exact bytes, RSSI/SNR and controlled errors; then authenticated session/ACK/STOP, timeout and cross-radio deduplication.
Until all gates pass, backup LoRa stays disabled. Never treat RF chip similarity as proof of E22 protocol compatibility.

## Next implementation artifacts
- V1.2 manufacturer schematic pin crosswalk CSV with source page/board revision and tested headers.
- Radio bench configuration table with exact E22 readback and SX1262 register settings.
- Firmware changes only after these are verified; preserve disabled output macros.
