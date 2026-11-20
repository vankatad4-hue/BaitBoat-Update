# Remote Power BOM Candidate List R1

**Status:** Engineering shortlist only. Not purchase-ready; verify exact footprints, capacitor bias, stock/alternates and datasheets before ordering.

| Ref | Function | Candidate / value | Package | Notes |
|---|---|---|---|---|
| U_DISPLAY | 5V boost | TPS61023DRLR | SOT563 | Selected candidate; validate thermal |
| L_DISPLAY | Boost inductor | Coilcraft XFL4020-102MEC, 1µH | 4×4×2.1mm | Candidate, peak current calculation pending |
| CIN_DISPLAY | Boost input | 10µF, 10V+, X7R/X7S | 0805/1206 depending derating | Check effective C at battery voltage |
| CIN_BYP | Input bypass | 100nF, X7R | 0402/0603 | Local |
| COUT_DISPLAY | Boost output | 2×22µF, 10V+, X7R/X7S | 1206 or larger as needed | Check effective C at 5V |
| RFB1 | Boost feedback | 732kΩ, 1% | 0603 | VOUT→FB |
| RFB2 | Boost feedback | 100kΩ, 1% | 0603 | FB→GND |
| R_EN_DISPLAY | Enable default | 100kΩ | 0603 | Pull-up candidate; datasheet check |
| U_RADIO | Radio buck-boost | TPS63070 | Package per exact ordering suffix | Verify symbol/footprint |
| L_RADIO | Radio inductor | Coilcraft XFL4020-152MEC, 1.5µH | 4×4×2.1mm | Candidate |
| CIN_RADIO | Radio input | 2×10µF, 25V X7S | 0805 | Effective C check |
| CIN_RADIO_LOCAL | Radio input | 10µF, 25V X5R | 0603 | Local |
| COUT_RADIO | Radio output | 3×22µF, 16V X6S | 0805 | Bias check |
| COUT_RADIO_LOCAL | Radio output | 10µF | 0603 | Local bulk |
| C_VAUX | TPS63070 bypass | 100nF X7R | 0402/0603 | Close to VAUX |
| R_RADIO_FB1 | Feedback | 470kΩ, 1% | 0603 | VOUT→FB |
| R_RADIO_FB2 | Feedback | 150kΩ, 1% | 0603 | FB→GND |
| F_MAIN | Pack fuse | Littelfuse 0458004.DR, 4A | 1206 | Fast-acting candidate; verify interrupt/I²t and thermal |
| Q_REV | Reverse protection | onsemi FDS6575 P-MOSFET | SO-8 | 17mΩ max @ −2.5V; pin map/body diode check |
| R_GATE | MOS gate default | 100kΩ | 0603 | Gate→system GND |
| R_BAT_TOP | Battery ADC | 220kΩ, 1% | 0603 | BAT_PROT→ADC |
| R_BAT_BOTTOM | Battery ADC | 68kΩ, 1% | 0603 | ADC→GND |
| C_BAT_ADC | ADC filter | 100nF | 0402/0603 | ADC→GND |
| R_NTC_PULLUP | Thermistor divider | 10kΩ, 1% | 0603 | 3V3 controller rail→NTC sense |
| TH_BAT | Pack temperature | 10kΩ NTC, B≈3950K | wired/selected footprint | Candidate; exact part TBD |
| C_NTC | NTC filter | 100nF | 0402/0603 | NTC sense→GND |
| J_BAT | Pack connector | TBD, keyed/locking | TBD | ≥5A design target, verify |
| J_DISPLAY | Display power | TBD | TBD | +5V/GND, verify Waveshare input |
| J_RADIO | nRF module | TBD keyed header | TBD | Power, SPI, CE/CSN/IRQ |
| TP | Test points | BAT_PROT, 5V, 3V3, GND, ADC, NTC, enables | SMT loop/test pads | Include probe clearance |

## Estimated power
At 3.0V loaded pack, assumed efficiencies:
- 5V/450mA continuous, 90%: ≈0.83A battery input.
- Radio 3.3V/300mA, 90%: ≈0.37A.
- Combined continuous ≈1.20A.
- Display 5V/1A transient, 85% plus radio: ≈2.33A.
These are estimates only. Fuse/BMS/wiring coordination must be completed against actual load measurements.

## Purchase gates
- Confirm cell and pack BMS model.
- Confirm TPS61023 and TPS63070 exact package/footprints.
- Select capacitors using manufacturer DC-bias curves.
- Confirm fuse interrupt/I²t suitability and reverse MOSFET thermal/pinout.
- Freeze connector models, pinouts and wire.
- Verify supplier availability and approved alternates.
