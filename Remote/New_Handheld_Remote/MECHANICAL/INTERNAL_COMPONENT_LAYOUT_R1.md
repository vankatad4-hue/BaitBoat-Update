# New Handheld Remote — Internal Component Placement Mockup R1

**Status:** Spatial study only. No actual component footprints or fit have been verified.

## Mockup zones
- Combined display + ESP32-S3 controller board: centered behind the display aperture; placeholder envelope 130 × 80 × 12 mm. The concept assumes an integrated display/controller board to avoid an unnecessary separate main-board stack. The exact board with external-antenna support is not selected.
- nRF24L01+ PA/LNA module: left/upper RF zone, close to Antenna A; placeholder envelope 32 × 22 × 12 mm.
- USB-C charging/power-management board: lower-right service zone, near USB-C port; placeholder envelope 55 × 35 × 10 mm.
- Input/cable strips: along the left and right side zones; illustrative placeholders only.
- Battery: four 18650 cells, shown as four parallel rows across the X axis in the lower-central zone (cell axes run along Y). Cell model is 65 × 18 mm envelope; pack restraint, wiring, protection, spacing and clearances are not modeled.
- Antenna connector stubs: rear upper corners, to visualize RF cable routing.

## Important conflicts to resolve
1. The integrated display/controller board must be checked against joystick bodies, front-panel depth, rear-shell depth, antenna cables and its connector/cable bend radius.
2. The joysticks extend into the rear cavity and may collide with the controller board or RF modules.
3. The nominal 62 mm shell depth is a target only. The depth stack, cable bends and cell restraint may require changes.
4. The battery zone is illustrative. Cells require mechanical retention, insulation, protection, fuse/temperature considerations and safe charging design; this mockup is not a battery holder or pack design.
5. Antenna separation and RF coexistence must be assessed for both 2.4 GHz systems. The diagram does not demonstrate RF performance.

## CAD usage
Open CAD/internal_layout_mockup.scad in OpenSCAD. This file is a separate visualization of internal placement zones; it does not replace the front/rear shell CAD or define fabrication geometry.

## CAD review note
The first visual render showed the four cell volumes overlapping. The mockup was revised to show four parallel cells with their 65 mm axes along Y and 18 mm spacing across X. This is only a visual packing concept; retain the USB/service-port and rear-depth clearance checks for the next iteration.

## Placement simplification
The mockup now treats the touchscreen and ESP32-S3 as one integrated display/controller board, rather than showing two overlapping boards. This is a design assumption to be confirmed when selecting the exact display module; the ESP32-S3 module must support the planned external antenna path.


## Mechanical refinement R2 — expanded battery reserve (2026-11-18)
- Provisional shell depth allowance increased from 62 to 70 mm to provide additional internal volume.
- Reserved lower-central battery volume enlarged to 104 × 82 × 24 mm (placeholder envelope).
- Controller/display envelope moved upward in the space study to improve separation from the battery reserve.
- Four 18650 placeholders remain illustrative; pack retention, electrical protection, charger, wiring, and clearances are not designed or validated.
- OpenSCAD internal mockup R2 was rendered to STL successfully; PNG render attempt failed due to headless OpenGL/X-server limitation. A separate schematic space preview is available for review.
- This revision is a layout study, not verified assembly or print-ready CAD.
