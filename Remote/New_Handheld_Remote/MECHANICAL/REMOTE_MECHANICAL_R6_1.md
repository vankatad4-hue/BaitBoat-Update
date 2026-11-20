# Remote Mechanical CAD R6.1

## R6.1 changes
- Added sculpted rear palm/grip pads as joined hull geometry; ergonomic shape is unvalidated.
- Added four 18650 envelopes: two per grip, stacked in shell depth. Provisional cells Ø18.6 × 65 mm; 1S4P is an assumed pack layout, not an engineered battery.
- Rear shell integrates cradle floor, end saddles, support columns and retaining rails.
- Carrier-board mounting posts for a 100 × 75 mm placeholder board, plus preview envelopes for LCD, sticks and dual-radio modules.
- Added CAD preview modes for internal layout and battery cradle.
- Extended the lower-center bridge and moved the USB-C cutout to that area. Connector alignment remains a placeholder pending actual board dimensions.

## Approximate envelope
- Outer footprint: 310 × 209 mm.
- Main shell: 66.5 mm depth; front panel 3.5 mm.
- Palm pads extend about 14.8 mm behind the shell datum; overall depth with front panel/release collar stack approximately 87 mm.
- Screen opening: 108 × 72 mm (provisional).
- Joystick aperture: 25 mm; ring OD 34 / ID 26 mm.
- Button banks revised to **2 columns × 4 rows per side** (16 total); 19.5 mm column pitch and 20.5 mm row pitch. Four guarded release keys occupy the top 2×2 of the right bank; right-bank lower 4 keys are auxiliary. Release apertures 18 mm with 21 mm collars.
- Carrier envelope 100 × 75 mm with 8 mm posts.
- Battery envelope: 4 cells Ø18.6 × 65 mm, two in each grip, stacked through shell depth.

## Local mesh checks
- Front panel: watertight, one connected component.
- Rear shell: watertight, one connected component.
- Button plate: watertight, 17 separate caps.
- Cradle feature STL: watertight mesh, 2 components; cradle is integrated into rear shell for the main print.

Mesh checks validate closure only—not component fit, strength, ergonomics, printability, tolerances, thermal management or battery safety.

## Required before full printing
1. Measure Waveshare board, bezel, connectors and cable exits.
2. Measure joysticks, switch bodies/caps, antenna bulkheads, cells and carrier PCB holes.
3. Check clearances to front controls, wiring and enclosure walls.
4. Engineer cell restraint, insulation, fuse/BMS, charging, wiring and thermal protection before using live cells.
5. Verify carrier mounting pattern and screw locations.
6. Print fit coupons for screen, sticks, buttons/collars, USB-C and screws.
7. Test a mock-up for grip comfort and button reach.

R6.1 is a prototype layout, not production-ready CAD.
