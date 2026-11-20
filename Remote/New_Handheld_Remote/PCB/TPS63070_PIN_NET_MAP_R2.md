# TPS63070 — Pin/Net Map R2

**Status:** Reviewed against TPS63070 Rev. B datasheet; exact CAD symbol/footprint, ERC and bench tests pending.

## Fixed rail
- Output: RADIO_3V3, nominal calculated 3.3067 V.
- Feedback: R1=470 kΩ from VOUT to FB; R2=150 kΩ from FB to GND.
- VSEL LOW, no dynamic voltage scaling.
- FB2 GND for unused voltage-selection path; confirm symbol/capture.
- PS/SYNC HIGH selects power-save PWM/PFM mode; implement via suitable pull-up/control network per datasheet.
- EN defined; proposed 10 kΩ pull-up to BAT_PROT for always-on rail.
- PG optional open-drain monitor with pull-up to RADIO_3V3.

## Pin map
| Pin | Name | Net / connection |
|---|---|---|
| 1 | PS/SYNC | PS_SYNC_HIGH; power-save mode; verify resistor/control |
| 2 | PG | RADIO_PGOOD; optional open-drain status |
| 3 | VAUX | VAUX_BYP; 100 nF to GND; no external load |
| 4 | GND | Signal/control ground |
| 5 | FB | RADIO_FB; R1/R2 divider junction |
| 6 | FB2 | GND; unused voltage-scaling path |
| 7–8 | VOUT | RADIO_3V3; output capacitor bank |
| 9 | L2 | SW_L2; other side of L_RADIO |
| 10 | PGND | Power ground |
| 11 | L1 | SW_L1; one side of L_RADIO |
| 12–13 | VIN | BAT_PROT; input capacitor bank |
| 14 | EN | RADIO_EN; defined active level, never floating |
| 15 | VSEL | GND; static LOW |

## Component baseline
| Ref | Value | Status |
|---|---|---|
| U_RADIO | TPS63070RNM/RNMT | Package/suffix to verify |
| L_RADIO | Coilcraft XFL4020-152MEC, 1.5 µH | Candidate |
| R1 | 470 kΩ, 1% VOUT→FB | TI typical 3.3 V value |
| R2 | 150 kΩ, 1% FB→GND | TI typical 3.3 V value |
| CIN1–2 | 10 µF, 25 V X7S 0805 | TI application baseline |
| CIN3 | 10 µF, 25 V X5R 0603 | TI application baseline |
| COUT1–3 | 22 µF, 16 V X6S 0805 | TI application baseline |
| COUT4 | 10 µF, 0603 | TI application baseline |
| CVAUX | 100 nF X7R | Required bypass |
| R_EN | 10 kΩ pull-up candidate | Verify tied-pin implementation |
| R_PG | 100 kΩ pull-up candidate | Optional |

Nominal output capacitance is 76 µF. Effective capacitance must be checked at DC bias and temperature.

## Radio connector
Proposed signals: RADIO_3V3, GND, SPI_SCK, SPI_MOSI, SPI_MISO, CSN, CE, IRQ. Pin order and connector part remain TBD. Place 100 nF + 10–47 µF local decoupling at module connector.

## Layout/verification
- Compact input/switch/output current loops.
- Place input/output capacitor closest to IC, and inductor adjacent to switching pins.
- Quiet feedback divider and control-ground routing.
- Verify package pin mapping and exposed pad.
- Verify VSEL/FB2, PS/SYNC, EN and PG implementation.
- Confirm effective capacitance, radio burst current, ripple and thermal behavior.
- No schematic/PCB/bench validation claimed.
