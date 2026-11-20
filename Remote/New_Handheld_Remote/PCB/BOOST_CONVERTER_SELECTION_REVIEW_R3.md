# 5V Controller Rail Review R3 — Onboard Converter Preferred

**Revised recommendation:** Do not populate the external TPS61023 as baseline. The selected Waveshare ESP32-S3-Touch-LCD-4.3 Standard board already includes a PH2.0 single-cell battery interface and CS8501 charge/discharge management circuit that boosts a single Li-ion cell to 5V. Manufacturer specifies board consumption 5V/450mA. That makes the external boost redundant for the intended battery configuration.

Connect the protected 1S4P pack to the board battery input, with external pack protection/fuse. Use the onboard converter for controller/display 5V. Keep the TPS61023 design as a fallback if bench tests show the onboard boost cannot support the actual load/transients. Never parallel boost outputs.

**Unknown:** CS8501 detailed output current and thermal limits were not confirmed from a primary datasheet. Manufacturer board consumption is not a converter transient guarantee.

Bench tests: startup; max backlight; Wi-Fi; simultaneous radio TX; low battery; 5V ripple/droop; board resets; input current; onboard boost temperature. Confirm board revision, battery header polarity and USB charging interactions.

Sources: Waveshare board docs https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3 ; board schematic https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3/ESP32-S3-Touch-LCD-4.3-Sch.pdf ; TPS61023 study retained in repo as fallback.
