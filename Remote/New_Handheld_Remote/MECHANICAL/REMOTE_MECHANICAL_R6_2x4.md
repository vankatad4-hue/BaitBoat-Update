# Remote Mechanical Revision R6.3 — Front Power + integrated OK

## Locked user-facing layout
- **Two button banks, each 2 columns × 4 rows** (8 per side, 16 total).
- Left bank: navigation/menu/utility functions.
- Right bank: upper 2×2 guarded bait/hook release keys; lower three auxiliary keys and bottom-right OK/Confirm key.
- Two joysticks, central 4.3-inch touchscreen, **one dedicated front Power button centered above the display**, and OK/Confirm integrated into the bottom-right position of the right 2×4 bank.
- CAD coordinates: Power center `(0, +55 mm)`; OK center `(115.75 mm, -71.5 mm)`.
- Two antenna exits, rear grip concept, USB-C service/charge opening on lower bridge.

## CAD source
- [R6.3 CAD source](../CAD/Flytec_Remote_Enclosure_R6_3.scad)
- [R6.3 front layout preview](../CONCEPT/RENDER_R6_3/Remote_Front_R6_3.svg)
- [Preliminary internal component placement plan](./REMOTE_INTERNAL_LAYOUT_R6_3.md)
- [R6.2 source (superseded)](../CAD/Flytec_Remote_Enclosure_R6_2x4.scad)
- R6.3 CAD provides `front`, `rear`, and `assembly` modes. Front and rear STL exports were generated successfully; an OpenSCAD preview is available. This is CAD parsing/export verification only, not dimensional fit or manufacturing validation.

## Preview and package
- R6.3 preview files are stored under `CONCEPT/RENDER_R6_3/`.
- STL export succeeded. Mesh closure, component fit, print strength and assembly alignment have not been validated.

## Still provisional
Display cutout, joystick, button switch and release hardware dimensions, battery restraint and pack safety, antenna connector clearances, USB position, internal board dimensions, wiring, heat management and mounting holes need verification against purchased components. Do not use the current CAD as final manufacturing release.
