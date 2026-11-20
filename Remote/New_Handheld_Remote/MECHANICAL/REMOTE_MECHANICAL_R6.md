# Remote Mechanical R6 — Initial CAD Layout

## User-confirmed panel layout
- Left and right physical button banks: **4 columns × 2 rows** each.
- Left bank functions: STOP, manual/autopilot, return-home, save waypoint, waypoint select, map full-screen, sonar full-screen, menu/settings.
- Right bank top row: H-L (left hopper), H-R (right hopper), K-L (left hook), K-R (right hook).
- Right bank bottom row: release status, release lock/status, map/sonar split, back/cancel.
- Joysticks upper left/right, central 4.3-inch screen, separate top-center power, lower-center USB-C, dual antennas at upper rear.

## R6 CAD source
`CAD/Flytec_Remote_Enclosure_R6.scad` supports `part="front"`, `part="rear"`, `part="buttons"`, `part="assembly"`.

Draft parameters:
- Body footprint: 310 × 209mm (outer perimeter scaled to target).
- Body depth: 66.5mm rear tray + 3.5mm front plate; release guard collars add 2mm locally (about 72mm total there).
- Screen cutout: 108 × 72mm; center offset Y=7mm.
- Joystick cutout: 25mm; center X=±108, Y=66mm.
- Button apertures: 14.2mm; 19.5mm column pitch, 21mm row pitch.
- Bank centers: ±108mm; two rows around Y=-31mm.
- Power aperture: 18mm at Y=96mm.
- USB opening: 15.5 × 8.5mm at bottom edge.
- Rear antenna pass-throughs: 8.6mm, X=±112mm, Y=82mm.
- Provisional screw locations, internal support ledges and joystick rings are modeled.

## Critical engineering limitations
The enclosure source is a **layout and fit draft**, not production-ready or component-verified CAD. The screen aperture may not match the Waveshare bezel/PCB; no Waveshare board, joysticks, tactile switches, battery or antenna bulkhead have been measured. The present button cap arrangement is a visual/print placeholder, and switch mounting/retention and guard clearances are not verified; the guard rings are only preliminary geometry and do not replace electronic safety interlocks. Internal volume/PCB stack-up, battery safety, screw bosses, assembly seam, grip ergonomics, wall thickness, clearances, waterproofing, and print orientation remain to be verified.

## Export
CAD workflow will export R6 front panel, rear shell, button-cap plate and assembly preview. Check the GitHub Actions build before treating meshes as valid exports. Print test pieces before a full enclosure.

## Local CAD checks performed

The source was rendered locally with OpenSCAD. Front panel, rear tray, and button-cap plate exports completed without syntax errors. STL meshes were inspected: front panel and rear shell are watertight single meshes; button plate is watertight with 17 separate caps. The shell footprint exports to 310×209mm; rear tray depth is 66.5mm. This checks CAD generation/mesh integrity only—not fit, switch actuation, ergonomics, strength, or printing tolerances.
