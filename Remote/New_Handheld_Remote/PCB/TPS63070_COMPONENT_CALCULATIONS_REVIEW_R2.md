# TPS63070 Radio Rail — Component Calculations & Design Review R2

**Status:** Datasheet-checked engineering draft; not captured in native schematic or hardware-validated.
**Purpose:** Dedicated 3.3 V supply for the nRF24L01+ PA/LNA module.
**Input:** Protected 1S4P pack, provisional 3.0–4.2 V loaded design range.

## 1. Specification
- Output rail: 3.3 V nominal.
- Design load budget: 200 mA burst assumption; consider 300 mA capacity margin for prototype testing.
- Input design range: 3.0–4.2 V.
- TPS63070 adjustable buck-boost converter.
- Power-save mode preferred initially; forced PWM is a test alternative if radio burst noise or transient response requires it.
- Output local radio connector decoupling: 100 nF + 10–47 µF, with final capacitance chosen after load-step testing.

## 2. Feedback divider

TI reference is 0.8 V. Formula:
R1 = R2 × (VOUT/0.8 − 1).

For 3.3 V:
- R1 = 470 kΩ, 1%, VOUT→FB.
- R2 = 150 kΩ, 1%, FB→GND.
- Calculated nominal output = 0.8 × (1 + 470/150) = 3.3067 V.
- Divider current ≈ 5.33 µA.

These values are listed by TI as typical values for 3.3 V. The divider current exceeds the datasheet minimum recommended 2 µA. PFM output accuracy is specified differently from PWM; confirm rail tolerance against the radio module's maximum supply.

## 3. Inductor

Candidate: Coilcraft XFL4020-152MEC, 1.5 µH, shielded.
- ±20% tolerance.
- DCR max 15.8 mΩ.
- Saturation current 4.1 A at 10% inductance drop, 4.4 A at 20% drop, 4.6 A at 30% drop.
- RMS 6.7 A at 20°C rise (vendor rating).
- 4.0 × 4.0 × 2.1 mm nominal package.

At 3.0 V input, 3.3 V output and 200 mA load, using 90% assumed efficiency:
- Boost duty D ≈ (3.3−3.0)/3.3 = 0.091.
- Average inductor current ≈ IOUT/[η(1−D)] = 0.244 A.
- Datasheet peak-current equation using f≈2.4 MHz and L=1.5 µH gives ripple half-amplitude ≈ 38 mA.
- Approximate peak ≈ 0.28 A.

This is well below the published current ratings. Recalculate for the measured PA/LNA burst envelope and real operating conditions; confirm inductance reduction under bias.

## 4. Input capacitor network
Use TI's typical adjustable TPS63070 application as baseline:
- CIN1/CIN2: 2 × 10 µF, 25 V X7S, 0805.
- CIN3: 10 µF, 25 V X5R, 0603, local.
- Ensure at least 4.7 µF effective input capacitance (datasheet minimum); place small capacitor closest to VIN/GND pins.

## 5. Output capacitor network
Baseline:
- COUT1–3: 3 × 22 µF, 16 V X6S, 0805.
- COUT4: 10 µF, 0603, local.
- Nominal total = 76 µF.

For adjustable TPS63070 with 1.5 µH nominal inductor, TI specifies an effective output capacitance ≥10× effective inductance (µF per µH); 15 µF effective is the minimum for 1.5 µH and 47 µF is the typical nominal selection. The proposed bank exceeds the nominal recommendation but **effective capacitance must be checked at 3.3 V DC bias, tolerance and temperature**. If actual effective C falls below 47 µF, increase nominal capacitance or choose lower-bias capacitors.

## 6. VAUX bypass
- CVAUX = 100 nF X7R, 25 V, 0402/0603, placed close to VAUX pin.
- VAUX must not be loaded externally.

## 7. Pin configuration / controls

- VSEL: tie LOW to GND for a single fixed output voltage; no dynamic voltage selection.
- FB2: GND for unused voltage-selection path, subject to confirming the adjustable-app schematic and pin function in the CAD capture.
- PS/SYNC: HIGH to select PWM/PFM power-save operation; connect through appropriate resistor/control network as TI recommends. LOW selects forced PWM.
- EN: must not float. Proposed 10 kΩ pull-up to BAT_PROT for always-on radio rail; alternatively MCU-controlled enable with a safe default.
- TI recommends a 10 kΩ series resistor when digital inputs are tied to VIN; EN/PS-SYNC implementation must follow this guidance.
- PG: open-drain output. Optional pull-up (100 kΩ candidate) to RADIO_3V3 and MCU status input; confirm MCU pin and PG behavior.

## 8. Layout
- Place inductor, input capacitors, output capacitors and IC tightly around the switching power loop.
- Separate control-ground return from power-ground return; connect per TI layout guidance near the IC ground.
- Feedback divider close to FB, with quiet return.
- Keep radio supply trace short and wide; connector-side 100 nF and bulk capacitor.
- Keep RF antenna region clear; avoid switching nodes and large copper/metal around antenna.

## 9. Battery current estimates

At 3.0 V battery and 90% efficiency:
- 3.3 V × 200 mA = 0.66 W → ~0.244 A input.
- 3.3 V × 300 mA = 0.99 W → ~0.367 A input.

For worst-case estimated 5 V boost load of 1 A (5 W at 75% assumed) plus 300 mA radio (0.99 W at 90% assumed):
- Battery power ≈ 6.67 W + 1.10 W = 7.77 W.
- Battery current at 3.0 V ≈ 2.59 A, excluding other loads, startup, and real efficiency.

## 10. Verification checklist
- [ ] Confirm exact nRF PA/LNA board supply limit and burst current from its documentation/measurement.
- [ ] Confirm TPS63070 package symbol/footprint and exact pin numbering.
- [ ] Verify EN, PS/SYNC, VSEL, FB2 and PG wiring.
- [ ] Confirm effective capacitor values at DC bias.
- [ ] Validate 3.3 V rail with radio TX burst, RX, sleep, display/Wi-Fi active.
- [ ] Measure rail ripple, droop, efficiency and thermal rise.
- [ ] Native schematic capture and ERC.

## Sources
- TI TPS63070 Rev. B datasheet: https://www.ti.com/lit/ds/symlink/tps63070.pdf
- Coilcraft XFL4020 series: https://www.coilcraft.com/en-us/products/power/shielded-inductors/xfl/xfl4020/
