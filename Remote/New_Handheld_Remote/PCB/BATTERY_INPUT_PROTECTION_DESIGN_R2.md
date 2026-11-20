# 1S4P Remote Battery Input Protection R2
**Status:** Architecture draft; parts, ERC, PCB and tests pending. Charging excluded.
1S4P matched Li-ion cells, 4.2V max; provisional 3.0V loaded budget. Pack BMS for overcharge/overdischarge/overcurrent/short, NTC and open/short detection. Match cells before parallel assembly.
Path: PACK+→BMS-protected→F_MAIN 4A time-delay candidate→Q_REV high-side P-MOS TBD→BAT_PROT; PACK−→GND. Fuse near pack; coordinate interrupt/time-current with fault current and conductors.
Budget assumptions: 5V/450mA=2.25W, at 90% and 3V≈0.83A. 5V/1A transient=5W, at 85% and 3V≈1.96A. Radio 3.3V/300mA=0.99W, at 90% and 3V≈0.37A. Combined ≈1.20A continuous, ≈2.33A transient. Excludes startup/other loads. 4A fuse and ≥5A BMS are provisional; wire/connector rating ≥5A target pending specs.
Reverse protection: high-side P-MOS with RDS(on) specified at suitable low VGS (about −2.5V), gate network/orientation per datasheet; calculate losses and verify.
BAT_ADC filtered divider, preferably switchable/high resistance; shutdown thresholds from cell datasheet, sag and calibration. BMS emergency only. NTC placement and fault handling required. Testpoints BAT_PROT, GND, ADC_BAT, NTC, +5V, RADIO_3V3.
Review: cell/BMS/fuse, connector/wire, MOSFET, ADC/NTC, schematic/ERC/review and controlled protection/thermal tests.