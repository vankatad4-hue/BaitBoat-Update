# Remote Power BOM Candidates R6 — consolidated

## Pack/protection
| Ref | Qty | Candidate | Notes |
|---|---:|---|---|
| J_PACK |1|Keyed 2-pin pack connector|1S4P pack; verify current rating/polarity |
| F_MAIN |1|4A fast fuse candidate|Coordinate BMS, cell, wire, connector, interrupt/I²t |
| Q_REV |1|FDS6575 P-channel candidate|Reverse protection; verify pin/body diode |
| R_GATE |1|100k|Gate pulldown candidate |
| C_BAT |set|100µF+10µF+100nF|BAT_PROT bypass candidate |

## Rails
| Ref | Qty | Candidate | Purpose / gates |
|---|---:|---|---|
| U_CTRL_BOOST |1|Waveshare CS8501 onboard boost|Controller/display only. Verify load, USB charge/backfeed and thermal |
| U_NRF_BOOST |1|TPS63070|BAT_PROT→RADIO_3V3 nRF rail; component design/reference review |
| L_NRF |1|1.5µH candidate|TPS63070 inductor; saturation/thermal/stability check |
| R_FB1/R_FB2 |2|470k/150k candidate|TPS63070 feedback; verify actual datasheet pin/target |
| U_LORA_BOOST |1|TPS61023 dedicated boost|BAT_PROT→5V LORA_5V; not parallel with CS8501 |
| L_LORA |1|TPS61023 reference inductor|Value/current/saturation from chosen datasheet design |
| C_LORA_IN |set|10µF×2+100nF|Effective capacitance and layout |
| C_LORA_OUT |set|22µF×2+100µF+100nF|Output transient/ESR/stability review |
| F_LORA |1|Branch fuse/PTC candidate|Select after current and wiring validation |

## Radio and UART
| Ref | Qty | Candidate | Notes |
|---|---:|---|---|
| U_NRF |1|nRF24L01+ PA/LNA module|Primary; 3.3V; local100nF+10–47µF |
| U_E22 |1|EBYTE E22-900T22S-V2 UART LoRa|Backup; 5V supply, 3.3V UART, 10dBm initial config |
| ANT_NRF |1|2.4GHz 50Ω antenna|LEFT port |
| ANT_E22 |1|868MHz 50Ω antenna|RIGHT port; gain/cable loss legal limit |
| U_SC16 |1|SC16IS750 I2C UART bridge|I2C@0x48; crystal1.8432MHz candidate; GPIO M0/M1/AUX/reset |
| X_SC16 |1|1.8432MHz crystal|Load caps calculate from exact crystal |
| C_E22 |set|100µF+10µF+100nF|Module local bypass |

## Control/sensing
| Ref | Qty | Candidate | Notes |
|---|---:|---|---|
| U_ADC_AX |1|ADS1115|I2C0x4A, joystick axes |
| U_ADC_MON |1|ADS1115|I2C0x4B, battery/NTC/rail |
| U_BUTTON |1|MCP23017|I2C0x20,16 physical buttons |
| U_RADIO_IO |1|MCP23017|I2C0x21, IRQ/enable/spares |
| R_BAT_TOP/BOT |2|220k/68k|Battery sense divider candidate |
| R_NTC |2|10k/10k|NTC divider candidates |
| C_ADC |set|100nF filters|Analog filters candidate |
| J_TEST |set|Test pads|BAT_PROT,5V,3V3,LORA_5V,I2C,SPI,UART,ADC |

## Power budget estimate (not measured)
Assume controller/display5V450mA, E22 5V150mA peak, nRF3.3V300mA burst, converter efficiencies90%, pack3V: about1.48A pack. Measure peak and average; cell/BMS/fuse/connector thermal must be validated.

## Design gates
No converter values/layout, footprint, thermal, ripple, load or protection tests are final. CS8501 output powers only controller/display; TPS61023 powers E22 separately; outputs are never paralleled.
