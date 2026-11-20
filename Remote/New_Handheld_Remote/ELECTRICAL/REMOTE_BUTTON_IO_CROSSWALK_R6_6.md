# Remote physical button and input crosswalk R6.6

**Layout authority:** latest approved front concept: two banks, each **2 columns × 4 rows**. There is one power key above the display. The OK key is the right bank's bottom-right key; it is electrically separate from the main 15-key scanner.

## Physical arrangement
- Left bank L1–L8: two columns, four rows; eight keys.
- Right bank: R1–R7 scanned on MCP23017 0x20; bottom-right physical key is OK/confirm on MCP23017 0x21 GPA4.
- One central Power key above the screen. No lower duplicate power key.
- Right bank top 2×2 are the four independent guarded release controls.

| Position | Physical key | Logical function | Electrical input |
|---|---|---|---|
| Left bank rows 1–4, cols 1–2 | L1–L8 | STOP, mode, return home, waypoint save/select, map/sonar full-screen, settings | MCP23017 0x20 GPA0–GPA7 |
| Right bank row 1 col 1 | R1 | Left bait hopper | MCP23017 0x20 GPB0 |
| Right bank row 1 col 2 | R2 | Right bait hopper | MCP23017 0x20 GPB1 |
| Right bank row 2 col 1 | R3 | Left hook | MCP23017 0x20 GPB2 |
| Right bank row 2 col 2 | R4 | Right hook | MCP23017 0x20 GPB3 |
| Right bank row 3 col 1 | R5 | Release status | MCP23017 0x20 GPB4 |
| Right bank row 3 col 2 | R6 | Release lock/status | MCP23017 0x20 GPB5 |
| Right bank row 4 col 1 | R7 | Back/cancel | MCP23017 0x20 GPB6 |
| Right bank row 4 col 2 | OK | Separate second-stage confirmation | MCP23017 0x21 GPA4 |
| Above display, centered | Power | Remote power | Separate power circuit; not MCU input |

## MCP23017 allocation
- 0x20 GPA0–7: L1–L8.
- 0x20 GPB0–6: R1–R7.
- 0x20 GPB7: spare, pull-up enabled.
- 0x21 GPA4: OK input, active-low with pull-up.
- 0x21 GPA1: nRF IRQ; GPA2 SC16 IRQ; GPA3 RADIO_EN candidate; GPA0 spare.
- The scanner exposes 15 key IDs (0–14). Bit15 is reserved/high in its state mask; the OK key is scanned independently from 0x21 GPA4.

## Safety gesture
Release key (R1–R4) hold ≥1200ms, release, then OK hold ≥800ms within 4000ms. Primary authenticated link and fresh boat safety/STOP telemetry are still required. The physical OK location is in the right bank but remains a separate expander input. 

## CAD status
R6 source has been corrected to this layout and invalid literal escaped-newline sequences were normalized. STL/PNG exports have not yet been regenerated from the corrected source; this source change is not fit-validated.
