# 5 V Boost Converter Selection Review R1

**Status:** Engineering recommendation; final IC choice is not frozen.
**Current candidate:** TPS61088RHLR.
**Alternative evaluated:** TPS61023DRLR.

## 1. Actual remote load

Waveshare ESP32-S3-Touch-LCD-4.3 standard board planning consumption is approximately 5 V / 450 mA (vendor figure; confirm exact board/revision and measure).

Recommended power-rail specification for the remote:
- 5.0 V nominal.
- 450 mA continuous design load (board's planning figure).
- 1.0 A transient/load-step capability target.
- Battery input 3.0–4.2 V operating design range.
- Low-noise, short high-current path; local bulk/ceramic capacitors near the display board.

The 1 A requirement should be treated as a transient capability target unless a measured system load requires 1 A continuous.

## 2. Comparison

| Attribute | TPS61088 | TPS61023 |
|---|---|---|
| Topology | Synchronous boost | Synchronous boost |
| Input | 2.7–12 V | 0.5–5.5 V |
| Output | 4.5–12.6 V | 2.2–5.5 V |
| Switch-current capability | 10 A class | 3.7 A typical valley limit |
| 5 V reference application | High power; 9 V/3 A typical datasheet example | 5 V/1.5 A from single-cell Li-ion |
| Frequency | Adjustable 200 kHz–2.2 MHz | 1 MHz typical above 1.5 V |
| Compensation | External | Internal |
| Package | 20-pin VQFN 4.5 × 3.5 mm | 6-pin SOT563 1.2 × 1.6 mm |
| External parts | More complex | Fewer |
| Thermal/layout sensitivity | Large power converter | Tiny package; layout copper critical |

## 3. TPS61023 reference design baseline (if selected)

TI's 5 V / 1.5 A reference application uses:
- L = 1 µH.
- CIN = 10 µF.
- COUT = 2 × 22 µF.
- R1 = 732 kΩ, VOUT→FB.
- R2 = 100 kΩ, FB→GND.
- EN controlled.
- Internal loop compensation; no external compensation required for the standard reference setup.

For our use, start with these values, but verify effective capacitor values under 5 V bias, actual inductor rating, PCB thermal layout and the 1 A transient requirement.

Candidate inductor: Coilcraft XFL4020-102MEC, 1 µH. Published ratings: DCR max 11.9 mΩ, Isat 4.5 A at 10% inductance drop, Irms 8 A at 20°C rise. Check exact manufacturer suffix and current curve.

## 4. Load/thermal interpretation

At 450 mA, output power is 2.25 W. Assuming 85–90% efficiency, estimated converter loss is about 0.25–0.40 W. The TPS61023 SOT563 package thermal resistance is layout-dependent; TI lists 142.7°C/W for a standard test condition and 91.4°C/W for its evaluation-board condition. This means thermal margin must be checked on the actual PCB, in the enclosure, with display brightness and radio activity.

At 1 A, output power is 5 W. A small SOT563 converter can have substantially higher losses and junction temperature; therefore 1 A is not accepted as a continuous design load without measured thermal evidence. TI's reference data includes a 1 A line transient and 800 mA-to-1.5 A load transient test at its stated conditions, supporting use as a candidate for short load peaks, not automatic continuous rating.

## 5. Why TPS61023 is preferred for this project

The remote's actual expected display/controller load is about 450 mA, while the current TPS61088 proposal is a much higher-power device that requires external loop compensation and more external parts. The TPS61023 has integrated compensation and a tested 5 V single-cell application, reducing schematic and design complexity and board area.

**Recommendation:** Change the 5 V rail candidate to TPS61023, target 450 mA continuous with 1 A transient margin, and validate thermal performance. Keep TPS61088 only as a fallback if measured continuous load, thermal conditions or transient requirements prove beyond TPS61023's practical capability.

## 6. Decision gates
- [ ] Confirm Waveshare board's exact revision and measure steady-state, startup and peak current.
- [ ] Confirm the remote is to use 450 mA continuous and 1 A transient specification.
- [ ] Check TPS61023 inductor/capacitor derating and exact package.
- [ ] Check thermal layout with copper area/vias and enclosure.
- [ ] Test startup, 450 mA continuous, 1 A load step, battery at 3.0 V, ripple and thermal soak.
- [ ] Freeze IC choice before native schematic capture.

## Sources
- TPS61088 Rev. D datasheet: https://www.ti.com/lit/ds/symlink/tps61088.pdf
- TPS61023 Rev. B datasheet: https://www.ti.com/lit/ds/symlink/tps61023.pdf
- TPS61023 TI product page: https://www.ti.com/product/TPS61023
- Waveshare display/controller documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3
