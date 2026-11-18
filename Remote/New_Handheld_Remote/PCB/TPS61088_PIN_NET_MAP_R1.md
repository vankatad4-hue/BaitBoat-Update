# TPS61088 — Pin-to-Net Map R1

**Scope:** First schematic sheet, 5 V boost rail.
**Status:** Pin/net mapping derived from TI TPS61088 typical application diagram and datasheet; symbol footprint, resistor/capacitor values, loop compensation and layout remain unverified. This is not a native KiCad schematic.

## Target
- Input: fused protected 1S battery system, nominal 3.6–3.7 V.
- Output: +5V_MAIN, intended for Waveshare ESP32-S3-Touch-LCD-4.3.
- Design load: verify board revision; 450 mA stated in project planning.
- TPS61088 output target 5.0 V, inside IC's 4.5–12.6 V output range.

## Pin map (verify against selected package symbol and datasheet before CAD)
| Pin | Name | Net / connection |
|---|---|---|
| 1 | VCC | VCC_BYP; C_VCC (≥1 µF ceramic) to GND |
| 2 | EN | BOOST_EN; hardware pull-down for default off |
| 3 | FSW | R_FSW to SW per datasheet |
| 4–7 | SW | SW_NODE; inductor L_BST connects BAT_PROT to SW_NODE |
| 8 | BOOT | C_BOOT 0.1 µF between BOOT and SW_NODE |
| 9 | VIN | BAT_PROT; local ceramic input capacitors to PGND |
| 10 | SS | C_SS to ground per datasheet soft-start guidance |
| 11–12 | NC | Tie to ground plane for thermal dissipation as datasheet instructs |
| 13 | MODE | Ground for forced PWM, or float for PFM; mode to be chosen/tested |
| 14–16 | VOUT | +5V_MAIN; output capacitors to ground |
| 17 | FB | FB_NODE; resistor divider from +5V_MAIN to AGND |
| 18 | COMP | Compensation network to AGND, values to be calculated/verified |
| 19 | ILIM | Current-limit programming resistor to AGND |
| 20 | AGND | Quiet ground reference; layout per TI |
| 21 | PGND | Power ground; layout per TI |

**Pin numbering caveat:** This table uses a 21-pin functional listing including multi-pin groups. Verify package pin numbering against the exact TPS61088 symbol and package variant in KiCad/library before placing wires. TPS61088 documentation/variant listings may differ in numbering presentation; do not treat this table as footprint pin mapping.

## Functional external components (values TBD)
- L_BST: input-to-SW power inductor, saturation/current rating calculated.
- C_IN: battery input ceramics + bulk; voltage rating and effective capacitance under bias checked.
- C_VCC: VCC bypass capacitor per datasheet.
- C_BOOT: 0.1 µF between BOOT and SW.
- C_OUT: 5 V output ceramics + bulk; ripple and transient sizing.
- R_FB_TOP/R_FB_BOTTOM: output feedback divider, calculate for 5.0 V.
- R_FSW: switch-frequency programming resistor.
- R_ILIM: peak switch-current limit setting.
- C_SS: soft-start.
- R_COMP/C_COMP (and additional compensation components if datasheet design requires): loop stability must be verified for chosen L/C/load.
- EN pull-down and controller enable connection.
- Optional test points on VIN, SW (use suitable probe caution), +5V, EN, GND.

## Sources
- TI datasheet: https://www.ti.com/lit/ds/symlink/tps61088.pdf
- Typical application diagram/pin functions: https://www.ti.com/product/TPS61088-Q1
