# Remote Power Net Draft R8 — dual radio

## Pack/protection
1S4P PACK+→4A fast fuse candidate→PACK_FUSED→reverse P-MOS candidate Drain; Source→BAT_PROT; Gate100k→GND. PACK−→GND. Verify MOSFET pins/body diode. Bulk100µF+10µF+100nF. Fuse near pack. BMS/cell/wire/connector/current/thermal coordination.

## Controller/display
BAT_PROT/GND→Waveshare PH2.0 battery input, polarity verify. CS8501 onboard boost powers controller/display only. Test load, USB charging/backfeed and thermal; do not parallel external boost.

## nRF primary 3V3
BAT_PROT→TPS63070→RADIO_3V3; 1.5µH candidate; feedback470k/150k; input/output capacitors from datasheet effective-capacitance/stability calculations. EN reset-safe OFF. RADIO_3V3→nRF only.

## E22 backup 5V
BAT_PROT→TPS61023 dedicated boost→LORA_5V→E22 VCC. Candidate5V/2A-class converter; exact reference design, inductor, capacitor, feedback, layout and thermal to calculate/verify. CS8501 rail is not shared; no parallel converter outputs. Reserve150mA module TX peak. UART/M0/M1/AUX logic3.3V. Local100µF+10µF+100nF; scope at10dBm TX.

## ADC
Battery divider220k/68k+100nF to ADS1115 AIN0; NTC10k pullup/10k B3950+100nF AIN1; optional radio rail divider AIN2. Check ADC range.

## Measurement estimate
Assumed controller5V/450mA, E22 5V/150mA peak, nRF3.3V/300mA burst,90% converter efficiencies. At pack3V estimated ~1.48A; not measured. Use bench current measurement and peak recording.
