# Remote Power Net Draft R3
**Status:** TPS61023 candidate selected; net-level draft only, not KiCad/ERC/PCB/validated.
## Battery
J_BAT PACK+/PACK−/NTC. PACK+→protected pack→4A time-delay fuse candidate→Q_REV P-MOS TBD→BAT_PROT; PACK−→GND. BAT_ADC and NTC sensing TBD. Charger excluded.
## 5V
TPS61023DRLR BAT_PROT→+5V_MAIN. L=1µH BAT_PROT→SW. CIN=10µF X7R/X7S 10V+; supplemental 100nF. COUT=2×22µF X7R/X7S 10V+. Rtop=732kΩ +5V→FB; Rbottom=100kΩ FB→GND. EN enabled from BAT_PROT (100k pull-up candidate, datasheet verification); MCU cannot be sole enable. Target 450mA continuous/1A transient, thermal validation pending. Internal compensation.
## Radio 3.3V
TPS63070 BAT_PROT→RADIO_3V3; 1.5µH. Divider 470k/150k, nominal 3.3067V. CIN 2×10µF 25V X7S 0805 +10µF 25V X5R 0603; COUT 3×22µF 16V X6S 0805 +10µF 0603 (76µF nominal, effective bias TBD); VAUX 100nF. VSEL low, FB2 GND, PS/SYNC power-save network, EN defined, PG optional. Radio connector 100nF+10–47µF. 200mA assumption, 300mA test budget.
## Control/test
BOOST_5V_EN hardware-default ON; RADIO_EN reset state defined; RADIO_PGOOD optional; ADC_BAT, NTC_BAT; testpoints BAT_PROT, +5V, RADIO_3V3, enables, ADC, NTC, GND.
## Budget @3.0V
5V/450mA at 90%≈0.83A; radio 3.3V/300mA at 90%≈0.37A; continuous≈1.20A. Display 1A transient at 85% plus radio≈2.33A. Estimated only; excludes other loads/startup. Fuse/BMS candidates not frozen.
Capture gates: package pins, feedback/EN, capacitor derating, protection, connectors; KiCad ERC and review.