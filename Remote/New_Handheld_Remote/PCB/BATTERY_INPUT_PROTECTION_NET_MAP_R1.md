# Battery Input and Protection — Net/Pin Map R1

**Status:** Architecture draft; component selection and ratings are not final.  
**Pack:** 1S4P lithium-ion, four parallel 18650 cells. Nominal pack voltage 3.6/3.7 V, full-charge voltage depends on the selected cells and pack protection (commonly 4.2 V). Verify exact cell specification and pack assembly. Charging circuit is outside this document's scope.

## Functional path
BATTERY_PACK → PACK_PROTECTION/BMS → J_BAT input → F_MAIN (3 A time-delay provisional) → reverse-polarity protection → protected distribution node BAT_PROT → branches:
- U1 TPS61088 boost input for 5 V display rail.
- U2 TPS63070 input for 3.3 V radio rail.
- Battery sensing divider/filter to MCU ADC.
- Pack thermistor input if provided by the pack.

The actual pack may already have BMS/protection; confirm its output leads, rating, cutoff behavior and thermistor before connecting. This sheet does not replace pack-level protection.

## Input connector and net plan

| Ref / interface | Pin | Net | Notes |
|---|---:|---|---|
| J_BAT | 1 | BAT_PACK_POS | Protected pack positive output; connector polarity must be keyed/marked. |
| J_BAT | 2 | BAT_PACK_NEG | Pack negative; common board return. |
| F_MAIN | 1 | BAT_PACK_POS | Fuse input. |
| F_MAIN | 2 | BAT_FUSED | 3 A time-delay placeholder only; final value from measured load, wiring, connector, pack and fuse curves. |
| Q_REV | input | BAT_FUSED | Reverse-polarity MOSFET protection stage; topology and part TBD. |
| Q_REV | output | BAT_PROT | Protected positive distribution. |
| C_IN bulk | + | BAT_PROT | Bulk capacitor near distribution point; voltage rating and capacitance TBD. |
| C_IN bulk | − | GND | Short return path. |
| TP_BAT | — | BAT_PACK_POS | Pack voltage measurement before fuse/protection (only if safe and useful). |
| TP_FUSED | — | BAT_FUSED | Fused input test point. |
| TP_PROT | — | BAT_PROT | Protected distribution test point. |
| TP_GND | — | GND | Ground test point. |

## Branch interface nets
- BAT_PROT and GND feed the TPS61088 5 V boost stage.
- BAT_PROT and GND feed the TPS63070 radio buck-boost stage.
- Battery sense: divider from BAT_PROT to ADC_BAT through R_TOP, R_BOTTOM; filter capacitor from ADC_BAT to GND. Values and ADC range/attenuation TBD. Confirm divider never exceeds ESP32-S3 ADC pin limits and account for ADC nonlinearity/calibration.
- Optional switched divider enable: use a GPIO-controlled switch if low standby drain is important. Default state must be defined; GPIO leakage and switch off-state must be considered.
- PACK_NTC: optional two-wire thermistor input if present on pack. Confirm polarity/type/value; do not assume 10 kΩ without checking. Use a suitable pull-up/reference and ADC protection.
- GND is the common return. Place high-current converter return paths close to the input bulk return; avoid routing radio/sense ground paths through high-current switching loops.

## Protection and component selection gates
- Pack BMS/protection: verify current rating, overcurrent/short-circuit cutoff, undervoltage/overvoltage, thermal behavior and whether it protects all four parallel cells as assembled.
- Main fuse: 3 A time-delay is provisional, not approved. Check actual peak current, startup surge, pack capability, wire and connector rating, fuse curve/I²t, and downstream converter limits. Do not increase fuse rating to prevent nuisance blowing without review.
- Reverse polarity: select and validate a MOSFET ideal-diode arrangement or equivalent. Check VGS limits, gate network, body-diode orientation, worst-case voltage drop and thermal dissipation.
- Connector: keyed, locking, current/voltage-rated connector selected only after pack, peak current and wire gauge are known.
- Bulk/ceramic capacitance: choose for converter input ripple/transient and effective capacitance after DC-bias derating; no final values assigned.
- PCB copper/trace widths: calculate for peak input current and acceptable temperature rise. The input current may be much higher than output current when converting from a 1S pack.

## Bring-up sequence
1. Verify pack polarity and protection behavior independently.
2. Check board for shorts from BAT_PROT to GND and both branches disconnected.
3. Test the reverse-protection stage using a current-limited supply; confirm forward drop and reverse blocking.
4. Validate fuse choice with load profile and current measurements.
5. Measure BAT_PROT sag and converter startup with display and radio branches separately.
6. Verify ADC divider calibration against a multimeter at low, nominal and full pack voltages.
7. Test pack thermistor interface only after its exact characteristic is known.

## Not finalized
- Cell model and pack assembly method.
- Pack BMS/protection model and current rating.
- J_BAT connector, wire gauge and strain relief.
- Fuse type/value and holder/footprint.
- Reverse-polarity MOSFET(s) and topology.
- Input bulk/ceramic capacitor values.
- Battery divider values, ADC attenuation and filtering.
- NTC type, connector and pull-up.
- PCB copper widths, thermal and layout.
- Measured peak/startup current.

This is a net-level design draft, not a native KiCad schematic, reviewed PCB, battery-pack design or fabrication-ready output.
