# Remote Power Capture Specification R3

Guide for native schematic capture, not an actual KiCad file or validation report.

## Sheet 1 — Battery/protection
BMS-protected PACK+→F_MAIN 4A fast fuse candidate→Q_REV FDS6575 Drain; Source→BAT_PROT; Gate→100k→GND. Verify package pin/body diode. PACK−→GND. BAT_PROT branches to Waveshare battery header and TPS63070 VIN. ADC divider 220k/68k +100nF; NTC 10k B3950 candidate with 10k pull-up from controller 3V3 and 100nF; open/short detection. Testpoints.

## Sheet 2 — Controller battery interface
J_WAVESHARE_BAT: BAT_PROT and GND to the board's PH2.0 single-cell battery input. Verify connector orientation, pin polarity, board revision and onboard CS8501 power path. Baseline uses onboard battery boost to 5V; external TPS61023 omitted. Bench-check 5V rail/current and transient behavior; do not add an external boost in parallel. Charging is not part of our design; USB/charge-source interaction must be understood before charging.

## Sheet 3 — Radio 3.3V
TPS63070 buck-boost, BAT_PROT input; 1.5µH L between L1/L2; divider 470k/150k; capacitor banks per calculations; VAUX 100nF. EN MCU-controlled after boot, 1k series candidate/100k pulldown, default OFF; pin map R3. Radio connector 3V3/GND/SPI/CSN/CE/IRQ and local 100nF+10–47µF.

## Layout/review
Fuse/MOS path rated, compact converter loops, quiet FB/ADC, RF keepout, common ground and testpoints. Verify board battery pins, all symbols/footprints, ERC, independent review and bench validation.
