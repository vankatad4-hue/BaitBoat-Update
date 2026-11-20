# 5 V Boost Converter Selection Review R2 — Decision
**Decision:** Adopt TPS61023DRLR as 5V boost candidate pending schematic/PCB/bench.
Requirement: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 450mA continuous planning load and 1A transient target (not continuous thermal rating); 1S4P input provisional 3.0–4.2V. Chosen over TPS61088 for simpler reference design/internal compensation and fewer parts.
TI reference (5V/1.5A, 2.7–4.35V input): 1µH, CIN 10µF, COUT 2×22µF, R1 732k VOUT→FB, R2 100k FB→GND, EN control. Values are starting points; verify effective cap, pinning, inductor current.
Coilcraft XFL4020-102MEC candidate: 1µH ±20%, DCR 10.8mΩ typ/11.9mΩ max, Isat 4.5A at 10%/5.1A at 20%, Irms 8A at 20°C rise, 4×4×2.1mm. Calculate peak and test.
450mA=2.25W; at 90% and 3V input≈0.83A. 1A transient=5W; at 85%≈1.96A. SOT563 thermal depends on PCB/enclosure. No continuous 1A claim without measurement. EN defaults ON because controller uses this rail.
Verify actual board load, package/pin, FB reference/EN, cap bias, peak-current calc, schematic/ERC, review, startup/ripple/load-step/efficiency/thermal tests.
Sources: TI datasheet https://www.ti.com/lit/ds/symlink/tps61023.pdf ; TI page https://www.ti.com/product/TPS61023 ; Coilcraft https://www.coilcraft.com/en-us/products/power/shielded-inductors/xfl/xfl4020/