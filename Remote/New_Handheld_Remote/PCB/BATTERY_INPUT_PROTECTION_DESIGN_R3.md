# 1S4P Remote Battery Input Protection — Design R3

**Status:** Candidate components and nets identified; schematic/ERC/PCB/bench validation pending. Charging remains out of scope.

## Pack and current assumptions
1S4P matched Li-ion, 4.2 V max; 3.0 V loaded budget point provisional. Pack BMS provides overcharge/overdischarge/overcurrent/short protection. NTC required and open/short detection. Verify actual cell model, pack BMS and ratings.

## Current budget estimates
- 5V/450mA = 2.25W; assumed 90% efficiency at 3V gives ~0.83A pack current.
- 5V/1A transient = 5W; assumed 85% at 3V gives ~1.96A.
- Radio 3.3V/300mA = 0.99W; assumed 90% at 3V gives ~0.37A.
- Combined continuous ~1.20A; simultaneous transient ~2.33A. Not measured; excludes startup/other loads.
4A fuse candidate, BMS ≥5A candidate, conductors/connectors ≥5A design target are not frozen.

## Proposed component candidates
### Main fuse
Littelfuse 0458004.DR, Nano² 458, 4A, 1206, fast-acting, 75V DC rating, 50A interrupt rating (vendor listing). Candidate only; verify datasheet derating, I²t, pulse handling, PCB pad/thermal behavior and pack fault-current coordination. Do not call this a slow/time-delay fuse.

### Reverse-polarity MOSFET
ON Semiconductor FDS6575, P-channel, −20V, SO-8, 10A class; RDS(on) 17mΩ max at VGS=−2.5V (datasheet). High-side reverse-protection candidate.
At 2.33A estimated transient: P≈I²R=0.092W using 17mΩ; at 1.20A continuous ≈0.024W. These are simplified conduction estimates at stated RDS and do not include temperature rise or dynamic conditions.
Topology: Source from fused pack positive; Drain to BAT_PROT; Gate pulled to system GND via 100kΩ candidate. Confirm source/drain pin mapping from selected package datasheet, body-diode orientation and reverse-polarity behavior in schematic. Pack maximum 4.2V keeps VGS within ±8V rating for this candidate under intended topology; still verify fault/transient conditions.
SO-8 occupies more area but reduces conduction loss versus small SOT-23 parts. Package footprint and pin layout must be confirmed.

## Battery sense
Proposed divider: R_BAT_TOP=220kΩ BAT_PROT→ADC_BAT; R_BAT_BOTTOM=68kΩ ADC_BAT→GND; C_BAT=100nF ADC_BAT→GND.
At 4.2V, ADC ≈0.977V; at 3.0V, ≈0.698V. Divider current at 4.2V ≈14.6µA. ADC pin, attenuation, calibration and acquisition behavior must be checked against ESP32-S3 board design. Add firmware calibration and cell-sag-aware warning/shutdown thresholds.

## Pack temperature
NTC candidate: 10kΩ at 25°C, B≈3950K. Suggested divider: 10k pull-up from controller 3.3V to NTC sense node, NTC from node to GND, 100nF at sense node. Confirm sensor exact B value, wiring, thresholds and open/short detection. Mount NTC near pack center/thermal hotspot as approved by cell/pack supplier.

## Proposed protected power path
PACK+ → pack/BMS output → F_MAIN (0458004.DR candidate) → Q_REV source → Q_REV drain → BAT_PROT.
Q_REV gate → 100kΩ to GND.
PACK− → GND.
BAT_PROT feeds TPS61023 and TPS63070.
J_BAT includes PACK+, PACK− and NTC (connector part/pinout TBD). Fuse near pack positive.

## Review/validation gates
- [ ] Confirm cell model, BMS board and its current/temperature limits.
- [ ] Confirm fuse derating/I²t/interrupt and source fault-current compatibility.
- [ ] Confirm FDS6575 package pins/footprint and gate/body-diode topology.
- [ ] ADC/NTC pin mapping, divider accuracy and calibration.
- [ ] Schematic ERC and independent review.
- [ ] Controlled tests for correct/reversed polarity, voltage drop, load, fuse/BMS protection, MOSFET/fuse temperature and NTC faults.
No protection design is validated until these tests are logged.

## References
FDS6575 datasheet: https://www.onsemi.com/download/data-sheet/pdf/fds6575-d.pdf
Littelfuse 458 series: https://www.littelfuse.com/~/media/electronics/datasheets/fuses/littelfuse_fuse_451_453_datasheet.pdf.pdf
