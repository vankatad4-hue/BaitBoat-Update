# TPS61023 — Pin/Net Map R1
**Status:** Draft net mapping; schematic/ERC/PCB/bench pending. Verify pin numbering from TI package drawing.
U_DISPLAY TPS61023DRLR, SOT563:
1 VIN=BAT_PROT, CIN→GND.
2 GND=ground plane.
3 EN=BOOST_5V_EN, default ON from BAT_PROT through 100k pull-up candidate; verify datasheet.
4 SW=SW_DISPLAY; L from BAT_PROT to SW.
5 VOUT=+5V_MAIN, COUT→GND.
6 FB=DISPLAY_FB divider midpoint.
Parts: L_DISPLAY 1µH; CIN 10µF X7R/X7S 10V+ plus 100nF local; COUT 2×22µF X7R/X7S 10V+; R_FB_TOP 732kΩ 1% +5V→FB; R_FB_BOTTOM 100kΩ 1% FB→GND; R_EN 100kΩ pull-up candidate.
Layout: compact input/IC/inductor loop, SW small, COUT close, FB away from switching node, short low-impedance ground. Testpoints BAT_PROT, +5V_MAIN, GND. J_DISPLAY_POWER +5V_MAIN/GND; connector and Waveshare power method TBD.