# TPS63070 — Radio 3.3 V Pin/Net Map R3

**Status:** Revised enable strategy; schematic/ERC/PCB/bench validation pending.

RADIO_3V3 nominal 3.3067 V, TPS63070 from BAT_PROT. Feedback 470k VOUT→FB, 150k FB→GND. VSEL LOW; FB2 GND (verify application pin treatment). PS/SYNC per TI power-save configuration. EN controlled by ESP32 GPIO after main controller starts: 1k series candidate and 100k pulldown to GND so rail defaults OFF at reset; verify EN thresholds/rating. GPIO TBD. PG optional open-drain status. VAUX 100nF to GND, no external load.

| Pin | Name | Net |
|---|---|---|
| 1 | PS/SYNC | PS_SYNC_CONFIG |
| 2 | PG | RADIO_PGOOD optional |
| 3 | VAUX | VAUX_BYP 100nF→GND |
| 4 | GND | GND |
| 5 | FB | RADIO_FB |
| 6 | FB2 | GND, verify |
| 7–8 | VOUT | RADIO_3V3 |
| 9 | L2 | SW_RADIO_L2 |
| 10 | PGND | GND |
| 11 | L1 | SW_RADIO_L1 |
| 12–13 | VIN | BAT_PROT |
| 14 | EN | RADIO_EN |
| 15 | VSEL | GND |

L_RADIO 1.5µH Coilcraft XFL4020-152MEC candidate. CIN 2×10µF 25V X7S 0805 +10µF 25V X5R 0603. COUT 3×22µF 16V X6S 0805 +10µF 0603 (76µF nominal; bias-effective TBD). Radio local 100nF +10–47µF. Verify footprint/pins, startup, bursts, droop, ripple and thermal.
