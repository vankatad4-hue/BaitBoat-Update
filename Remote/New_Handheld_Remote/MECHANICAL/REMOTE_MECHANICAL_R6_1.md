# Remote Mechanical CAD R6.1

## R6.1 changes
- Added sculpted rear palm/grip pads as a joined hull; pads are ergonomic placeholders, not validated grip contours.
- Added four 18650 cell envelopes: two per grip, stacked in shell depth. Provisional cell diameter 18.6mm, length 65mm. Pack layout assumes a 1S4P arrangement, but cells, welds, insulation, pack holder, restraint and electrical protection are not designed.
- Added a cradle base, end saddles, vertical support columns and lateral rails integrated into rear shell CAD.
- Added carrier PCB mounting posts for a 100×75mm placeholder board and internal preview envelopes for the LCD, carrier, sticks, and dual radio modules.
- Added `part="internal"`, `part="battery_cradle"` and corresponding preview/export modes.

## Prototype envelope
- Outer footprint: 310×209mm.
- Main rear shell: 66.5mm deep; front panel 3.5mm.
- Palm pad geometry protrudes about 14.8mm rearward from shell datum; full depth with the front-panel guard collars is approximately 87mm. Confirm this in CAD before printing.
- Front screen opening 108×72mm, provisional.
- Joystick aperture 25mm; placeholder ring OD34mm / inner opening 26mm.
- Button spacing 19.5mm horizontal and 21mm vertical. Release apertures 18mm with preliminary 21mm collar OD.
- Main carrier envelope 100×75mm with 8mm standoffs.
- Four 18650 placeholders: two in each side grip, stacked in depth. Cell envelope Ø18.6×65mm; assumed pack arrangement only.

## Export and mesh checks (local OpenSCAD)
- Front panel STL: watertight, one connected mesh.
- Rear shell STL: watertight, one connected mesh.
- Button cap plate: watertight, 17 separate parts.
- The rear shell now includes grip pads, battery cradle and carrier mounting standoffs.
- Mesh validity means the files export as closed meshes. It does not confirm fit, strength, tolerances, ergonomic comfort, print orientation, battery safety, or switch actuation.

## Fit and safety gates before full print
1. Measure actual Waveshare board, display bezel, PCB and cable connectors.
2. Measure joystick module, switch bodies, button caps, antenna bulkheads and battery cells.
3. Check cavity and grip interference; especially battery clearance from switch/joystick wiring and enclosure walls.
4. Design a secure cell restraint, insulation, pack fuse, BMS, wiring, charging and thermal management; do not use this cradle for live cells until reviewed.
5. Verify carrier mounting hole pattern and standoff positions.
6. Print small test coupons for button, collar, joystick, USB and screw fit.
7. Test ergonomics with a low-cost mock-up before printing the complete enclosure.

R6.1 is a prototype layout and not production-ready.
