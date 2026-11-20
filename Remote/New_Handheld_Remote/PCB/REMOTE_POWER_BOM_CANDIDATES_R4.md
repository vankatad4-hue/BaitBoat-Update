# Remote Power + Controls BOM Candidate R4

**Shortlist only. Verify MPN/package, footprints, datasheets, bus addresses, board pinout and supplier availability before purchase.**

| Ref | Part/value | Purpose / notes |
|---|---|---|
| J_BAT_BOARD | PH2.0 board battery connector | Protected pack to Waveshare battery input; verify polarity |
| F_MAIN | Littelfuse 0458004.DR 4A | 1206 fast-acting fuse candidate; interrupt/I²t/derating |
| Q_REV | onsemi FDS6575 P-MOS | SO8; Drain=input, Source=BAT_PROT; verify pins/body diode |
| R_GATE | 100k | Gate pull-down |
| Battery sense | 220k/68k 1% +100nF | GPIO6 ADC candidate |
| Pack NTC | 10k B3950 +10k pullup +100nF | NTC monitor |
| U_RADIO | TPS63070 | Radio 3.3V buck-boost |
| L_RADIO | XFL4020-152MEC 1.5µH | Candidate |
| Radio capacitors | TI baseline values | Effective capacitance/bias verify |
| U_AXIS_ADC | ADS1115 | 16-bit, 4-channel, I2C, address 0x48; four joystick axes |
| C_ADC | 100nF | ADS1115 supply bypass |
| R_AXIS | 1k per channel | ADC input series candidate |
| C_AXIS | 1µF per channel | Input filter candidate |
| U_BUTTONS | MCP23017 | 16 I2C GPIO, address 0x20; buttons |
| R_RESET | 10k | MCP23017 RESET pull-up candidate |
| J_JOY_L/R | 5-pin keyed connectors | 3V3/GND/X/Y/NC |
| J_RADIO | 8-pin keyed connector | GND/3V3/SPI/CSN/CE/IRQ |
| J_I2C | 4-pin | 3V3/GND/SDA/SCL |
| J_BUTTONS | TBD | 16 normally-open button harness contacts |
| Testpoints | BAT_PROT, RADIO_3V3, GND, ADC_BAT, NTC | Accessible |

Controller 5V boost is on the Waveshare board (CS8501); external TPS61023 is a fallback, not baseline. Check onboard 450mA load capability and thermal behavior.
