# Remote Power + Controls BOM Candidate R5

**Candidate list only. Validate MPN/package/footprint/datasheet and ratings before procurement.**

| Ref | Candidate | Function / notes |
|---|---|---|
| J_PACK | keyed 2-pin ≥4A connector, TBD | 1S4P pack interface; polarity keyed |
| F_MAIN | Littelfuse 0458004.DR, 4A fast fuse candidate | Validate I²t, interrupt rating, derating, pack coordination |
| Q_REV | onsemi FDS6575 P-MOS SO-8 | Reverse-polarity protection candidate; D=PACK_FUSED, S=BAT_PROT; verify package/body diode |
| R_GATE | 100k | Gate-to-GND default bias |
| C_BAT | 100µF + 10µF + 100nF | BAT_PROT local decoupling candidates; review |
| J_WAVE_BAT | PH2.0 2P | Waveshare battery input; verify polarity/connector current |
| U_RADIO | TPS63070 | Dedicated RADIO_3V3 buck-boost |
| L_RADIO | Coilcraft XFL4020-152MEC 1.5µH | Candidate; saturation/current/footprint check |
| Radio capacitors | existing R4 values | Effective capacitance under bias, stability and ripple review |
| U_AXIS_ADC | ADS1115 @0x48 | Four joystick channels |
| U_MON_ADC | ADS1115 @0x49 | Battery, NTC and optional radio rail monitoring; address ADDR→3V3 |
| C_ADCx | 100nF each | Local supply bypass |
| R_AXIS | 1k each | Analog input series candidate |
| C_AXIS | 1µF each | Analog filter candidate; ~159Hz corner with 1k |
| U_BUTTONS | MCP23017 @0x20 | 16 button inputs; A0/A1/A2 low |
| U_RADIO_IO | MCP23017 @0x21 | CE/CSN/IRQ control candidate; A0 high, A1/A2 low |
| R_RESET | 10k per expander | RESET pull-up candidates |
| R_BAT_TOP/BOTTOM | 220k/68k, 1% | Pack sense divider to ADS1115@0x49 AIN0 |
| R_NTC_PULLUP | 10k, 1% | NTC divider from 3V3 |
| NTC_PACK | 10k B3950 candidate | Pack temperature sensor; exact curve/placement confirm |
| R_RADIO_DIV | 100k/100k 1% candidate | Optional RADIO_3V3 monitor to ADC2 AIN2; scales ~3.3V to ~1.65V |
| J_JOY_L/R | keyed 5P | 3V3/GND/X/Y/NC |
| J_RADIO | keyed 8P | GND/3V3/SCK/MOSI/MISO/CSN/CE/IRQ |
| J_NTC | 2P | NTC sense/GND |
| J_BUTTONS | TBD | 16 direct contacts; pinout and harness TBD |
| J_I2C | 4P | 3V3/GND/SDA/SCL, board-side access TBD |
| Testpoints | battery/rails/bus/analog | Access for bring-up |

Controller/display 5V boost is on the Waveshare board (CS8501); external TPS61023 is fallback only. Onboard board load spec 5V/450mA and battery boost/charge current documentation must be bench-confirmed for this use.
