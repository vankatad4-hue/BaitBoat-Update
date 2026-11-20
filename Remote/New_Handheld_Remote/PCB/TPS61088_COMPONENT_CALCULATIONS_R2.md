# TPS61088 5 V Boost — Component Calculations R2

**Status:** Engineering design review draft. Values marked proposed are not yet fabrication-ready. No schematic, simulation, PCB, or bench validation is claimed.

## 1. Load and rail specification

- Battery: 1S4P Li-ion, 3.0 V minimum design point (provisional), 4.2 V maximum.
- Target output: 5.0 V.
- Waveshare ESP32-S3-Touch-LCD-4.3 standard board: vendor-stated 5 V / 450 mA typical planning figure; verify actual board revision and load.
- Nominal rail load: 450 mA = 2.25 W.
- Design rail target: 1.0 A (5 W) to provide margin; not a guaranteed capability until tested.
- Switching frequency target: approximately 1 MHz.
- Light-load mode: PFM (MODE left floating per TPS61088 datasheet).
- Converter: TPS61088RHLR, 20-pin VQFN, TI datasheet Rev. D.

## 2. Feedback divider

TPS61088 PWM reference is 1.204 V typical.

Using Rbottom = 56.0 kΩ:
Rtop = (5.0 V − 1.204 V) × 56.0 kΩ / 1.204 V = 176.5 kΩ.

Proposed E96 value: 177 kΩ, 1%.

Nominal output:
VOUT = 1.204 × (1 + 177/56) = 5.008 V.

Divider current is approximately 21.5 µA. Review the PFM reference shift and tolerance in the final design.

## 3. Frequency programming resistor

TI equation:
R_FSW = 4 × [1/fSW − tDELAY × (VOUT/VIN)] / CFREQ
where CFREQ = 23 pF and tDELAY = 89 ns.

At 1 MHz, VOUT = 5 V:
- VIN = 3.0 V: calculated R = 148.1 kΩ.
- VIN = 4.2 V: calculated R = 155.5 kΩ.

Proposed 150 kΩ, 1%, gives roughly 0.99–1.04 MHz across the input range (calculated from the equation; actual behavior must be measured). FSW resistor connects FSW pin to SW as required by datasheet.

## 4. Peak-current limit

PFM-mode relationship: I_LIMIT ≈ 1,190,000 / R_ILIM (ohms).

Proposed R_ILIM = 249 kΩ, 1%:
- Typical limit ≈ 4.78 A.
- TI notes worst-case limit can be approximately 1.3 A lower than calculated, so conservative minimum ≈ 3.48 A.

The selected limit must be greater than worst-case operating inductor peak current, with margin. This is a protection threshold, not the expected operating current.

## 5. Inductor

Proposed: Coilcraft XAL5030-222MEC, 2.2 µH, shielded molded type.
- ±20% inductance tolerance; minimum nominal-tolerance value 1.76 µH before bias effects.
- Maximum DCR 14.5 mΩ.
- Isat listed 9.2 A.
- Irms listed 7.2 A at 20°C rise / 9.7 A at 40°C rise.
- Package approx. 5.0 × 5.0 × 3.0 mm.

This part is comfortably above the estimated 2.6 A operating peak and proposed 4.78 A nominal current limit on published saturation rating, but verify the manufacturer's current/inductance curve, thermal conditions, actual land pattern and PCB airflow.

## 6. Conservative peak-current estimate

Assume 5 V / 1 A output, VIN = 3.0 V, efficiency = 75%, L = 1.76 µH (−20%), f = 0.9 MHz (conservative low-frequency assumption).

Inductor average current:
I_L,DC = VOUT × IOUT / (VIN × η) = 5 × 1 / (3 × 0.75) = 2.22 A.

Boost ripple estimate:
ΔI_L = VIN × (VOUT − VIN) / (VOUT × L × f)
= 3 × 2 / (5 × 1.76 µH × 0.9 MHz)
≈ 0.76 A p-p.

Peak estimate:
I_L,peak ≈ 2.22 + 0.76/2 = 2.60 A.

This estimate is simplified and does not include every parasitic, transient, or controller-specific behavior. It leaves roughly 0.88 A between this estimate and the conservative 3.48 A current-limit floor.

## 7. Capacitors

### Input
- At least 10 µF effective ceramic capacitance at the converter input, plus 100 nF placed very close to VIN.
- Start with 2 × 22 µF ceramic, 10 V or higher, plus 100 nF; verify effective capacitance under 3–4.2 V bias and temperature.
- A separate battery bulk capacitor may be added after system transient measurements.

### VCC
- 2.2 µF ceramic between VCC and ground, placed adjacent to the pin.

### BOOT
- 100 nF ceramic between BOOT and SW, adjacent to pins.

### Output
- 3 × 22 µF ceramic, 10 V rating minimum, in parallel as initial selection.
- Check effective capacitance under 5 V DC bias; nominal capacitance must not be treated as effective.
- TI's example recommends three 22 µF ceramic output capacitors and warns about DC-bias derating.
- Target effective COUT is to be verified; 30–50 µF effective is a useful starting target for ripple/transient calculations.

## 8. Soft-start

Proposed C_SS = 47 nF.
Using tSS = VREF × CSS / ISS, with VREF = 1.204 V and ISS = 5 µA:
tSS ≈ 11.3 ms. TI describes 47 nF as generally sufficient for most applications.

## 9. Loop compensation — deliberately not frozen

TPS61088 requires external compensation. Values must be derived for the chosen inductor, effective output capacitance, load range and crossover target. The final design must demonstrate acceptable phase/gain margin or equivalent transient stability through simulation and bench testing.

**Do not populate compensation values based solely on a different TI reference design's 9 V / 3 A conditions.** Use TI WEBENCH or a documented calculation with the actual 5 V / 1 A design parameters; then confirm by load-step testing.

## 10. MODE / EN / thermal connections

- MODE: floating for PFM (recommended starting choice for this handheld battery rail).
- EN: hardware-defined off state; use controller enable only after the ESP32 is initialized. Add a pull-down so converter is not accidentally enabled during reset.
- NC pins 11/12: connect to ground plane as datasheet thermal recommendation.
- AGND/PGND: follow TI layout guidance; preserve quiet feedback reference and minimize high-current switching loop.
- Thermal: do not rely on IC thermal shutdown as normal regulation. Measure IC, inductor and PCB temperatures at low battery, full display brightness and radio activity.

## 11. Battery-side current budget (estimates)

At 3.0 V battery:
- Display board nominal: 2.25 W / 85% = 2.65 W input ≈ 0.88 A.
- Boost at 5 V / 1 A, 75% efficiency: 6.67 W input ≈ 2.22 A.
- Radio assumed 3.3 V × 200 mA = 0.66 W, 90% efficiency → 0.73 W battery power ≈ 0.24 A.
- Both rails at design loads: approximately 2.47 A battery input, before losses/other loads or startup surges.

The existing 3 A time-delay fuse is still provisional and may be close to this combined design load. Confirm fuse time-current behavior, cell continuous-current ratings, BMS, wire gauge, connector rating and startup before finalizing. Charging circuitry remains outside this design scope.

## 12. Layout requirements (carry into PCB phase)

- Place input capacitors, IC VIN/PGND and inductor in a compact high-current loop.
- Keep SW copper small; no routing beneath sensitive analog feedback.
- Place output capacitors close to VOUT and PGND.
- Place feedback divider and compensation components close to FB/COMP; route FB as quiet Kelvin-like sense from output capacitor node.
- Provide solid ground plane; do not create a split beneath converter/RF region.
- Connect NC thermal pins to ground plane per datasheet.
- Add test points for BAT_PROT, 5V, EN and GND. Avoid a large exposed SW test stub.

## 13. Verification gates before schematic freeze

- [ ] Confirm exact Waveshare board variant and actual 5 V current draw, including brightness and Wi-Fi/Bluetooth activity.
- [ ] Confirm the TPS61088 RHL package footprint/pin numbering in the CAD library against datasheet.
- [ ] Confirm component datasheets, availability and DC-bias derating.
- [ ] Complete compensation design using TI WEBENCH or equivalent validated calculation.
- [ ] Confirm inductor current curve and thermal rating.
- [ ] Recalculate fuse and battery current capability for combined rails.
- [ ] Schematic ERC and independent review.
- [ ] Prototype measurements: startup, shutdown, 450 mA, 1 A load, load step, 3.0 V input, ripple, efficiency and thermal soak.
- [ ] Mark the design tested only after measured results are logged.

## Sources
- Texas Instruments TPS61088 datasheet, Rev. D: https://www.ti.com/lit/ds/symlink/tps61088.pdf
- TI PMP9778 TPS61088 low-EMI reference design (use for layout guidance, not copy/paste compensation): https://www.ti.com/tool/PMP9778
- Waveshare ESP32-S3-Touch-LCD-4.3 documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3
- Coilcraft XAL50xx datasheet/product page: https://www.coilcraft.com/en-us/products/power/shielded-inductors/molded-inductor/xal/xal50xx/
