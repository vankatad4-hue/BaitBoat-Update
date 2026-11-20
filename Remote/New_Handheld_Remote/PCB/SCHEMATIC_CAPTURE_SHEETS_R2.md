# Remote Electrical Schematic Capture R2 — prototype interface design

**Revision intent:** convert R1 net-level concepts into explicit connector, component, address and test requirements. This remains a schematic specification; no native KiCad symbols, ERC or PCB DRC has been run.

## Sheet 00 — Architecture
Battery 1S4P → pack protection → BAT_PROT.
BAT_PROT branches:
- Waveshare single-cell PH2.0 battery input and its onboard CS8501 power/charge circuitry.
- TPS63070 converter → dedicated 3.3V primary nRF rail.
- LoRa E22 module supply (BAT_PROT direct, valid 2.7–5.5V) with power set conservatively; do not assume 22dBm output at pack voltage. Use separate switched rail only after current/voltage tests.
Controller I2C GPIO8/9:
- MCP23017 buttons 0x20
- MCP23017 radio I/O 0x21
- ADS1115 axes 0x4A
- ADS1115 monitor 0x4B
- SC16IS750 I2C-UART bridge 0x48 for E22 serial
Controller SPI GPIO11/12/13:
- TF card and nRF24L01+ PA/LNA shared bus; one SPI lock; TF chip select via onboard CH422G EXIO4, nRF CSN direct GPIO.
All ground returns common; separate high-current battery/radio return layout and quiet analog ADC ground region.

## Sheet 01 — Pack input/protection
J_PACK keyed 2-pin, rated at least the measured peak pack current.
1 PACK+
2 PACK− → GND
PACK+ → F_MAIN → PACK_FUSED
PACK_FUSED → Q_REV reverse-protection stage → BAT_PROT
R_GATE 100k from gate to GND; confirm MOSFET pin numbering, orientation and gate-source limits.
C_BAT: 100µF + 10µF + 100nF BAT_PROT-to-GND, candidate only.
F_MAIN 4A candidate must be coordinated with cell, BMS, wire, connector and load; fuse close to pack.
Test polarity, fuse coordination, MOSFET thermal drop, startup and short protection with current-limited supply. Do not use pack until protection is verified.

## Sheet 02 — Waveshare controller battery
J_WAVE_BAT PH2.0: BAT_PROT and GND, polarity per board schematic/silkscreen and continuity measurement. Onboard CS8501 battery charge/boost remains unmodified. USB-C charging/backfeed with battery attached must be checked. External 5V boost is not paralleled.
Board 5V output/load capacity is not assumed adequate for radios until simultaneous load/thermal test.

## Sheet 03 — Primary radio supply
U_RADIO TPS63070, 3.3V:
- VIN → BAT_PROT
- L1/L2 switching pins through 1.5µH inductor candidate
- VOUT → RADIO_3V3
- Feedback divider: 470k VOUT→FB; 150k FB→GND (calculate against selected variant/reference and tolerances)
- CIN and COUT per TI reference topology with effective capacitance at bias/temperature considered.
- VAUX 100nF to GND.
- VSEL, FB2, PS/SYNC and EN must be connected per selected datasheet/reference design; R1 draft assumptions are not accepted until a pin-level review.
- EN default disabled during reset via pull-down; controlled by verified GPIO/expander only after logic levels checked.
- PG open-drain pullup and optional monitor.
Layout: switch loop/inductor/caps tight; RF output supply branch separate; avoid routing antenna feed over converter.

## Sheet 04 — nRF24 PA/LNA radio
J_NRF module keyed connector:
1 GND
2 RADIO_3V3
3 MOSI
4 SCK
5 MISO
6 CSN (direct MCU GPIO)
7 CE (direct MCU GPIO preferred; expander test only)
8 IRQ_N (input)
9 optional VCC monitor/test
Local bypass: 100nF + 10µF + optional 22–47µF low-ESR at module supply pins.
RF: LEFT 2.4GHz 50Ω antenna connector/coax.
SPI shared with TF card; start 1MHz, radio access priority, mutex, no SD transactions during TX/ACK window. Keep RF module supply/ground short and avoid long flying wires after prototype.

## Sheet 05 — LoRa backup radio + UART bridge
U_LORA E22-900T22S candidate:
- VCC → BAT_PROT (2.7–5.5V allowed; 5V required to guarantee full max output according to manual; use low power setting and test at pack voltage)
- GND → GND
- TXD → U_BRIDGE UART RX
- RXD ← U_BRIDGE UART TX (3.3V logic; no 5V TTL)
- M0/M1 controlled via U_BRIDGE GPIO or straps; for initial transparent mode both LOW.
- AUX → U_BRIDGE GPIO input or bridge IRQ status; never block while waiting.
- NRST → U_BRIDGE GPIO output or RC reset with pull-up; reset polarity from datasheet.
- ANT → dedicated 50Ω coax/pigtail → RIGHT 868MHz antenna.
Decoupling at VCC: 100µF + 10µF + 100nF candidates, voltage rating > pack maximum; scope module rail while TX.

U_BRIDGE NXP SC16IS750 (I2C/UART, 64-byte FIFO):
- VDD 3V3_CONTROLLER, GND common; local 100nF and 1µF.
- SDA/SCL shared I2C GPIO8/9; address strap selected for 0x48; verify address pins.
- TX/RX to E22 UART as above.
- GPIO0=M0, GPIO1=M1, GPIO2=AUX input, GPIO3=NRST; spare GPIOs reserved.
- IRQ to U_RADIO_IO GPA2 (or direct MCU GPIO if pin allocation audited).
- Crystal/clock circuit per selected SC16IS750 variant datasheet; use specified crystal/oscillator and load capacitors, or exact documented clock source. Do not omit clock network.
- Bridge driver polls RX FIFO non-blocking; read bursts bounded and use I2C bus mutex.

## Sheet 06 — I2C bus and joystick ADC
I2C_SDA GPIO8, I2C_SCL GPIO9.
Measure onboard pullups and bus capacitance; do not add pullups blindly. Target 100kHz for initial scan, then 400kHz if rise time meets spec.
U_AXIS ADS1115 @0x4A, ADDR→SDA (address strap; confirm mapping), VDD=3V3, GND, SDA/SCL.
AIN0 LX, AIN1 LY, AIN2 RX, AIN3 RY.
Each joystick potentiometer 10k nominal from 3V3 to GND, wiper→ADC input via 1k; 100nF to GND candidate RC; verify ADC input/common-mode and source impedance.
Sample four axes at 50Hz minimum; ADS1115 max 860SPS means ~215SPS/channel with sequential scans; no command before calibration.

## Sheet 07 — Monitor ADC
U_MON ADS1115 @0x4B, ADDR→SCL; VDD=3V3; common I2C.
AIN0 BAT_SENSE: R_TOP 220k BAT_PROT→AIN0; R_BOTTOM 68k AIN0→GND; C 100nF; at 4.2V≈0.977V. ADC config gain/input and tolerances verify.
AIN1 NTC_SENSE: 10k pullup to 3V3, 10k B3950 NTC to GND, 100nF. Detect open/short; sensor thermally couple to pack safely.
AIN2 optional RADIO_3V3 monitor: 100k/100k divider, 100nF.
AIN3 spare.

## Sheet 08 — Buttons
U_BTN MCP23017 @0x20 A0/A1/A2=GND; VDD=3V3; GND; SDA/SCL; RESET pulled up 10k; 100nF local.
GPA0–7 left button bank L0–L7; GPB0–7 right bank R0–R7.
Each switch normally-open to GND. Internal pull-ups enabled. Debounce 10–20ms. Button actions are debounced and safety-gated; release commands require one-shot action ID/boat ACK.

## Sheet 09 — Radio control expander
U_RADIO_IO MCP23017 @0x21: A0=3V3, A1/A2=GND; reset 10k pull-up; local 100nF.
GPA0 CE candidate, GPA1 nRF IRQ_N, GPA2 SC16IS750 IRQ, GPA3 LoRa power enable candidate. GPB reserved/test.
nRF CE via I2C expander is prototype-only and timing validation required. Production should route CE to direct GPIO if verified spare is available. CSN must be direct GPIO.

## Sheet 10 — External ports / test points
J_I2C 4-pin: 3V3, GND, SDA, SCL (keyed).
J_SPI 6-pin: GND, 3V3, MOSI, SCK, MISO, CSN; separate CE/IRQ test pads.
J_UART_DEBUG: 3V3, GND, bridge TX, bridge RX, RESET/AUX.
Test pads: BAT_PROT, 5V_WAVE, RADIO_3V3, LORA_VCC, GND, I2C SDA/SCL, SPI, CE, CSN, IRQ, ADC channels.
All connectors keyed, locking, labelled, pin-1 marked.

## Sheet 11 — RF and EMC
LEFT bulkhead nRF24 2.4GHz, RIGHT LoRa 868MHz. Separate 50Ω coax; never swap antennas.
Grounded RF keepout around connector/feed. Do not run high-speed display/SD or switch-node traces beneath RF feed/module antenna region. Antennas separated; test enclosure and hand detuning. Battery and converter layout near antennas may require revision.
