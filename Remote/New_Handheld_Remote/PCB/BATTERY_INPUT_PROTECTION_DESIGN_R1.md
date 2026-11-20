# 1S4P Remote Battery Input Protection — Design R1

**Status:** Architecture proposal only; component values and layout not finalized. Charging circuit is explicitly outside this phase.

## Pack assumptions
- 1S4P 18650 Li-ion, 4 cells in parallel.
- 4.2 V maximum charge voltage; provisional 3.0 V loaded design floor.
- Capacity depends on the selected cells; cells must be same model, age, capacity and state of charge before parallel assembly.
- Pack-level protection/BMS required for overcharge, overdischarge, overcurrent and short-circuit protection.
- Cell temperature monitoring required (NTC at pack); BMS must provide compatible temperature protection or the remote must monitor the NTC.

## Proposed power path
BATTERY+ → pack protection/BMS → main fuse close to pack positive → reverse-polarity protection → BAT_PROT → TPS61088 and TPS63070 input rails.

Pack negative → system ground. Fuse should be placed as close as practical to the pack positive lead so the wiring downstream is protected.

## Current budget
A conservative remote design-load estimate is about 2.6 A at 3.0 V pack input when the 5 V rail is loaded at 1 A and the radio rail at 300 mA, using 75% and 90% assumed efficiencies respectively. This excludes startup/inrush and other loads.

- Main fuse: 4 A time-delay candidate for review, **not yet selected**.
- Pack BMS: at least 5 A continuous candidate, subject to cell, PCB, connector and fuse coordination.
- Battery wiring and connector: select for at least 5 A continuous with low voltage drop and adequate thermal margin; 18 AWG is a starting point for short internal pack leads, confirm with the actual wire/connector datasheet.
- Never raise the fuse rating without ensuring every protected conductor, connector and component can safely carry the fuse's let-through current.

## Reverse polarity protection
- High-side P-channel MOSFET reverse-protection topology is preferred for low drop at this 1S voltage.
- Select a MOSFET with RDS(on) specified at VGS = −2.5 V or lower (appropriate for 3–4.2 V pack), adequate current, voltage and thermal rating.
- Include gate/source protection if required by selected MOSFET ratings; define gate resistor and pull-down/up behavior.
- Final MOSFET, gate circuit and orientation must be verified against a schematic and reverse-polarity test before capture.

## Battery monitoring / low-voltage behavior
- ADC divider and filter are required for pack-voltage telemetry.
- Divider must be switched or high impedance to avoid unnecessary idle battery drain.
- Software should warn at a calibrated threshold and initiate safe shutdown before the BMS deep-discharge cutoff. Exact threshold depends on cell specification, pack sag and measured ADC calibration; 3.0 V is only a provisional loaded design floor.
- BMS is a last-resort protection, not normal firmware shutdown.
- NTC monitoring and sensor fault behavior (open/short) to be designed.

## Protection gates
- [ ] Confirm exact cell model, capacity, discharge rating and datasheet.
- [ ] Select 1S BMS with protection thresholds, current and thermal specs documented.
- [ ] Select fuse and holder/package; review time-current and interrupt rating for Li-ion pack fault current.
- [ ] Select connector and wire gauge.
- [ ] Select reverse-protection MOSFET and calculate conduction loss at 3 A.
- [ ] Define battery ADC divider, filtering, calibration and low-voltage cutoff behavior.
- [ ] Define NTC placement, threshold and fault handling.
- [ ] Schematic capture, ERC and independent review.
- [ ] Test correct polarity, reverse polarity, overload, short-circuit protection, voltage drop and thermal rise.
