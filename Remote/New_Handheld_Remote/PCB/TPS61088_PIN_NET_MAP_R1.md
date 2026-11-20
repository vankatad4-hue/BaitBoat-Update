# TPS61088 — Pin-to-Net Map R2

**Scope:** Remote 5 V boost converter schematic sheet.
**Status:** Pin/net mapping reviewed against TI TPS61088 datasheet Rev. D. Values are preliminary; compensation and native CAD/ERC are pending.

## Target
- Input: fused/protected 1S4P battery system, nominal 3.6–3.7 V; provisional operating range 3.0–4.2 V.
- Output: +5V_MAIN for Waveshare ESP32-S3-Touch-LCD-4.3 standard board.
- Nominal load estimate: 450 mA (verify hardware/revision).
- Design load target: 1.0 A, not yet validated.
- IC: TPS61088RHLR, 20-pin VQFN with exposed thermal pad.

## Pin/net map (datasheet package mapping; verify KiCad symbol/footprint again before CAD)
| Pin | Name | Net / connection |
|---|---|---|
| 1 | VCC | VCC_BYP; 2.2 µF ceramic to GND |
| 2 | EN | BOOST_EN; add pull-down, control from MCU/enable circuit |
| 3 | FSW | R_FSW=150 kΩ preliminary between FSW and SW |
| 4–7 | SW | SW_NODE; L_BST connects BAT_PROT to SW_NODE |
| 8 | BOOT | C_BOOT 100 nF between BOOT and SW_NODE |
| 9 | VIN | BAT_PROT; 100 nF local bypass and input capacitor bank |
| 10 | SS | C_SS=47 nF to ground |
| 11–12 | NC | Ground plane for thermal dissipation per datasheet |
| 13 | MODE | Float for PFM starting mode |
| 14–16 | VOUT | +5V_MAIN; output capacitor bank |
| 17 | FB | FB_NODE; divider Rtop=177 kΩ from +5V_MAIN, Rbottom=56 kΩ to AGND |
| 18 | COMP | Compensation network to AGND; values pending proper calculation |
| 19 | ILIM | R_ILIM=249 kΩ preliminary to AGND |
| 20 | AGND | Quiet reference ground |
| 21 | PGND | Power ground |

## Proposed external component values
- L_BST: 2.2 µH shielded; Coilcraft XAL5030-222MEC candidate. See calculations R2.
- C_IN: 2×22 µF, 10 V or higher ceramic plus 100 nF local bypass; verify effective capacitance at bias.
- C_VCC: 2.2 µF.
- C_BOOT: 100 nF between BOOT and SW.
- C_OUT: 3×22 µF, 10 V or higher ceramic; verify effective capacitance at 5 V.
- R_FB_TOP: 177 kΩ 1%; R_FB_BOTTOM: 56 kΩ 1%; calculated output about 5.008 V nominal.
- R_FSW: 150 kΩ 1%, preliminary ~1 MHz.
- R_ILIM: 249 kΩ 1%, preliminary ~4.78 A typical PFM limit.
- C_SS: 47 nF, approximately 11.3 ms nominal soft start.
- COMP network: intentionally not frozen until actual output capacitance/load and loop crossover design are verified.
- EN: defined low during reset/off; test point.
- Test points: BAT_PROT, +5V_MAIN, BOOST_EN, GND. SW probing point only if needed and with probe/layout precautions.

## Critical layout notes
- Compact input capacitor–VIN/PGND–inductor switching loop.
- Keep SW copper compact; avoid spreading it to other layers.
- Output caps close to VOUT and PGND.
- FB divider and COMP network near IC, away from SW node; sense output at output-capacitor node.
- Continuous ground plane; follow TI grounding and thermal pad recommendations.

## Sources
- TI TPS61088 datasheet Rev. D: https://www.ti.com/lit/ds/symlink/tps61088.pdf
- Calculation document: TPS61088_COMPONENT_CALCULATIONS_R2.md
