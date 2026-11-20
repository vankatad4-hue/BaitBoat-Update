# Remote Power BOM Candidates R2

Candidate shortlist only; exact footprint, datasheets, effective capacitor bias, supply and test verification pending.

| Ref | Candidate | Notes |
|---|---|---|
| U_DISPLAY | TPS61023DRLR | SOT563 5V boost |
| L_DISPLAY | Coilcraft XFL4020-102MEC 1µH | 4×4×2.1mm |
| CIN_DISPLAY | 10µF 10V+ X7R/X7S | Bias curves/package TBD |
| CIN_BYP | 100nF X7R | 0402/0603 |
| COUT_DISPLAY | 2×22µF 10V+ X7R/X7S | Effective capacitance at 5V |
| RFB_DISPLAY | 732k/100k 1% | Feedback pair |
| R_EN_DISPLAY | 100k | Pull-up candidate |
| U_RADIO | TPS63070 | Package suffix verify |
| L_RADIO | XFL4020-152MEC 1.5µH | Candidate |
| Radio CIN | 2×10µF 25V X7S 0805 +10µF 25V X5R 0603 | Bias verify |
| Radio COUT | 3×22µF 16V X6S 0805 +10µF 0603 | Bias verify |
| VAUX | 100nF | 0402/0603 |
| RFB_RADIO | 470k/150k 1% | Feedback pair |
| F_MAIN | Littelfuse 0458004.DR 4A | 1206 fast-acting; I²t/interruption review |
| Q_REV | onsemi FDS6575 P-MOS | SO-8; Drain pack side, Source BAT_PROT, Gate pulldown; verify pin/body diode |
| R_GATE | 100k | Gate→GND |
| Battery ADC | 220k/68k 1%, 100nF | ADC divider/filter |
| NTC | 10k B3950 candidate +10k pull-up +100nF | exact sensor TBD |
| Connectors | Pack/display/radio TBD | keyed and rated; freeze later |
| Testpoints | BAT_PROT, 5V, RADIO_3V3, GND, ADC, NTC, EN | Accessible pads |

Estimated 3V pack current: ≈1.20A continuous, ≈2.33A simultaneous transient based on assumed efficiencies; not measured.
