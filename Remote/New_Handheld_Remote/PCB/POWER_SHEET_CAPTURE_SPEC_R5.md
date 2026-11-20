# SUPERSEDED by POWER_SHEET_CAPTURE_SPEC_R6.md

# Remote Power + Peripheral Schematic Capture Specification R5

Capture specification only; no native KiCad/ERC/PCB or hardware validation.

## Sheet 1 — Battery input/protection
J_PACK keyed 2-pin; PACK+ → F_MAIN 4A fast-fuse candidate → PACK_FUSED → Q_REV FDS6575 P-channel candidate Drain; Source → BAT_PROT; Gate → 100k to GND. Verify datasheet package pins/body diode and topology. PACK−→GND. BAT_PROT local 100µF/10µF/100nF candidates to GND. BAT_PROT feeds Waveshare battery input and TPS63070 radio converter. Fuse near pack connector. Confirm pack BMS, connector, wire, fuse I²t/interrupt and thermal ratings.

## Sheet 2 — Waveshare controller power
BAT_PROT/GND to board PH2.0 single-cell battery input; polarity must be physically verified on exact revision. Use onboard CS8501 boost. No TPS61023 baseline, no paralleling boost. Bench test display/backlight load, startup, 5V rail droop/ripple/thermal, radio concurrent load, USB power/charging interactions.

## Sheet 3 — RADIO_3V3
TPS63070 from BAT_PROT, L=1.5µH candidate, feedback 470k/150k candidate, VSEL/FB2/PS-SYNC pin config verified against datasheet, EN reset-safe default OFF. Effective input/output capacitance under DC bias must be calculated. Dedicated rail only to nRF module. Add 100nF+10–47µF near radio connector.

## Sheet 4 — ADS1115 joystick ADC @0x48
AIN0 L-X; AIN1 L-Y; AIN2 R-X; AIN3 R-Y. Address ADDR→GND. 3V3/GND, 100nF bypass. 1k series +1µF shunt per axis candidate. Inputs 0–3.3V; ensure no overrange. I2C GPIO8/9 shared; no extra pullups until measured.

## Sheet 5 — MCP23017 button expander @0x20
A0/A1/A2→GND. GPA0–7 left button bank, GPB0–7 right. Normally open button to GND, active-low. Internal pullups, reset pullup 10k, local 100nF. Poll/debounce firmware; interrupts optional.

## Sheet 6 — Monitoring ADS1115 @0x49
Second ADS1115 added to avoid using GPIO6/Sensor AD (board circuit unresolved) and avoid overloading the joystick ADC channels. Address ADDR→3V3. AIN0 battery divider (220k BAT_PROT top, 68k bottom, 100nF filter); AIN1 pack NTC divider (10k pullup from controller 3V3, 10k B3950 NTC to GND, 100nF); AIN2 optional RADIO_3V3 monitor divider (100k/100k, 100nF); AIN3 spare. All channels must remain within ADS1115 input limits and ADC supply. Board 3V3 available from verified PORT1 pin 2. ADC only operates while controller is powered.

## Sheet 7 — Radio control expander @0x21 (experimental)
MCP23017: A0=3V3, A1/A2=GND (address 0x21). GPA0 CE, GPA1 CSN, GPA2 IRQ_N. I2C shared; local 100nF; RESET pulled up 10k. Mark schematic note **RADIO CONTROL TIMING MUST BE BENCH VALIDATED**. Replace control strategy if I2C expander latency harms RF handling.

## Sheet 8 — Shared SPI and radio connector
ESP GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO. Shared with TF card; separate CS (radio CSN through expander, SD_CS via onboard CH422G EXIO4). Software mutex, radio priority, chunk SD operations. J_RADIO: 1 GND, 2 RADIO_3V3, 3 SCK, 4 MOSI, 5 MISO, 6 CSN, 7 CE, 8 IRQ_N. Connector/module pinout must be checked against exact radio breakout. Radio 1–4MHz initial SPI clock; characterize.

## Validation gates
Board revision/header polarity; BMS/connector/fuse/MOS sizing; converter configuration/capacitance; I2C scan/pullups/rise time; ADC/joystick latency; expander radio-control timing; shared SPI/TF; PA/LNA current bursts/thermal; USB behavior; KiCad ERC, footprint and independent review.
