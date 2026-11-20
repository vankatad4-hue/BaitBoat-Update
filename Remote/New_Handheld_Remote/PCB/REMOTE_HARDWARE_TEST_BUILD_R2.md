# Remote Hardware Test Build R2

R2 improves pin allocation using an I2C UART bridge for LoRa, moves ADC addresses, and identifies the integrated board pin constraints. This is a bench-testable architecture document, **not a fabrication release**.

## Build hardware blocks
1. Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948; verify board revision.
2. Protected 1S4P 18650 pack; pack/BMS/fuse/connectors must be reviewed and bench tested before pack use.
3. nRF24L01+ PA/LNA primary with dedicated 3.3V TPS63070 supply.
4. E22-900T22S LoRa backup module on remote; supply BAT_PROT (2.7–5.5V) and legal low-power setting; module TX/RX bridged by SC16IS750 I2C-UART.
5. Boat side uses Heltec Wireless Tracker candidate integrated SX1262 for backup if exact version/band confirmed—no second E22 module required by default.
6. ADS1115 x2 and MCP23017 x2 plus SC16IS750 I2C bridge.

## Controller connections
- I2C SDA GPIO8, SCL GPIO9. Start at 100kHz. Scan bus; then 400kHz only after rise-time/pull-up verification.
- SPI MOSI GPIO11, SCK GPIO12, MISO GPIO13 shared with TF card; lock and prioritize radio. Start 1MHz.
- nRF CSN GPIO6 candidate direct output (sensor AD unused). nRF CE via MCP23017 0x21 GPA0 is provisional and must be latency-tested; preferred final CE direct GPIO. IRQ via GPA1, active-low.
- SC16IS750 I2C UART @0x48 (A0/A1 straps to VDD); 1.8432MHz crystal candidate, load capacitors calculated from crystal CL. TX/RX to E22, GPIO0/1 M0/M1 low, GPIO2 AUX input, GPIO3 reset. Bridge IRQ on MCP23017 0x21 GPA2. UART 9600 8N1 for initial configuration; increase after module setup and test.
- ADS1115 axes @0x4A ADDR=SDA, AIN0 LX/AIN1 LY/AIN2 RX/AIN3 RY.
- ADS1115 monitor @0x4B ADDR=SCL, AIN0 battery divider, AIN1 NTC, AIN2 optional RADIO_3V3 monitor, AIN3 spare.
- MCP23017 buttons @0x20: GPA left 8, GPB right 8; switches active-low to GND, pullups.
- MCP23017 radio I/O @0x21: GPA0 CE, GPA1 nRF IRQ, GPA2 UART bridge IRQ, GPA3 RADIO_EN; reset pullup and bypass.

## Electrical tests
- Current-limited 3.7V supply; verify protected pack rail, polarity, board input and controller boost.
- Verify pack input current and battery charging path/backfeed over USB; do not install cells until safety review.
- I2C scan: record all device addresses. Verify no collision: planned addresses 0x20,0x21,0x48,0x4A,0x4B plus onboard touch/RTC/CH422G.
- Measure I2C pullups and rise times; ensure all bus devices operate at 3.3V.
- ADS1115 check with known voltages; joystick calibration; test divider/NTC and open/short fault handling.
- MCP button mapping and debounce.
- SC16IS750 clock, reset, I2C address and UART loopback; then E22 module configuration/readback; verify 3.3V UART.
- nRF SPI register test with radio output disabled; check CE/CSN/IRQ timing and module rail droop. Shared SD SPI arbitration test.
- Test LoRa radio pair over shielded/controlled RF; match PHY settings with Heltec SX1262 on boat, ACK, packet CRC/authentication, sequence freshness, and failover.
- Simultaneous display/backlight/Wi-Fi/nRF/LoRa activity current, voltage droop, thermal and reset test.
- Verify antenna feed continuity, connector mapping, band match and enclosure detuning before outdoor operation.

## GPIO/PCB gate
The pin functions on Waveshare documentation are confirmed, but physical PORT1 pin-number crosswalk and R2 repurposing must be confirmed against exact board revision and continuity. GPIO6 CSN may be loaded by sensor AD circuitry. MCP CE timing may be too slow. If either fails, choose a direct GPIO using verified header crosswalk; don't fabricate until resolved.

## Firmware gates
- Current source includes protocol codec (CRC and HMAC) and dual-radio state-machine scaffold, but hardware drivers, key provisioning, packet encode/decode integration, boat-side duplicate action cache, transport ACK, and actual nRF/LoRa modules are incomplete.
- E22 transparent UART framing and native Heltec SX1262 raw packet protocol require PHY settings to be matched and interop bench-tested.
- LoRa airtime/duty-cycle must be enforced for the selected EU subband. Do not assume a 2–5Hz backup control rate is compliant.
- No actuator connection until boat watchdog and release command idempotency are complete and tested.
