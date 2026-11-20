# TPS61023 5 V Rail — Calculations R1
**Status:** Selected candidate; not schematic/ERC/PCB/bench validated.
**Load:** 5 V for Waveshare ESP32-S3-Touch-LCD-4.3 Standard; planning 450 mA continuous, 1 A transient (not approved continuous). Verify board revision and measure. 1S4P Li-ion, provisional 3.0–4.2 V.
450mA=2.25W; at assumed 90% efficiency, 3V input ≈0.83A; at 4.2V ≈0.60A. 1A transient=5W; at assumed 85%, input ≈1.96A at 3V and 1.40A at 4.2V.
## IC
TPS61023DRLR SOT563 6-pin; VIN 0.5–5.5V, min startup 1.8V, typical 3.7A valley switch-current limit, 1MHz typical above 1.5V, internal compensation, light-load PFM. TI reference application: 5V/1.5A at 2.7–4.35V; reference is not thermal guarantee.
## Reference components
L=1µH; CIN=10µF; COUT=2×22µF; R1=732kΩ VOUT→FB; R2=100kΩ FB→GND. EN enabled from BAT_PROT via 100kΩ pull-up candidate (verify; MCU cannot sole-enable its own supply). Supplemental CIN 100nF. Approximate FB reference 0.6V gives 4.992V; confirm datasheet.
## Inductor candidate
Coilcraft XFL4020-102MEC: 1µH ±20%, DCR 10.8mΩ typ/11.9mΩ max, Isat 4.5A at 10% drop/5.1A at 20%, Irms 8A at 20°C rise, 4×4×2.1mm. Calculate converter peak current per datasheet.
## Caps/layout
Use 10V+ X7R/X7S caps; effective capacitance under bias must be checked. CIN directly at VIN/GND; L from BAT_PROT to SW; SW copper minimal; COUT near VOUT/GND; FB quiet and away from SW. Follow TI layout guide.
## Validation
Measure startup, ripple at 3.0/3.6/4.2V, real board current/backlight/Wi-Fi, 450mA steady, 1A load step, efficiency and enclosure thermal soak, overshoot/undershoot and controlled short protection. No 1A continuous claim before thermal test.
Sources: https://www.ti.com/lit/ds/symlink/tps61023.pdf ; https://www.ti.com/product/TPS61023 ; https://www.coilcraft.com/en-us/products/power/shielded-inductors/xfl/xfl4020/ ; https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3