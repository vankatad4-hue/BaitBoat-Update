# Remote Power Net Draft R4

**Status:** Candidate component architecture; net-level draft only, not native KiCad/ERC/PCB/validated.

## 1 Battery/protection
J_BAT: PACK+, PACK−, NTC. Protected pack positive → F_MAIN Littelfuse 0458004.DR 4A fast-acting fuse candidate → Q_REV FDS6575 P-MOS source. Q_REV drain→BAT_PROT; gate→100kΩ→GND. Confirm pin numbering/body diode in schematic. PACK−→GND.
ADC divider: BAT_PROT→220kΩ→ADC_BAT; ADC_BAT→68kΩ→GND; 100nF ADC_BAT→GND. Approx 0.977V at 4.2V, 0.698V at 3.0V; 14.6µA at 4.2V.
NTC: controller 3V3→10kΩ pull-up→NTC_SENSE; 10kΩ B3950 NTC candidate from sense→GND; 100nF sense→GND. Fault detection required. Testpoints BAT_PROT/GND/ADC/NTC.

## 2 5V boost
TPS61023DRLR. BAT_PROT→VIN; 1µH BAT_PROT→SW; CIN 10µF + local 100nF→GND; VOUT→+5V_MAIN; COUT 2×22µF. Rtop 732kΩ +5V→FB; Rbottom 100kΩ FB→GND. EN default enabled from BAT_PROT via 100kΩ candidate; verify datasheet. +5V supplies Waveshare. Target 450mA continuous/1A transient, thermal validation pending.

## 3 Radio 3.3V
TPS63070 BAT_PROT→RADIO_3V3; 1.5µH between L1/L2; 470kΩ VOUT→FB, 150kΩ FB→GND. CIN 2×10µF 25V X7S 0805 +10µF 25V X5R 0603. COUT 3×22µF 16V X6S 0805 +10µF 0603 (76µF nominal). VAUX 100nF. VSEL low, FB2 GND, PS/SYNC config per datasheet.
EN MCU-controlled after boot, 1kΩ series candidate and 100kΩ pulldown; converter/radio default OFF during reset. Verify EN limits. PG optional. J_RADIO RADIO_3V3, GND, SPI SCK/MOSI/MISO, CSN, CE, IRQ; local 100nF+10–47µF.

## 4 Estimated budget at 3V
5V/450mA at assumed 90%≈0.83A; radio 3.3V/300mA at assumed 90%≈0.37A; continuous≈1.20A. 5V/1A transient at assumed 85% plus radio≈2.33A. Estimated only; fuse/BMS coordination not validated.

## 5 Capture requirements
Use power sheet capture specification and BOM shortlist. Freeze connector, symbols/footprints, verify pin/net mapping, ERC and independent review before PCB.
