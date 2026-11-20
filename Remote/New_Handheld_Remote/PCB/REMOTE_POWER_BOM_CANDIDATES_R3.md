# Remote Power BOM Candidates R3

Candidate shortlist only. Verify footprints, specs, bias, sourcing and bench performance before purchase.

| Ref | Candidate | Notes |
|---|---|---|
| J_WAVESHARE_BAT | PH2.0 battery interface to Waveshare board | Verify board revision/polarity; external pack feed |
| F_MAIN | Littelfuse 0458004.DR, 4A | 1206 fast-acting; I²t/interrupt coordination pending |
| Q_REV | onsemi FDS6575 P-MOS | SO-8; Drain input, Source BAT_PROT, Gate pulldown; pin/body diode verify |
| R_GATE | 100k | Gate→GND |
| Battery ADC | 220k/68k 1%, 100nF | Divider/filter |
| NTC | 10k B3950 candidate, 10k pullup, 100nF | Exact sensor TBD |
| U_RADIO | TPS63070 | Package suffix verify |
| L_RADIO | Coilcraft XFL4020-152MEC 1.5µH | Candidate |
| Radio CIN | 2×10µF 25V X7S 0805 +10µF 25V X5R 0603 | Bias check |
| Radio COUT | 3×22µF 16V X6S 0805 +10µF 0603 | Bias check |
| VAUX | 100nF X7R | 0402/0603 |
| R_RADIO_FB | 470k/150k 1% | Divider |
| RADIO_EN | 1k series +100k pulldown candidates | MCU-controlled after boot; reset OFF |
| J_RADIO | Keyed connector TBD | 3V3/GND/SPI/control |
| TP | BAT_PROT, RADIO_3V3, GND, ADC, NTC, EN | Probe access |

The TPS61023/inductor/capacitor external boost BOM is retained as fallback only. Estimated 3V pack current ≈1.20A continuous with assumed 90% efficiencies; measurement required.
