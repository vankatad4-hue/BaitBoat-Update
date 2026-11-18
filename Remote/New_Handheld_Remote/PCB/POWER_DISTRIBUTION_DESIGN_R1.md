# Remote Power Distribution Design R1

**Status:** Design basis; component validation and PCB implementation pending.

## Battery
1S4P protected Li-ion pack, nominal 3.6–3.7 V, 4.2 V maximum; 3.0 V loaded design point provisional. Pack cells, BMS, cutoff and temperature protection must be verified.

## Rail A — 5 V display/controller
- Controller: Waveshare ESP32-S3-Touch-LCD-4.3.
- Converter candidate: TPS61088 synchronous boost, 5.0 V output.
- Display load planning estimate: 450 mA; exact board revision and worst-case brightness/peripheral current to be verified.
- 5 V target 2 A continuous design capacity is a target only; peak 3 A is not approved without design/thermal validation.
- TPS61088 component values, current limit, compensation, inductor and thermal design remain under calculation.

## Rail B — 3.3 V radio
- Converter: TPS63070 adjustable buck-boost.
- Target 3.3 V, calculated divider nominal 3.307 V.
- R1 470 kΩ VOUT→FB, R2 150 kΩ FB→GND, 1% candidates.
- L_RADIO 1.5 µH Coilcraft XFL4020-152ME candidate.
- Input caps: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output caps: 3×22 µF 16 V X6S 0805 + 10 µF 0603; 76 µF nominal.
- VAUX: 100 nF.
- Config: FB2 grounded; VSEL static LOW; PS/SYNC HIGH power-save pending datasheet pin bias confirmation; EN active via 10 kΩ pull-up to BAT_PROT; optional PG pull-up 100 kΩ to 3V3_RADIO.
- Radio output load budget: 200 mA transient assumption; module-specific burst current TBD.
- Local module decoupling: 100 nF + 10–47 µF.

## Battery current estimates
At 3.0 V battery:
- Display: 5 V × 0.45 A = 2.25 W; assumed 85% boost efficiency → 2.647 W battery power, 882 mA battery current.
- Radio: 3.307 V × 0.2 A = 0.661 W; assumed 90% efficiency → 0.735 W battery power, 245 mA battery current.
- Combined estimated current: 1.13 A.
At 4.2 V battery with same assumptions, combined estimate is approximately 0.81 A.
Estimates exclude other loads and startup/inrush; efficiencies and display current are assumptions. Measure under operating conditions.

## Protection, layout, thermal
- Main fuse: 3 A time-delay is provisional only; coordinate with pack BMS, wiring, connectors, measured inrush and fault current.
- Reverse polarity: MOSFET ideal-diode approach; exact circuit not designed.
- Common ground, continuous ground plane; separate high-current switching paths from sensitive RF/signal paths by placement and routing, not by cutting the ground plane beneath RF.
- Tight converter switching loops; place local capacitors close to pins, minimize SW copper, keep FB quiet.
- Thermal test enclosed remote at low battery, full display brightness, radio bursts and Wi-Fi/Bluetooth active.
- Temperature target below 70°C on selected hotspots is a project test goal, not a component rating.

## Validation
No native KiCad schematic, ERC, layout or thermal bench test is claimed. Exact pack, display load, converter component values and protection require measurement/verification.
