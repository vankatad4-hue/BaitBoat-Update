# TPS63070 Radio 3.3 V Supply — Pin/Net Map R1

**Status:** Datasheet-checked functional pin map; values and PCB layout not yet design-verified.  
**Purpose:** Dedicated buck-boost rail for the nRF24L01+ PA/LNA module from the remote's 1S4P battery.  
**Source:** Texas Instruments TPS63070 datasheet, Rev. B, March 2019: https://www.ti.com/lit/ds/symlink/tps63070.pdf

## Design target
- Input: protected 1S4P battery bus (approximately 3.0–4.2 V during normal use; actual pack limits must be checked).
- Output: 3.3 V nominal for radio.
- Converter: TPS63070 adjustable buck-boost.
- Radio rail transient/load budget: design for at least 200 mA provisionally; verify actual PA/LNA module current and RF bursts before final sizing.
- TI specifies 2.0–16 V input, 2.5–9 V output, and up to 2 A output in buck or boost mode under specified conditions. This does **not** guarantee 2 A on our PCB.
- Package: 15-pin VQFN-HR, 3 × 2.5 mm nominal body. Confirm footprint/pad numbering against selected CAD library and datasheet package drawing.

## Pin-to-net map (datasheet pin numbers)

| Pin | Name | Net / connection | Notes |
|---:|---|---|---|
| 1 | PS/SYNC | RADIO_PFM_SYNC | Pull low for forced PWM, or high for PWM/PFM power-save operation. External sync only if intentionally designed. Default mode decision TBD. |
| 2 | PG | RADIO_PGOOD_N | Open-drain power-good output; pull-up and MCU monitoring optional. Do not assume it can drive a load directly. |
| 3 | VAUX | VAUX_CAP | 100 nF capacitor to GND close to pin; do not load externally. |
| 4 | GND | LOGIC_GND | Control/logic ground. |
| 5 | FB | RADIO_FB | Adjustable output feedback node; resistor divider design must be calculated from datasheet equations and selected variant. |
| 6 | FB2 | RADIO_FB2 | Feedback voltage-scaling output; leave open or ground if unused as specified. Do not treat as a second feedback input. |
| 7, 8 | VOUT | RADIO_3V3 | Output pins joined to 3.3 V output and capacitor bank. |
| 9 | L2 | SW_L2 | Inductor switching node; route compactly. |
| 10 | PGND | POWER_GND | Power ground; high-current return. |
| 11 | L1 | SW_L1 | Inductor switching node; route compactly. |
| 12, 13 | VIN | BAT_PROT | Input supply pins; local input capacitance. |
| 14 | EN | RADIO_REG_EN | Enable. Define default-on/off and MCU control; add bias so state is deterministic during reset. |
| 15 | VSEL | RADIO_VSEL | Voltage scaling control. Tie low for the fixed divider configuration unless dynamic scaling is intentionally implemented and verified. |

## Functional connections
- L1 (pin 11) and L2 (pin 9) connect to opposite ends of the buck-boost inductor.
- VIN pins 12/13 connect to protected battery rail with local input capacitor(s).
- VOUT pins 7/8 connect to 3.3 V output with output capacitor bank.
- FB, FB2, VSEL network must follow the adjustable-output datasheet circuit; values are not assigned in this revision.
- VAUX pin 3 gets its required 100 nF capacitor to ground.
- PS/SYNC pin 1 mode is a design choice; default power-save behavior is under review.
- PG pin 2 is optional open-drain status output; if used, pull it up to a logic rail and route to a controller input.
- EN pin 14 must have a defined state at startup. A pull resistor and/or MCU control are to be selected.
- GND pin 4 and PGND pin 10 join into the board ground system with layout that respects high-current return paths and datasheet guidance.

## Datasheet reference component guidance (not yet adopted as final BOM)
The TI simplified circuit shows a 1.5 µH inductor, input capacitor(s), output capacitor(s), a feedback resistor network, 100 nF at VAUX, and EN biasing. These are reference values/topology only. The final design must use the exact datasheet design procedure, effective capacitance after DC-bias derating, inductor saturation/current rating, switching-loop layout, thermal calculation and selected output configuration.

## Layout constraints
- Place inductor, IC, input capacitors and output capacitors close together; keep switching loops small.
- Keep SW_L1/SW_L2 copper compact; do not route RF, ADC or touch/control lines through switching-node area.
- Keep the radio 3.3 V output and ground return low impedance; local decoupling at the radio connector.
- Grounding and exposed-pad/thermal implementation must follow the package and datasheet layout recommendations.
- Do not copy the TPS61088 5 V converter layout; the two power stages need separate optimized layouts.

## Bring-up checks (radio disconnected first)
1. Inspect shorts/continuity and footprint/pin orientation.
2. Power with a current-limited bench supply; start below intended load.
3. Verify output voltage, startup, shutdown, ripple and load step with dummy load.
4. Verify stability and temperatures across battery input range.
5. Test RF module bursts and measure 3.3 V droop/ripple with appropriate probing.
6. Only then connect the nRF24 PA/LNA module and perform RF coexistence tests.

## Verification gates
- Exact TPS63070 variant/package symbol and footprint pin mapping: verify before schematic capture.
- Output-feedback calculation and resistor values: TBD.
- Inductor and capacitor selection: TBD, calculate using TI design procedure and bias derating.
- EN default and MCU control: TBD.
- PS/SYNC mode: TBD.
- PG pull-up/controller pin: TBD.
- Radio peak current and module variant: TBD.
- Layout review, thermal, ripple and load tests: not performed.

This file is a pin/net planning artifact, not a native KiCad schematic, reviewed PCB, ERC result or fabrication-ready design.
