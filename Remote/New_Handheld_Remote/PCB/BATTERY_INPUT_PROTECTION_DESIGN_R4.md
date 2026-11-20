# Battery Protection R4 — Reverse MOS Orientation Corrected

Status: candidate design, not validated. Charger excluded.

Pack: 1S4P matched Li-ion, 4.2V maximum; provisional 3.0V loaded design point. BMS and pack NTC required.

Estimated load at 3V: 5V/450mA at 90% efficiency ≈0.83A; 5V/1A transient at 85% ≈1.96A; radio 3.3V/300mA at 90% ≈0.37A. Combined ≈1.20A continuous, ≈2.33A simultaneous transient. Estimates only.

Fuse candidate: Littelfuse 0458004.DR, 4A 1206 fast-acting, 75VDC rating/50A interrupt per listing. Verify I²t, pulse handling, derating and pack fault coordination. Not time-delay.

Reverse MOS candidate: onsemi FDS6575 P-channel, −20V, SO-8, 17mΩ max at VGS=−2.5V. **Drain to fused pack-input side; Source to BAT_PROT/load side; Gate to GND through 100kΩ.** This orientation is intended to forward-bias the body diode during correct-polarity startup and reverse-bias it with reversed input. Verify pin map/body diode and schematic before capture. Approximate conduction loss using 17mΩ: 2.33A → 0.092W; 1.20A → 0.024W; actual temperature-dependent resistance and tests required.

Battery ADC: 220k BAT_PROT→ADC, 68k ADC→GND, 100nF filter; ~0.977V at 4.2V and ~0.698V at 3V, ~14.6µA at 4.2V. Verify ESP32 ADC pin/attenuation/calibration.

NTC: 10k B3950 candidate, 10k pull-up from controller 3V3 to sense, NTC to GND, 100nF. Detect open/short. Fuse near pack positive.

Path: BMS PACK+→fuse→Q_REV Drain; Q_REV Source→BAT_PROT; Gate→100k→GND; PACK−→GND. BAT_PROT feeds both converters. Components and protection coordination pending.
