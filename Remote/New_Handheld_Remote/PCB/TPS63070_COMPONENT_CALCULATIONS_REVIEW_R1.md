# TPS63070 Radio Rail — Component Calculations & Design Review R1

**Date:** 2026-11-18  
**Status:** Calculated design baseline; not schematic/ECR/layout validated.  
**Rail:** BAT_PROT (1S4P) → TPS63070 → 3.3 V radio supply  
**Reference:** TI TPS63070 Rev. B datasheet, https://www.ti.com/lit/ds/symlink/tps63070.pdf

## 1. Electrical targets
- Battery input design range: 3.0–4.2 V provisional; verify pack chemistry, protection cutoff and loaded voltage.
- Output target: 3.3 V.
- Radio load design envelope: 200 mA (conservative working envelope; not measured module current).
- Assumed efficiency for budget: 90%; actual efficiency must be taken from TI curves/design and measured.
- The TPS63070's 2.0–16 V input and 2.5–9 V output ranges cover the target; published current capability is operating-condition dependent and is not a board guarantee.

## 2. Feedback resistor calculation
The adjustable version uses nominal FB reference 0.8 V:

\[
V_{OUT}=0.8\left(1+\frac{R_1}{R_2}\right)
\]

Choose TI's published typical 3.3 V pair:
- R1 = 470 kΩ, VOUT-to-FB
- R2 = 150 kΩ, FB-to-GND

Calculated nominal output:
\[
V_{OUT}=0.8(1+470/150)=3.3067\ V
\]
Feedback divider current:
\[
I_{DIV}=3.3067/(470k+150k)=5.33\ \mu A
\]
Nominal result is 3.307 V. The pair is a selected starting value and resistor tolerance/FB error must be included in final output tolerance. Use 1% resistors unless design review gives a reason otherwise. TI's datasheet lists this pair for 3.3 V.

## 3. Inductor selection
**Candidate selected:** Coilcraft XFL4020-152ME, 1.5 µH, ±20%, 4 × 4 × 2.1 mm.
Published values from Coilcraft:
- DCR: 14.4 mΩ typical / 15.8 mΩ maximum
- Isat: 4.1 A at 10% inductance drop; 4.4 A at 20%; 4.6 A at 30%
- RMS current: 6.7 A for 20°C rise; 9.1 A for 40°C rise
- SRF: 59 MHz typical
The TPS63070 datasheet lists this 1.5 µH part as a recommended inductor. Verify ordering suffix, temperature, pad geometry and current criteria in the chosen library and supplier listing before BOM release.

## 4. Capacitors — reference set and conservative output-bank proposal
TI typical application BOM:
- Input: 2 × 10 µF, 25 V, X7S, 0805 plus 1 × 10 µF, 25 V, X5R, 0603 local
- Output: 3 × 22 µF, 16 V, X6S, 0805 plus 1 × 10 µF, 25 V, X5R, 0603 local
- VAUX: 100 nF ceramic local capacitor

For this design, use the following **provisional** bank pending DC-bias curves:
- CIN: retain 2 × 10 µF 25 V X7S 0805 + 1 × 10 µF 25 V X5R 0603 local.
- COUT: increase the nominal bank to 4 × 22 µF 0805 X6S/X7S + 1 × 10 µF 0603 local; nominal sum = 98 µF.
- CVAUX: 100 nF X7R 0402 close to VAUX.

Important: TI's filter table for 1.5 µH indicates 47 µF effective output capacitance as a recommended combination, with tolerance/DC-bias derating assumptions. Nominal 98 µF does **not** guarantee 47 µF effective. Confirm exact capacitor models' bias curves at 3.3 V, tolerance and temperature. If bank cannot maintain the effective capacitance, select higher nominal capacitance or a different verified combination. This capacitor-bank increase is a design proposal, not a proven stable configuration.

## 5. Radio current budget
Assume 3.3 V output and 90% conversion efficiency:
\[
I_{BAT}=V_{OUT}I_{OUT}/(V_{BAT}\eta)
\]
At 3.0 V battery and 200 mA radio rail:
- Output power = 3.3067 × 0.200 = 0.6613 W
- Battery input power = 0.6613 / 0.90 = 0.7348 W
- Battery current = 0.7348 / 3.0 = **245 mA**
At 4.2 V battery, same load and efficiency, input current = 175 mA.
Scenarios at 3.0 V, 90% assumed efficiency:
| Radio output | Battery current |
|---:|---:|
| 50 mA | 61 mA |
| 100 mA | 122 mA |
| 150 mA | 184 mA |
| 200 mA | 245 mA |

## 6. Whole-remote preliminary budget
This is a calculation, not measured current:
- Display/controller assumed 5 V × 450 mA = 2.25 W.
- 5 V converter assumed 85% efficient: battery power for display = 2.647 W.
- Radio at 3.3067 V × 200 mA = 0.6613 W; 90% radio converter: battery power = 0.735 W.
- Combined battery power = 3.382 W.
- At 3.0 V battery: **1.13 A estimated battery current**.
This excludes any additional loads and transient/inrush margin. Verify display/controller current for exact board revision, brightness, Wi-Fi/Bluetooth state and USB peripherals. The 450 mA figure is an earlier estimate, not confirmed datasheet maximum.

## 7. Component-value baseline (not final BOM)
| Ref | Value / part | Status |
|---|---|---|
| U_RADIO | TPS63070RNM/RNMT exact suffix to verify | Candidate |
| R1 | 470 kΩ, 1% | Calculated selection |
| R2 | 150 kΩ, 1% | Calculated selection |
| L_RADIO | Coilcraft XFL4020-152ME, 1.5 µH | Candidate selected |
| CIN1, CIN2 | 10 µF, 25 V, X7S, 0805 | TI reference values |
| CIN3 | 10 µF, 25 V, X5R, 0603 | TI reference value |
| COUT1–4 | 22 µF, 0805, X6S/X7S, voltage rating/model TBD | Provisional bank |
| COUT5 | 10 µF, 0603, X5R, model TBD | TI local reference value |
| CVAUX | 100 nF, X7R, 0402 | Reference |
| R_PGOOD | Optional pull-up; value/rail TBD | Optional |
| EN pull/control | Defined default; resistor value/source TBD | Must not float |
| PS/SYNC | Mode level TBD; assess power-save vs forced PWM | Open decision |
| VSEL/FB2 | Single-output configuration to follow datasheet exactly | Open verification |

## 8. Schematic capture gates
Before committing schematic symbols/footprints:
1. Confirm selected IC ordering suffix, package and symbol pin numbers against TI drawing.
2. Verify VSEL/FB2 single-output wiring and PS/SYNC level against the datasheet truth table/reference circuit. Do not infer pin logic from an incomplete secondary diagram.
3. Verify exact capacitor part numbers and effective capacitance under 3.3 V DC bias.
4. Confirm inductor Isat and RMS current criteria at operating temperature.
5. Define EN reset/default state and optional PG connection.
6. Capture the circuit with datasheet/reference schematic annotations.
7. Run ERC after native schematic capture and review high-current loop/grounding.
8. Bench-test output regulation, ripple, load steps, radio transmit bursts and temperature at 3.0/3.6/4.2 V inputs before integrating radio.

## 9. Layout-critical items
- Keep VIN/input-capacitor and VOUT/output-capacitor loops short and compact.
- Place inductor adjacent to L1/L2; keep both switch nodes small.
- Place local 0603 input/output capacitors close to IC pins.
- Feedback divider close to FB; route FB away from SW copper and radio RF feed.
- Keep radio supply/return separate from display converter high-current routing until they join the main ground plane; use continuous ground reference.
- Follow TI exposed-pad and thermal via recommendations for the exact package.

## 10. Review status
**Calculated:** feedback resistor pair/output, divider current, radio input current scenarios, combined remote current estimate.  
**Selected candidate:** 1.5 µH Coilcraft XFL4020-152ME.  
**Proposed:** input/output capacitor bank; output bank increased nominally to 98 µF pending bias verification.  
**Not validated:** effective capacitance, stability, loop response, ripple, transient response, efficiency, thermal performance, PCB layout, ERC or bench tests.  
This document is an engineering calculation/review record and does not claim a native KiCad schematic or fabrication-ready design.

## Sources
- TI TPS63070 datasheet, Rev. B: https://www.ti.com/lit/ds/symlink/tps63070.pdf
- TI product page: https://www.ti.com/product/TPS63070
- Coilcraft XFL4020-152ME: https://www.coilcraft.com/en-us/products/power/high-voltage-inductors/xfl/xfl4020/xfl4020-152/

## Datasheet correction and configuration review update (2026-11-18)
- TPS63070 adjustable single-rail configuration: FB2 grounded; VSEL held LOW (no dynamic voltage scaling); PS/SYNC HIGH to enable power-save PFM/PWM operation; EN pulled high through a 10 kΩ resistor to BAT_PROT for always-enabled rail. These are the design choices to implement, pending symbol/net review. If MCU shutdown control is added, it must safely override EN.
- PG is open-drain; 100 kΩ pull-up to 3V3_RADIO is a candidate for MCU status monitoring, not mandatory. Verify the intended MCU input and PG behavior before capture.
- Capacitor-bank proposal corrected: follow TI's adjustable 3.3 V typical application BOM (3×22 µF X6S 0805 + 10 µF 0603, nominal 76 µF), rather than the earlier 98 µF proposal. TI's filter table supports 1.5 µH with nominal 47/68/100 µF; the 76 µF typical-app bank is within the recommended combination table. Effective capacitance remains to be confirmed using manufacturer DC-bias/temperature data. Datasheet permits effective output capacitance 15–470 µF for nominal 1.5 µH.
- Selected 22 µF candidate: Murata GRM21BC81C226ME44L, 16 V X6S, 0805, ±20%, -55°C to +105°C. Exact DC-bias capacitance at 3.3 V is not stated in the basic part specification and must be verified from Murata's bias curves/approval data. Three units are specified by TI's reference BOM.
- The prior 98 µF nominal output proposal is superseded. See the calculation review and pin/net map for corrected baseline.
Sources: TI TPS63070 Rev. B datasheet, §§8.5.1, 9.2.2.1–9.2.2.3, Tables 2–4; Murata GRM21BC81C226ME44L reference sheet.
