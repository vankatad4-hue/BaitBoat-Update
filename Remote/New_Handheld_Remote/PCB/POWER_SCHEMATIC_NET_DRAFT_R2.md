# Remote Power — Electrical Net Draft R2

**Status:** Net-level schematic specification; this is not a native KiCad schematic and has not passed ERC or bench validation.

## A. Battery input / protection

J_BAT:
- PACK+
- PACK−

Power path:
PACK+ → BMS-protected pack positive → F_MAIN (4 A time-delay candidate, not selected) → Q_REV (high-side P-MOS reverse-polarity protection, component/circuit TBD) → BAT_PROT.
PACK− → GND.

Include:
- NTC input from battery pack.
- BAT_PROT/GND test points.
- Battery voltage sense divider to MCU ADC; switched divider or sufficiently high resistance to reduce idle drain.
- Hardware and firmware default states for all enables.
- Charging circuit is excluded from this sheet.

## B. 5 V display/controller boost

U_BOOST = TPS61088RHLR, input BAT_PROT, output +5V_MAIN.

Preliminary component values from TPS61088_COMPONENT_CALCULATIONS_R2.md:
- L_BST = 2.2 µH shielded (Coilcraft XAL5030-222MEC candidate).
- R_FB_TOP = 177 kΩ, 1% (+5V_MAIN→FB).
- R_FB_BOTTOM = 56 kΩ, 1% (FB→AGND).
- R_FSW = 150 kΩ, 1% (FSW→SW), approximately 1 MHz.
- R_ILIM = 249 kΩ, 1% (ILIM→AGND), approximately 4.78 A typical PFM current limit.
- C_SS = 47 nF to GND.
- C_BOOT = 100 nF BOOT→SW.
- C_VCC = 2.2 µF VCC→GND.
- C_IN = 2×22 µF, 10 V+ ceramics plus 100 nF local bypass.
- C_OUT = 3×22 µF, 16 V preferred ceramics (effective capacitance to be checked).
- MODE floating for PFM initial choice.
- EN default low with pull-down; enabled by power-control/MCU logic.
- NC pins 11/12 to ground plane per datasheet.
- COMP network intentionally TBD until calculation/WEBENCH for this 5 V/1 A design.

Output: +5V_MAIN powers Waveshare ESP32-S3-Touch-LCD-4.3. Use a dedicated connector, short/wide 5 V and GND traces, and test points.

## C. 3.3 V radio buck-boost

U_RADIO = TPS63070, input BAT_PROT, output RADIO_3V3.

- L_RADIO = 1.5 µH between L1 and L2.
- R1 = 470 kΩ VOUT→FB.
- R2 = 150 kΩ FB→GND.
- Calculated nominal output = 3.3067 V.
- Input caps: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output caps: 3×22 µF 16 V X6S 0805 + 10 µF 0603; 76 µF nominal; bias/temperature effective capacitance check required.
- VAUX: 100 nF to GND.
- VSEL LOW; FB2 GND for unused selection path.
- PS/SYNC HIGH power-save mode; use correct resistor/control network.
- EN defined (10 kΩ pull-up candidate for always-on rail, subject to datasheet implementation review).
- PG optional open-drain status with 100 kΩ pull-up to RADIO_3V3.
- Local nRF module decoupling at connector: 100 nF + 10–47 µF.

## D. Controller sense / enable

- ADC_BAT: pack voltage divider/filter, calibration, low-voltage warning and shutdown behavior.
- NTC_BAT: temperature sensing; define open/short fault detection.
- BOOST_EN and RADIO_EN: default OFF during reset unless hardware design intentionally enables the radio rail.
- RADIO_PGOOD: optional MCU input with compatible pull-up.
- Test points: BAT_PROT, +5V_MAIN, RADIO_3V3, EN signals, ADC_BAT, GND.

## E. Connectors — not frozen

- J_BAT: pack positive/negative plus optional NTC.
- J_DISPLAY: +5V_MAIN, GND, required control/interface signals only if external.
- J_RADIO: RADIO_3V3, GND, SPI SCK/MOSI/MISO, CSN, CE, IRQ.
- J_DEBUG/CONTROL: programming and GPIO as needed.
- Connector part numbers, keying, pin order, wire gauges and footprints must be selected and reviewed.

## F. Power and protection envelope

At 3.0 V pack, estimated battery input:
- 5 V rail 1 A, assumed 75% efficiency: 2.22 A.
- Radio 3.3 V rail 300 mA, assumed 90% efficiency: 0.37 A.
- Combined ≈2.59 A, excluding startup and miscellaneous loads.

4 A fuse and ≥5 A BMS are candidates only. Confirm actual cells, BMS thresholds/current, fuse interrupt/time-current ratings, wiring, connectors, reverse-protection loss, and thermal conditions before design freeze.

## G. Capture / validation gates

- [ ] Complete TPS61088 compensation design.
- [ ] Verify exact IC symbols and footprints against datasheets.
- [ ] Verify all resistor/capacitor effective values and tolerances.
- [ ] Finalize BMS, fuse, MOSFET reverse-protection and pack sense.
- [ ] Freeze connectors/pinouts.
- [ ] Native KiCad capture and ERC.
- [ ] Independent schematic review.
- [ ] Prototype bench tests before claiming validated.
