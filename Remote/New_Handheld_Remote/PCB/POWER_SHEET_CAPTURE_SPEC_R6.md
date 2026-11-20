# Remote Power Capture Specification R6

## Sheet A — pack input
1S4P pack; keyed J_PACK; PACK+→4A fast fuse candidate→PACK_FUSED→reverse P-MOS FDS6575 candidate Drain; Source→BAT_PROT; Gate100k→GND. Verify pins/body diode. PACK−→GND. BAT_PROT bulk100µF+10µF+100nF. Fuse close to pack. Coordinate pack BMS, wire, connector and fuse.

## Sheet B — controller/display
BAT_PROT/GND→Waveshare PH2.0 battery input; polarity physically verify. CS8501 onboard boost powers controller/display only. Do not parallel any external boost. Bench board load, USB charging/backfeed, 5V ripple, droop, startup and thermal.

## Sheet C — nRF RADIO_3V3
BAT_PROT→TPS63070 buck-boost→RADIO_3V3. 1.5µH candidate, FB470k/150k candidate, input/output capacitor values from effective capacitance and datasheet reference. EN default OFF. Rail to nRF only. Local100nF+10–47µF.

## Sheet D — E22 LORA_5V
BAT_PROT→TPS61023 dedicated boost→LORA_5V→E22 VCC. CS8501 is not shared. Use reference design; exact inductor, caps, feedback, layout and thermal validate. Reserve150mA TX peak. E22 UART/M0/M1/AUX 3.3V logic. Local100µF+10µF+100nF. No converter-output paralleling.

## Sheet E — monitoring
Battery divider220k/68k+100nF to ADS1115@0x4B AIN0; NTC10k pullup/10k B3950+100nF AIN1; optional RADIO_3V3 divider AIN2; AIN3 spare. Check ADC range, calibrate and detect open/short.

## Test pads
BAT_PROT,5V_CONTROLLER,RADIO_3V3,LORA_5V,GND,converter inputs/outputs,UART,CE/CSN,SPI,I2C,ADC.
