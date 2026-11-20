# Handheld Remote Mechanical Envelope R2

**Target outer envelope: 300 × 210 × 70 mm (W × H × D)**, subject to fit-check CAD and physical mockup. This modestly enlarges the R1 envelope to provide clearance for button banks, joystick throw, grips, four 18650 cells, RF module placement, coax bends and service access.

## Front layout
- 4.3-inch touchscreen centrally placed; actual board/display revision measured before CAD. Approximate active board envelope 107 × 68 mm.
- Two joystick modules at upper left and upper right, symmetric; at least 20mm free cap travel in every direction, 30mm preferred from neighbouring surfaces.
- 16 buttons: two 2×4 banks flanking screen; cap 12–16mm and pitch ≥18mm initial.
- Confirm/OK button and emergency stop placed separate from release buttons; safety-critical release controls require guarded/long-press UI, but actual bait/hook release remains physical buttons with unique action ACK.
- No shoulder or rear buttons; rear grips only.
- Antenna ports along upper rear edge: LEFT nRF24 2.4GHz, RIGHT LoRa 868MHz.

## Internal stack
- Four matched 18650 cells 1S4P in lower rear cradle; exact cell size and holder envelope required.
- Pack protection board/fuse near positive terminal, accessible.
- Waveshare controller/display mounted central; FPC bend and service access protected.
- Carrier PCB located below/behind controller only if 3D clearance, thermal and RF checks pass; otherwise beside display, enlarging shell.
- nRF PA/LNA near LEFT coax; LoRa module near RIGHT coax. Keep both antennas away from cells and switching converter.
- USB-C service/charge access from lower edge; gasketed port, no IP rating claim.

## Fit-check steps
1. Download exact 3D models for board, joystick, switches, cells, connectors, radio modules.
2. Confirm clearances and stick throw, display viewing angle, button accessibility and hand grip.
3. Build printed shell prototype; test for comfortable reach, stability, daylight readability and weight.
4. Adjust dimensions before PCB/shell freeze. R2 envelope is not a finished CAD model.
