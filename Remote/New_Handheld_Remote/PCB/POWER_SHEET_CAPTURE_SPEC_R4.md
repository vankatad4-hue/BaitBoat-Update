# Remote Power + Peripheral Schematic Capture Specification R4

Capture guide only; not a native KiCad schematic/ERC/PCB.

## Sheet 1 — Pack/protection
Protected 1S4P PACK+→4A fast-acting fuse candidate→Q_REV FDS6575 Drain; Source→BAT_PROT; Gate→100k→GND. PACK−→GND. Verify pin/body diode. BAT_PROT feeds board battery input and radio buck-boost. Battery divider 220k/68k+100nF; NTC 10k B3950 candidate, 10k pull-up from controller 3V3, 100nF; fault detection.

## Sheet 2 — Waveshare controller power
BAT_PROT/GND to board PH2.0 single-cell battery input. Use onboard CS8501 boost to 5V. External TPS61023 not baseline; fallback only after testing. Confirm board revision, connector polarity, power-path/USB charging behavior, 5V output capability. No external boost outputs in parallel.

## Sheet 3 — Radio 3.3V
TPS63070, BAT_PROT→VIN; 1.5µH; divider 470k/150k; caps per review; VAUX 100nF. EN MCU controlled after boot, 1k series candidate and 100k pulldown; default OFF. Radio connector SPI/control, local decoupling.

## Sheet 4 — Joystick ADC and button expander
- ADS1115 3.3V I2C ADC, address 0x48, AIN0–3 four joystick axes, 100nF supply bypass; input RC candidate 1k series+1µF per axis.
- MCP23017 3.3V I2C expander, address 0x20, GPA0–7 bank A, GPB0–7 bank B; buttons active-low to GND with pullups. RESET pullup. Polling baseline; interrupts optional.
- I2C GPIO8/9 shared with onboard touch/CH422G/RTC. Check bus addresses/pullups.
- ADC and expander pins/footprints per datasheet; no native capture yet.

## Sheet 5 — Connectors and controller signals
J_JOY_L/R 3V3/GND/X/Y/NC; J_RADIO GND/3V3/SPI/CSN/CE/IRQ; I2C expansion 3V3/GND/SDA/SCL; button harness 16 contacts. Battery ADC GPIO6 candidate; radio CSN GPIO15 and CE GPIO43 candidates (verify header and attached circuits). Radio IRQ omitted/polled initially. SPI 11/12/13 shared with SD card, lock/arbitration and radio priority required.

## Capture/layout gates
Verify board pinout/revision, connector pin numbering, GPIO mux/attached circuits, I2C scan and pullups, all symbol pins, ERC, independent review, PCB DFM, RF/power layout and bench tests.
