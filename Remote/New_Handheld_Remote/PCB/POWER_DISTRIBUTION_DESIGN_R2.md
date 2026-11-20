# Remote Power Distribution Design R2

**Status:** Current design basis. Calculations are preliminary and must be validated; no native schematic, ERC, PCB, simulation or bench validation is claimed.

## Battery
- 1S4P Li-ion pack, nominal 3.6–3.7 V, maximum 4.2 V.
- 3.0 V loaded design point is provisional.
- Pack protection/BMS, cells, current capability, fuse, wiring and connector remain to be selected.
- Charging circuit excluded from current scope.

## Rail A — 5 V display/controller
- Converter: TPS61088RHLR synchronous boost.
- Output target 5.0 V.
- Waveshare ESP32-S3-Touch-LCD-4.3 standard board vendor load planning figure 450 mA; verify revision and actual current.
- Nominal 2.25 W load; 1.0 A / 5 W rail target subject to thermal and electrical validation.
- Preliminary components: L=2.2 µH shielded (Coilcraft XAL5030-222MEC candidate); feedback 177 kΩ/56 kΩ; FSW 150 kΩ; ILIM 249 kΩ; SS 47 nF; BOOT 100 nF; VCC 2.2 µF; input 2×22 µF + 100 nF; output 3×22 µF 16 V preferred.
- PFM starting mode.
- Loop compensation intentionally not frozen.
- Detailed calculations: TPS61088_COMPONENT_CALCULATIONS_R2.md.

## Rail B — 3.3 V radio
- Converter: TPS63070 adjustable buck-boost.
- Output calculated 3.3067 V from 470 kΩ/150 kΩ feedback divider.
- 1.5 µH Coilcraft XFL4020-152MEC candidate.
- Input 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output 3×22 µF 16 V X6S 0805 + 10 µF 0603; 76 µF nominal.
- VAUX 100 nF.
- Radio load assumption 200 mA, test margin up to 300 mA.
- Configuration reviewed against TI datasheet; exact CAD and bench verification pending.
- Detailed calculations: TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md.

## Protection
- Pack-level BMS required.
- Main fuse: 4 A time-delay candidate, not selected.
- High-side P-MOS reverse-polarity protection proposed.
- Battery ADC sensing and pack NTC required.
- Design: BATTERY_INPUT_PROTECTION_DESIGN_R1.md.

## Battery current budget (estimated)
At 3.0 V:
- Display 450 mA at 5 V = 2.25 W; assumed 85% efficiency gives 0.88 A battery current.
- Boost full design load 5 W; assumed 75% efficiency gives 2.22 A.
- Radio 200 mA at 3.3 V = 0.66 W; assumed 90% efficiency gives 0.24 A.
- Radio 300 mA design margin: ~0.37 A input.
- Combined maximum rail-load estimate: approximately 2.59 A at 3.0 V (5 V rail 1 A + radio 300 mA), excluding startup, other loads and efficiency deviations.

## Layout
- Common ground plane.
- Compact switching loops and small SW copper.
- Local input/output capacitors.
- Quiet feedback and control-ground routes.
- RF region and antenna keepout respected.
- Test points for BAT_PROT, +5V_MAIN, RADIO_3V3, enables, ADC and GND.
- Thermal testing in enclosure at low pack voltage and maximum system load.

## Freeze conditions
- Verify display load and radio burst current.
- Finish TPS61088 compensation.
- Check effective capacitance under DC bias.
- Finalize cell/BMS/fuse/reverse-protection, wiring and connectors.
- Capture native schematic, ERC, review, PCB and prototype measurements.
