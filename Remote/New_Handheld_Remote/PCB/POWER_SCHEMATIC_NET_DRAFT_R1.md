# Power PCB — Electrical Schematic Net Draft R1

**Status:** Net-level schematic draft, not a finished KiCad schematic. Pin-specific implementation, compensation and component values require datasheet/reference design review and bench validation.

## Design constraints
- Pack: protected 1S4P 18650 (four cells in parallel); battery chemistry voltage range must be confirmed from pack documentation.
- Charging circuit excluded; battery pack is assumed to have protection. Confirm protection rating and cutoffs.
- Main display/controller: Waveshare ESP32-S3-Touch-LCD-4.3 powered from 5 V.
- Radio: nRF24L01+ PA/LNA module supplied from dedicated 3.3 V rail.
- Two independent switching rails from fused battery: 5 V boost and 3.3 V buck-boost.
- Ground is common; power return routing must keep converter/radio current loops from sensitive signal returns.

## Sheet A — BATTERY_INPUT
**Proposed nets**
- J1.1 = PACK_POS
- J1.2 = PACK_NEG / GND
- F1 in series: PACK_POS → BAT_FUSED (3 A time-delay provisional; verify)
- Q1 reverse-polarity protection (MOSFET ideal-diode topology, exact circuit TBD): BAT_FUSED → BAT_PROT
- C_IN bulk + ceramics: BAT_PROT to GND, near converter input branches
- TP_BAT = PACK_POS
- TP_FUSED = BAT_FUSED
- TP_VSYS = BAT_PROT

Do not treat pack BMS as a substitute for F1. Fuse and wiring must be coordinated with pack protection, connector and wire ratings. The fuse value remains provisional.

## Sheet B — BOOST_5V (TPS61088)
**Power flow**
BAT_PROT → boost inductor L_BST → TPS61088 SW pins; converter output VOUT pins → +5V_MAIN.
The precise topology/pin connectivity must follow TI's datasheet typical application circuit.

**Functional connections**
- VIN pins → BAT_PROT with local ceramic input capacitors.
- VCC pin → local required bypass capacitor to GND.
- BOOT → bootstrap capacitor to SW per datasheet.
- EN → BOOST_EN; provide defined default state (pull-down/firmware enable, not floating).
- FB → divider from +5V_MAIN to AGND; divider values calculated for TPS61088 reference.
- COMP → compensation network to AGND; values derived for chosen L/C/load.
- ILIM → resistor to AGND; set peak limit after inductor and load-current calculations.
- SS → soft-start capacitor to AGND.
- MODE → defined configuration for PFM or forced-PWM; mode chosen after EMI/efficiency tests.
- FSW → programming resistor to SW per datasheet.
- AGND and PGND → ground plane with layout/return guidance from TI.
- NC pins → datasheet-required treatment; note TPS61088 datasheet indicates NC pins 11/12 are tied to ground plane for thermal dissipation.
- +5V_MAIN connector J2: 5 V and GND; add output capacitors near connector/load.

**Output**
- Target: 5.0 V.
- Load: Waveshare display/controller.
- Initial power-budget baseline: board documentation lists approx. 450 mA at 5 V (verify board revision and actual load).
- Converter target capacity is not a promise; select parts/layout after load and thermal validation.

## Sheet C — RADIO_3V3 (TPS63070)
**Power flow**
BAT_PROT → TPS63070 buck-boost VIN → 3V3_RADIO.
Inductor between L1 and L2; exact pin/inductor routing from datasheet.

**Functional connections**
- VIN pins → BAT_PROT, with local input capacitors.
- L1/L2 → inductor (initial datasheet reference value 1.5 µH; confirm final design with current, saturation, DCR, layout and manufacturer recommendations).
- VOUT pins → 3V3_RADIO with output capacitor bank.
- FB network → program 3.3 V on adjustable version; exact resistor values calculated from datasheet equation and tolerance.
- EN → RADIO_EN, defined default-off pull-down.
- VSEL → ground for single output configuration, following datasheet typical application.
- PS/SYNC → defined level for power-save/PWM mode; choose after current/noise tests.
- VAUX → required 100 nF bypass.
- PG → RADIO_PGOOD, pulled up to logic rail only if needed; don't connect to MCU until signal voltage is checked.
- PGND and GND → common ground, layout per datasheet.
- nRF connector J3: 3V3_RADIO, GND, SPI_SCK, SPI_MOSI, SPI_MISO, CSN, CE, IRQ. Signal pin order TBD; add keyed connector.

**Output**
- Target: 3.3 V.
- Radio transient supply budget: at least 200 mA provisionally; verify actual module.
- Near module connector: 100 nF ceramic + 10–47 µF low-ESR bulk capacitor (confirm module vendor recommendations).

## Sheet D — SENSE_CONTROL
- VBAT_SENSE: divider from BAT_PROT to ADC_BAT; values chosen to keep ADC below ESP32 ADC maximum under battery/overshoot conditions. Include RC filter; consider switched divider to reduce idle drain.
- PACK_NTC: connector for pack thermistor, with ADC bias/filter; firmware must handle open/short as fault.
- BOOST_EN and RADIO_EN: enable control inputs with hardware-defined default states.
- RADIO_PGOOD: optional status signal; level and pull-up voltage verified before connection to MCU.
- Test points: GND, BAT_PROT, +5V_MAIN, 3V3_RADIO, BOOST_EN, RADIO_EN, RADIO_PGOOD, ADC_BAT.

## Sheet E — CONNECTORS
- J1 battery pack: polarity/keying and current rating TBD.
- J2 Waveshare 5 V input: 5 V/GND; connector part/pinout must match board revision.
- J3 radio: power + SPI/control, keyed connector.
- J4 pack NTC: NTC/GND.
- J5 control interface: enable/status/ADC lines to MCU, pinout TBD.
- Include labels and pin-1 marks in PCB silkscreen; connector pin numbers and wire colors are not frozen.

## Power budget — first pass
- Waveshare nominal documented load: approx. 5 V × 0.45 A = 2.25 W.
- At VBAT=3.0 V and 85% boost efficiency, display load alone is approximately 0.88 A from battery (calculation: 2.25 W / (3.0 V × 0.85)).
- Radio and all other loads add to this; input current peaks and low-battery operation must be measured.
- This is why the main fuse and battery/connector/wire ratings cannot be finalized from the display load alone.

## Parts not yet value-locked
- TPS61088 inductor and current rating.
- TPS61088 input/output capacitors, feedback divider, compensation, ILIM, FSW, MODE and soft-start values.
- TPS63070 inductor and current rating; input/output capacitor set; FB resistor values.
- Input bulk capacitance and any EMI filtering.
- Reverse polarity MOSFET and gate network.
- Fuse part, holder and final rating.
- Connector part numbers/pin assignments.
- NTC, ADC divider and filter.
- Layout and thermal copper/via design.

## References
- TPS61088 datasheet: https://www.ti.com/lit/ds/symlink/tps61088.pdf
- TPS63070 datasheet: https://www.ti.com/lit/ds/symlink/tps63070.pdf
