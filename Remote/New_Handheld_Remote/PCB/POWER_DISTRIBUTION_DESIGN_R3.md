# Remote Power Distribution R3
**Status:** TPS61023 selected candidate. No native schematic/ERC/PCB/bench validation claimed.
Battery: protected 1S4P, 4.2V max; 3.0V loaded provisional. BMS, fuse, reverse protection, wires/connectors, NTC TBD. Charger excluded.
## 5V rail
TPS61023DRLR boost, SOT563. TI reference: 1µH, 10µF input, 2×22µF output, 732k/100k FB divider, enabled. Supplemental 100nF CIN. Verify pin mapping/EN. Waveshare ESP32-S3-Touch-LCD-4.3 Standard; planning 450mA continuous, 1A transient only. Small-package thermal validation required; internal compensation.
## Radio 3.3V
TPS63070; 1.5µH XFL4020-152MEC candidate; 470k/150k feedback; nominal 3.3067V. CIN 2×10µF 25V X7S 0805 +10µF 25V X5R 0603; COUT 3×22µF 16V X6S 0805+10µF 0603 (nominal 76µF); VAUX 100nF. Verify configuration and effective capacitance. Radio 200mA nominal assumption, 300mA test.
## Estimated pack current @3.0V
5V/450mA at 90%≈0.83A; radio 3.3V/300mA at 90%≈0.37A; continuous≈1.20A. 5V 1A transient at 85% plus radio≈2.33A. Estimates exclude startup/other loads. 4A fuse and ≥5A BMS provisional.
Layout: common ground, compact switching loops, minimal SW copper, local caps, quiet FB, RF section away from switchers, trace sizing/testpoints, thermal testing in enclosure at low pack/max load.
Freeze: confirm load, pin/FB/EN, capacitance/inductor, protection/connectors; KiCad capture, ERC, review, PCB and bench.