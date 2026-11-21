# R21 — Heltec v1.2 evidence and backup-radio test specification
2026-11-21. Active module-only R15–R20 policy. DESIGN / NO ACTUATOR ENABLE.

## Source confidence
- Heltec official Wireless Tracker page https://heltec.org/project/wireless-tracker/ advertises hardware revisions and links datasheets/schematics.
- Official Heltec docs repository https://github.com/HelTecAutomation/HeltecDocs/blob/master/doc/node/esp32/source/wireless_tracker/index.rst currently points to a **1.1** datasheet and HTIT-Tracker_V0.5 schematic; these are **NOT proof of v1.2 header assignments**.
- https://resource.heltec.cn/download/Wireless_Tracker/Wireless%20tracke.pdf explicitly labels its pin map **Rev 1.1**, May 2023.
- Zephyr board pin list https://github.com/zephyrproject-rtos/zephyr/blob/main/boards/heltec/heltec_wireless_tracker/doc/index.rst also describes **V1.1**.
- No authenticated V1.2-specific header schematic was established during this review. **DO NOT freeze numeric pins**.

## Candidate GPIO crosswalk — v1.1 evidence only, not v1.2 wiring
| GPIO | V1.1 indicated use | R21 handling |
|---|---|---|
| 8–14 | SX1262 CS/SCK/MOSI/MISO/RST/BUSY/DIO1 | RESERVED; cannot reassign |
| 33–34 | GNSS UART | RESERVED |
| 35–37 | GNSS control/PPS on V1.1 family | RESERVED pending v1.2 schematic |
| 38–42 | TFT | RESERVED |
| 21 | TFT backlight | RESERVED |
| 0,45,46 | boot/strapping or board functions | RESERVED |
| 1–3 | battery ADC/control, VEXT | RESERVED |
| 43–44 | UART0 debug | RESERVED |
| 19–20 | USB or flow-control depending revision | RESERVED |
| 4–7,15–18,26,47,48 | candidate header functions | NOT VERIFIED; 15/16 oscillator, 18 LED, 47/48 board functions and 26 SPICS cautions |
No source supports claiming all candidate GPIOs are free. Verify silkscreen revision, physical header-to-chip, peripheral power rails, strapping, and boot-time states.

## EBYTE E22-900T22S manufacturer facts and test gate
EBYTE user manual https://robu-prod-media.s3.ap-south-1.amazonaws.com/uploads/2024/10/E22-xxxT22S_UserManual_EN_V1.4.pdf lists 2.7–5.5V VCC, 3.3V UART logic, ~150mA TX at 22dBm, UART modem and 240-byte maximum packet size. Exact module variant and configured output power must be read back. The E22-900T22S uses SX1262 silicon, but has additional modem packetization and potential FEC/encryption. **DO NOT assume raw Heltec SX1262 packets decode.**

### Bench interoperability steps
1. Record E22 full label, hardware/firmware revision, readback registers and transparent/fixed mode.
2. Set lawful EU 868MHz channel and power/duty cycle after local regulatory review.
3. Capture E22-to-E22 reference packets with known payload and a suitable RF test setup.
4. Attempt Heltec raw SX1262 receive with documented frequency/BW/SF/CR/preamble/sync/CRC/header/IQ; compare packet payload and receive statistics.
5. If E22 encapsulation is opaque, seek manufacturer protocol documentation or select a compatible premade raw SX1262 remote module with owner approval; do not invent an RF protocol.
6. Validate bidirectional ACK, deduplication across nRF/LoRa, authenticated STOP and radio-loss failsafe with motors disconnected.
No live link, packet capture or physical radio test has been performed.

## Pin-freeze and safety exit conditions
Require official v1.2 schematic or manufacturer confirmation, physical board marking, board-support package mapping, and unpowered continuity evidence before assigning numeric GPIO. The nRF24 CE/CSN reset levels and shared SPI bus contention must be checked. Actuator branches remain isolated by independent hardware STOP and unselected onehot/pulse/POR/current-zero stack; no bench test may energize motors/coils before safety approval.
