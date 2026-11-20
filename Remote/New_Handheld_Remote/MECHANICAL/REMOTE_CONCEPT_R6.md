# Remote Controller Concept R6 — Reference-Inspired Redesign

**Status:** concept/layout revision only; not mechanically validated.

## Visual direction
Inspired by `CONCEPT/Flytec_Remote_Concept_Reference.jpg`: wide landscape controller, sculpted ergonomic grips, raised central screen bezel, twin upper joystick pods with blue accents, compact physical button banks flanking the display, central power key, and two antenna exits.

## Button layout correction (user-confirmed)
**Each bank is 4 columns × 2 rows (4 across, 2 high).** This replaces the earlier 2-column × 4-row arrangement.

### Left bank (4×2)
Top row, left to right: L1 STOP; L2 MANUAL/AUTOPILOT; L3 HOME; L4 SAVE WAYPOINT.
Bottom row, left to right: L5 WAYPOINT SELECT; L6 MAP FULLSCREEN; L7 SONAR FULLSCREEN; L8 MENU/SETTINGS.

### Right bank (4×2)
Top row, left to right: R1 H-L (left hopper); R2 H-R (right hopper); R3 K-L (left hook); R4 K-R (right hook).
Bottom row, left to right: R5 RELEASE STATUS; R6 RELEASE LOCK/STATUS; R7 MAP/SONAR SPLIT; R8 BACK/CANCEL.

## Main hardware arrangement
- Central 4.3-inch Waveshare ESP32-S3-Touch-LCD-4.3 Standard display.
- Two analog joysticks above the banks, upper left and upper right.
- Eight tactile buttons on each side in a horizontal 4×2 grid.
- Separate power key above display.
- USB-C access at lower center (subject to board charging/service fit).
- Two antenna exits: left nRF24 2.4GHz, right E22 EU868; Wi-Fi/Bluetooth use board antenna.
- Ergonomic side grips; no shoulder or rear controls.

## Release safety remains unchanged
Four distinct release buttons; no combined bait/hook action. Each release requires the primary nRF link and boat safety interlocks, hold release key 1.2s, then hold OK 0.8s within 4s. Backup LoRa cannot release. STOP, timeout, loss of primary link, or unsafe telemetry cancels.

## Mechanical envelope
R5 provisional starting target: 310 × 209 × 69.5mm. Concept styling does not validate these dimensions. Recheck against actual display, joystick, switch, antenna, battery and PCB envelopes; confirm grip ergonomics, button reach, wall thickness, bosses, cable paths, printability, and clearances before CAD/fabrication.

## Preview
- [R6 front concept SVG](../CAD/REMOTE_CONCEPT_R6.svg)
- [Reference image](../CONCEPT/Flytec_Remote_Concept_Reference.jpg)
- [Button map](REMOTE_BUTTON_MAP_R6.md)

This preview is a layout illustration, not dimensioned CAD or a production drawing.
