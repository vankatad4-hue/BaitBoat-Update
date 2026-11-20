# Remote Power Net Draft R5

**Candidate net architecture; not a native schematic/ERC/PCB or validated design.**

Battery: BMS protected PACK+→F_MAIN Littelfuse 0458004.DR 4A fast-acting candidate→Q_REV FDS6575 Drain. Q_REV Source→BAT_PROT; Gate→100kΩ→GND. This corrected orientation is intended to pass normal polarity and block reverse; verify package pin/body diode. PACK−→GND. ADC divider BAT_PROT→220k→ADC_BAT; ADC→68k→GND; 100nF filter. NTC 10k B3950 candidate from NTC_SENSE to GND, 10k pull-up from controller 3V3, 100nF; fault detection required.

5V boost: TPS61023DRLR; BAT_PROT→VIN; L=1µH BAT_PROT→SW; CIN 10µF +100nF local; VOUT→+5V_MAIN; COUT 2×22µF; 732k +5V→FB, 100k FB→GND. EN always enabled from BAT_PROT via 100k candidate. 450mA continuous/1A transient target; thermal pending.

Radio 3V3: TPS63070; BAT_PROT→VIN; L=1.5µH between L1/L2; 470k VOUT→FB, 150k FB→GND; caps per component draft; VAUX 100nF; VSEL low, FB2 GND, PS/SYNC per datasheet. EN MCU-controlled after boot, 1k series candidate, 100k pulldown; OFF at reset. PG optional. Radio connector local 100nF+10–47µF.

Estimated 3V battery input ≈1.20A continuous and ≈2.33A simultaneous transient (assumed efficiencies). Protection coordination not validated.
