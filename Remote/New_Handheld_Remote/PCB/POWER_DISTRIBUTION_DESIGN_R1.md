# Remote Power Distribution Design R2

**Status:** Updated design basis. Component calculations remain under review; no native schematic, PCB, ERC, simulation or bench validation is claimed.

## Battery
- 1S4P Li-ion pack, nominal 3.6–3.7 V, maximum 4.2 V.
- 3.0 V loaded design point is provisional; pack BMS/cutoff, cell model, continuous current, cell matching, fuse and wire ratings must be verified.
- Charging circuit excluded from this current design scope.

## Rail A — 5 V display/controller
- Converter: TPS61088RHLR synchronous boost.
- Target: 5.0 V.
- Waveshare ESP32-S3-Touch-LCD-4.3 standard board: vendor's stated planning consumption 450 mA; verify board revision and actual load.
- Nominal board load ≈2.25 W.
- Design target: 1.0 A (5 W), subject to thermal and electrical validation.
- Preliminary components: 2.2 µH shielded inductor (Coilcraft XAL5030-222MEC candidate); Rtop 177 kΩ, Rbottom 56 kΩ; RFSW 150 kΩ; RILIM 249 kΩ; CSS 47 nF; CBOOT 100 nF; CVCC 2.2 µF; input 2×22 µF + 100 nF; output 3×22 µF. Capacitor bias derating and availability to be checked.
- PFM light-load mode is preferred starting choice.
- Loop compensation is deliberately not specified until calculation/WEBENCH for actual 5 V / 1 A design and bench verification.
- Full calculations in TPS61088_COMPONENT_CALCULATIONS_R2.md.

## Rail B — 3.3 V radio
- Converter: TPS63070 adjustable buck-boost.
- Target 3.3 V, divider R1=470 kΩ VOUT→FB and R2=150 kΩ FB→GND (nominal 3.307 V calculation).
- Inductor 1.5 µH Coilcraft XFL4020-152ME candidate.
- Input capacitors: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output capacitors: 3×22 µF 16 V X6S 0805 + 10 µF 0603; nominal 76 µF (verify effective capacitance).
- VAUX 100 nF.
- Radio burst-load budget 200 mA assumption; actual PA/LNA module current to be measured.
- Local radio module decoupling 100 nF + 10–47 µF.
- TPS63070 pins/configuration must still be verified against datasheet and exact package.

## Battery current estimates (not measured)
At 3.0 V battery:
- Display at 5 V × 450 mA = 2.25 W; assumed 85% boost efficiency → 2.65 W battery input, about 0.88 A.
- Boost at 5 V × 1 A = 5 W; assumed 75% efficiency → 6.67 W battery input, about 2.22 A.
- Radio at 3.3 V × 200 mA = 0.66 W; assumed 90% buck-boost efficiency → 0.73 W battery input, about 0.24 A.
- Combined design-load estimate ≈2.47 A from the 3 V pack, before other loads/startup effects. Nominal board load plus radio estimate ≈1.13 A.

The 3 A time-delay fuse is provisional and may be marginal at the combined design load with startup transients. Re-evaluate after cell, BMS, connector, wire and fuse time-current specifications are confirmed. Do not automatically increase fuse rating without checking the wiring/pack protection.

## Protection and PCB layout
- Main fuse: provisional 3 A time-delay, not finalized.
- Reverse polarity: MOSFET ideal-diode approach; exact circuit TBD.
- Shared ground plane, no split beneath converter/RF area.
- Compact converter switching loops; keep SW nodes small.
- Sensitive analog, ADC and radio signals separated from switching nodes by placement/routing.
- Keep RF antenna/module area free of switching noise and copper/metal obstruction as required by module antenna design.
- Test points for BAT_PROT, +5V_MAIN, RADIO_3V3, enables, battery ADC and GND.
- Temperature target below 70°C at selected hotspots is a project test goal, not a component limit.

## Validation gates
- Confirm pack/cells, BMS and current capability.
- Measure display current at startup, maximum brightness, Wi-Fi/Bluetooth and peripheral use.
- Verify radio burst current.
- Finish TPS61088 compensation calculations and TPS63070 pin/configuration review.
- Finalize fuse, reverse protection and connectors.
- Native KiCad capture, exact footprints, ERC, independent review, PCB layout.
- Bench test both rails for startup, load steps, ripple, efficiency and thermal behavior.
