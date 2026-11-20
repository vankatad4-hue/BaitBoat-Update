# Remote Hardware Test Build R2

R4 adds matched E22 backup modules, direct nRF pin proposals and radio bench drivers. This is a test-build spec, not fabrication release. This is a bench-testable architecture document, **not a fabrication release**.

## Build hardware blocks
1. Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948; verify board revision.
2. Protected 1S4P 18650 pack; pack/BMS/fuse/connectors must be reviewed and bench tested before pack use.
3. nRF24L01+ PA/LNA primary with dedicated 3.3V TPS63070 supply.
4. E22-900T22S LoRa backup module on remote; dedicated BAT_PROT→TPS61023→LORA_5V converter; TX/RX bridged by SC16IS750 I2C-UART.
5. Boat side uses matching E22-900T22S backup module; integrated Tracker SX1262 unused in first prototype.
6. ADS1115 x2 and MCP23017 x2 plus SC16IS750 I2C bridge.

## Controller connections
- I2C SDA GPIO8, SCL GPIO9. Start at 100kHz. Scan bus; then 400kHz only after rise-time/pull-up verification.
- SPI MOSI GPIO11, SCK GPIO12, MISO GPIO13 shared with TF card; lock and prioritize radio. Start 1MHz.
- nRF CE GPIO6 (Sensor AD repurposed) and CSN GPIO15 (RS485 TX path; external bus disconnected/transceiver inactive) direct. SPI GPIO11/12/13. IRQ via GPA1.
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
- Test matching E22 radio pair over shielded/controlled RF; config/readback, packet CRC, authenticated protocol and duty-cycle tests are separate gates.
- Simultaneous display/backlight/Wi-Fi/nRF/LoRa activity current, voltage droop, thermal and reset test.
- Verify antenna feed continuity, connector mapping, band match and enclosure detuning before outdoor operation.

## GPIO/PCB gate
The pin functions on Waveshare documentation are confirmed, but physical PORT1 pin-number crosswalk and R2 repurposing must be confirmed against exact board revision and continuity. GPIO6 CSN may be loaded by sensor AD circuitry. MCP CE timing may be too slow. If either fails, choose a direct GPIO using verified header crosswalk; don't fabricate until resolved.

## Firmware gates
- Source includes nRF/E22 transports, SC16 bridge, CRC/HMAC codec, command codec, link manager, boat receive/dedupe/failsafe scaffold. Key provisioning, session negotiation, app ACK, actuator integration and hardware validation remain incomplete.
- Remote and boat use matching E22 modules and the same transparent UART settings; module config readback and pair RF tests are required.
- LoRa airtime/duty-cycle must be enforced for the selected EU subband. Do not assume a 2–5Hz backup control rate is compliant.
- No actuator connection until boat watchdog and release command idempotency are complete and tested.


## LoRa profile R3 (supersedes prior channel assumptions)
- Remote and boat use matching E22-900T22S modules.
- Initial config: channel 0x12 = 868.125MHz, UART 9600 8N1, air rate 62.5kbps, subpacket 32 bytes, output 10dBm, transparent mode, network/address 0x0000, LBT initially off.
- E22 config write: `C0 00 09 00 00 00 67 C3 12 00 00 00`; readback must match. M1=1/M0=0 for config, M1=M0=0 for normal; M1=M0=1 is sleep, not configuration.
- 868.125MHz is within the 868.0–868.6MHz 1% duty sub-band. E22's 1MHz channel step cannot reach the 869.525MHz center of the 10% sub-band. Strict airtime control is required.
- Driver estimate is 25ms/frame until measured. A 1% budget allows 36s/h; one 25ms TX per 3s is 0.83%. Remote and boat airtime are tracked independently. Default backup cadence ≤0.33Hz, no sonar. Actual airtime and persistent duty counter are mandatory before field operation.
- Source configures remote SC16IS750/E22 at boot and includes nRF24 driver; nRF pins are provisional. No command TX or authenticated connection occurs because key provisioning is absent.


## nRF direct GPIO R3 test connection
- CE=GPIO6 Sensor AD terminal, repurposed and no sensor attached.
- CSN=GPIO15 RS485 TXD; isolate/disable the onboard RS485 driver and verify it cannot drive the external bus.
- SPI GPIO11/12/13. TF card and nRF share SPI; test SD access arbitration.
- IRQ_N to MCP23017 0x21 GPA1 (poll initially).
- Source pin defaults are set to CE6/CSN15/channel76, but physical board revision/crosswalk and electrical isolation remain a hard wiring gate.


## Pin assignment correction
GPIO6 is CE, GPIO15 is CSN; earlier R2 text suggesting GPIO6 CSN and expander CE is superseded. Both direct GPIO pins remain provisional until board revision/continuity is checked. RS485 must be disconnected/disabled and Sensor AD left unused.


## E22 power source gate
E22 VCC uses a dedicated TPS61023 BAT_PROT→5V boost; CS8501 is reserved for controller/display. UART/M0/M1/AUX remain3.3V logic. Reserve150mA peak; scope the dedicated rail at10dBm TX and test converter thermal.


## E22 dedicated power branch R5
Controller/display uses CS8501 5V output only. E22 VCC uses a separate BAT_PROT→TPS61023→LORA_5V boost branch; UART/M0/M1/AUX logic remains3.3V. Never parallel the two converter outputs. Scope rail and measure input/output currents with 10dBm E22 TX and nRF bursts.
