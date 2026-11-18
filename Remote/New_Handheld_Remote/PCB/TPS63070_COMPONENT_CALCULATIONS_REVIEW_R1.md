# TPS63070 Radio Rail — Component Calculations & Design Review R1

**Updated:** 2026-11-18  
**Status:** Calculated baseline; not captured in native schematic and not validated on hardware.  
**Purpose:** Dedicated 3.3 V supply for nRF24L01+ PA/LNA module.  
**Input:** Protected 1S4P battery, provisional loaded range 3.0–4.2 V.

## Targets and assumptions
- Output: 3.3 V nominal, adjustable TPS63070.
- Radio load budget: 200 mA transient design envelope, not measured module consumption.
- Efficiency used for battery budget: 90%, assumed.
- Radio rail needs local 100 nF + 10–47 µF at the module connector, final value per module/vendor and load-step test.

## Feedback calculation
Assuming nominal FB reference 0.8 V and adjustable single-output divider:
\[
V_{OUT}=0.8(1+R_1/R_2)
\]
- R1 = 470 kΩ, 1%, VOUT to FB.
- R2 = 150 kΩ, 1%, FB to GND.
\[
V_{OUT}=0.8(1+470/150)=3.3067\ V
\]
Divider current = 3.3067/(620 kΩ) = 5.33 µA. Nominal calculated output is 3.307 V; tolerance includes FB accuracy and resistor tolerance.

## Inductor
Candidate: Coilcraft XFL4020-152ME, 1.5 µH, ±20%, shielded 4×4 mm.
Published candidate ratings recorded for review: 14.4 mΩ typical/15.8 mΩ maximum DCR; Isat 4.1 A at 10% inductance drop (4.4 A at 20%, 4.6 A at 30%); RMS 6.7 A at 20°C rise. Verify exact suffix, current criteria and vendor data before procurement.

## Capacitors — corrected baseline
**Input:** TI typical application values:
- CIN1/CIN2: 10 µF, 25 V X7S, 0805 (2 pieces).
- CIN3: 10 µF, 25 V X5R, 0603 local.
**Output:** TI typical application values:
- COUT1–COUT3: 22 µF, 16 V X6S, 0805 (3 pieces).
- COUT4: 10 µF, 0603 local.
- Nominal total = 76 µF.
**VAUX:** 100 nF X7R ceramic, placed close to pin.


22 µF capacitor candidate: Murata GRM21BC81C226ME44L (16 V X6S, 0805). Verify DC-bias capacitance at 3.3 V and temperature from manufacturer data. TI's nominal capacitance recommendations do not substitute for the effective capacitance check. Check capacitor tolerance and temperature against TI output filter guidance before schematic release.

## Configuration decisions — fixed 3.3 V rail
- FB2: GND for single-output adjustable configuration, verify against datasheet schematic.
- VSEL: LOW; static output, no dynamic voltage selection. Confirm pin-state interpretation and topology from datasheet before capture.
- PS/SYNC: HIGH to select power-save mode; verify recommended pull-up/control connection in datasheet. Evaluate radio burst noise/response on prototype; forced PWM is a fallback if RF or transient tests require it.
- EN: defined active state. Proposed 10 kΩ pull-up to BAT_PROT for always-on operation; do not leave floating. If MCU-controlled disable is later used, include a safe default and hardware control.
- PG: open-drain; optional 100 kΩ pull-up to 3V3_RADIO and MCU input, only after voltage/pin configuration is verified.

## Current budget
Radio-only at 3.307 V × 0.200 A = 0.661 W. At 3.0 V battery and 90% assumed efficiency:
I_BAT = 0.661/(3.0×0.90) = 245 mA.
At 4.2 V: 175 mA. These are estimates; PA/LNA bursts and actual module current must be measured.

## Whole remote preliminary estimate
- Display/controller estimate: 5 V × 450 mA = 2.25 W.
- Boost efficiency assumption: 85% → 2.647 W from battery.
- Radio output: 3.307 V × 200 mA = 0.661 W.
- Radio buck-boost efficiency assumption: 90% → 0.735 W from battery.
- Total estimated battery input power: 3.382 W.
- At 3.0 V battery: 1.13 A estimated input current.

This excludes joysticks, buttons, other peripherals, startup/inrush, conversion losses beyond the assumptions, and does not represent measured display consumption. Do not finalize pack, fuse, connector or wire ratings from this alone.

## Reference design and capture gates
TI typical application uses 1.5 µH, 2×10 µF 0805 + 10 µF 0603 input, 3×22 µF output + 10 µF 0603, and 100 nF VAUX. Verify exact pin mapping, package, VSEL/FB2 logic, PS/SYNC, EN, PG, effective capacitance, layout and thermal before capture. ERC and bench test are not yet completed.

Sources:
- TI TPS63070 Rev. B datasheet: https://www.ti.com/lit/ds/symlink/tps63070.pdf
- Coilcraft XFL4020 series: https://www.coilcraft.com/en-us/products/power/high-voltage-inductors/xfl/xfl4020/
- Murata GRM21BC81C226ME44L product data: https://www.murata.com/en-us/products/productdetail?partno=GRM21BC81C226ME44L
