# Remote Power Net Draft R6

**Status:** Proposed revised baseline using Waveshare onboard 5V boost; not native KiCad/ERC/PCB/bench validated.

## Battery and protection
BMS-protected 1S4P pack output → F_MAIN 4A fast-acting fuse candidate → Q_REV FDS6575 Drain; Q_REV Source→BAT_PROT; Gate→100kΩ→GND. Verify pin/body diode. BAT_PROT feeds Waveshare PH2.0 battery input and TPS63070 VIN. PACK−→GND.
ADC: BAT_PROT→220k→ADC_BAT; 68k ADC→GND; 100nF filter. NTC: controller 3V3→10k pull-up→NTC_SENSE; 10k B3950 candidate NTC→GND; 100nF. Detect open/short.

## Controller 5V
Do **not** add TPS61023 to baseline. Waveshare ESP32-S3-Touch-LCD-4.3 Standard has an onboard CS8501 battery/boost-management circuit and a single-cell battery header; board documentation describes boosting one Li-ion cell to 5V. Connect protected BAT_PROT/GND to board battery input, connector polarity verified. Test onboard boost for actual 450mA board load and transients. TPS61023 remains fallback only if tests show insufficiency; never parallel boost outputs.

## Radio 3.3V
TPS63070 BAT_PROT→VIN; 1.5µH between L1/L2; 470k VOUT→FB, 150k FB→GND; CIN 2×10µF 25V X7S 0805+10µF 25V X5R 0603; COUT 3×22µF 16V X6S 0805+10µF 0603; nominal 76µF; VAUX 100nF. Verify effective capacitance/configuration. EN MCU-controlled after boot, 1k series candidate and 100k pulldown; OFF during reset. PG optional. J_RADIO local 100nF+10–47µF, SPI/control pinout TBD.

## Budget estimate @3V loaded
Controller board 5V/450mA at assumed 90% onboard conversion ≈0.83A pack. Radio 3.3V/300mA at assumed 90% ≈0.37A. Combined ≈1.20A continuous. This is an estimate; measure onboard boost efficiency/load. Fuse/BMS coordination pending.
