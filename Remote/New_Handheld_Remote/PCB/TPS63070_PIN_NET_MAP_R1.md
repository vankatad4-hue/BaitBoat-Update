# TPS63070 — Pin/Net Map R1

**Status:** Functional pin/net draft. Pin numbers and footprint must be verified against exact TPS63070 package/library symbol. Not a native KiCad schematic.

## Fixed-rail configuration
- Output: 3.3 V radio supply (calculated nominal 3.307 V).
- FB2: GND for single-output adjustable operation.
- VSEL: LOW/static selection; no dynamic voltage switching planned.
- PS/SYNC: HIGH for power-save mode, subject to datasheet pin-state and bias verification.
- EN: always enabled via proposed 10 kΩ pull-up to BAT_PROT; defined state, never floating.
- PG: optional open-drain status, proposed 100 kΩ pull-up to 3V3_RADIO.

## Functional net mapping
| Pin/function | Net/connection | Notes |
|---|---|---|
| VIN | BAT_PROT | Input ceramics placed close to IC |
| L1 | SW_L1 | One end of 1.5 µH inductor |
| L2 | SW_L2 | Other end of inductor |
| VOUT | RADIO_3V3 | Output capacitor bank and radio connector |
| FB | RADIO_FB | Junction of R1/R2 |
| FB2 | GND | Single-output configuration |
| VSEL | VSEL_LOW | Static LOW; verify datasheet |
| PS/SYNC | PS_SYNC_HIGH | Power-save; verify proper connection |
| EN | RADIO_EN | Defined high state; proposed 10 kΩ BAT_PROT pull-up |
| VAUX | VAUX_BYP | 100 nF X7R to GND |
| PG | RADIO_PGOOD | Optional; open-drain pull-up 100 kΩ to RADIO_3V3 |
| PGND/GND | GND | Follow datasheet ground/exposed-pad guidance |

## Components
| Ref | Value / part | Status |
|---|---|---|
| U_RADIO | TPS63070RNM/RNMT suffix/package to verify | Candidate |
| L_RADIO | Coilcraft XFL4020-152ME, 1.5 µH | Candidate |
| R1 | 470 kΩ, 1% VOUT→FB | Calculated |
| R2 | 150 kΩ, 1% FB→GND | Calculated |
| CIN1–2 | 10 µF, 25 V X7S 0805 | TI typical application |
| CIN3 | 10 µF, 25 V X5R 0603 | TI typical application |
| COUT1–3 | 22 µF, 16 V X6S 0805; Murata GRM21BC81C226ME44L candidate | TI typical application |
| COUT4 | 10 µF 0603 | TI typical application |
| CVAUX | 100 nF X7R | Datasheet bypass |
| R_EN | 10 kΩ pull-up | Proposed; verify control behavior |
| R_PG | 100 kΩ pull-up | Optional |

Output nominal capacitance is 76 µF. Earlier 98 µF nominal proposal (4×22 µF + 10 µF) is superseded. Check DC-bias effective capacitance and temperature before BOM release.

## Radio connector J_RADIO
Proposed signals: RADIO_3V3, GND, SPI_SCK, SPI_MOSI, SPI_MISO, CSN, CE, IRQ. Pin order and exact connector remain TBD. Add 100 nF and 10–47 µF local decoupling at module.

## Validation gates
- Verify exact symbol pin numbers/package/footprint.
- Check FB divider topology, VSEL and FB2 configuration against TI adjustable application.
- Verify PS/SYNC bias and EN default.
- Verify PG voltage compatibility.
- Check capacitor effective values at DC bias and temperature.
- Review SW-node loop, input/output capacitor placement, feedback routing and ground/exposed pad.
- Native schematic, ERC, PCB and bench validation remain pending.
