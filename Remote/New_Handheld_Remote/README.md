# New Handheld Remote Hardware Development

This folder is the design workspace for the new game-controller-style bait-boat remote, separate from the existing Waveshare remote firmware.

## Current concept baseline
- 4.3-inch central touchscreen.
- Power button centered above display.
- USB-C service/charging port on the lower edge (location is provisional).
- Two joysticks above button banks.
- 16 buttons: two symmetrical banks, each 2 columns × 4 rows.
- No shoulder buttons or rear buttons.
- Two rear antenna exits: nRF24L01+ PA/LNA radio and ESP32-S3 external-antenna Wi-Fi/Bluetooth radio.
- 1S4P Li-ion pack: 4 × matched 2500 mAh cells, nominal 10,000 mAh.
- Ergonomic grips.

## Mechanical refinement (R1)\n- Target envelope: 310 × 190 × 62 mm, still provisional.\n- Front-panel coordinate placement is documented in `MECHANICAL/COMPONENT_PLACEMENT_R1.md`.\n- Battery, controller, radio, power, and cable zones are mapped conceptually; actual parts and stack heights must be measured before locking CAD.\n- No component fit, waterproofing, or battery/charging design is approved yet.\n- Internal component mockup is available at `CAD/internal_layout_mockup.scad` and is visual-only.\n\n## Folder map
- `CAD/`: parametric OpenSCAD prototype shell/front panel with `front`, `rear`, and `assembly` selectors.
- `PCB/`: board architecture, schematic planning and connector/pinout documents.
- `MECHANICAL/`: dimensions, component fit, print and assembly notes.
- `WIRING/`: cable/connector interface tables.
- `docs/`: design decisions, power/RF and bring-up plans.

## Important status
CAD and electrical design are **preliminary templates**. Component footprints, display exact dimensions, joystick mounting, button switches, charging/protection design, RF layout, connector pinouts and safety details must be validated before ordering PCB fabrication or assembling a battery pack. The first CAD model is intended for fit/ergonomic printing, not final production.

## Suggested CAD workflow
1. Open `CAD/remote_shell_template.scad` in OpenSCAD.
2. Select `part = "front"` to export the front template, or `part = "rear"` for the rear shell.
3. Choose `part = "front"`, `"rear"`, or `"assembly"`; render (F6), export STL, and print a low-cost fit prototype. The current draft has been successfully rendered to STL with OpenSCAD for all three selectors (geometry checks passed; component fit remains unverified).
4. Measure real display, joystick, switch and button parts before finalizing openings.

Do not treat placeholder cutouts as verified component footprints.

## CAD geometry check
- Fixed the front screen opening to cut fully through the panel. Front/rear STL exports are simple meshes in the local OpenSCAD verification; fit is still unverified.


## Visual design baseline — reference image (LED-free)
- Adopt the reference's sculpted controller silhouette: angular shoulder transitions, deep central screen bezel, integrated joystick wells, and large textured outer grips/lower handles.
- Finish: matte black/dark graphite. Remove blue illuminated strips, joystick rings, illuminated button surrounds, and decorative backlighting.
- Preserve central 4.3-inch screen, two upper-corner joysticks, symmetrical 2×4 button banks, separate central power button above display, lower-edge USB-C, and two rear antennas.
- CAD shell remains simplified; reference styling requires dedicated silhouette/grip-contour revision before fit testing.


## Official visual reference
[Official concept image](../New_Handheld_Remote/CONCEPT/Flytec_Remote_Concept_Reference.jpg)

The JPEG is uploaded and verified in the CONCEPT folder. It is the styling reference; blue LED/backlighting elements are excluded from the mechanical design.


## Mechanical refinement R2 — concept preview (2026-11-18)
- Generated a multi-view concept preview to explore front, angled, rear, side, and internal layout styling against the user's uploaded reference.
- Visual target: sculpted matte-black shell, textured grips, recessed display, joystick wells, rear access panel, dual antennas, lower USB-C.
- Decorative LED rings, grip strips, illuminated button surrounds, and blue backlighting remain excluded.
- Preview is illustrative only. Dimensions and internal arrangement shown are not verified CAD or component-fit results; next CAD work must translate the contours into parametric geometry and validate physical parts.


## Mechanical refinement R2 — expanded battery reserve (2026-11-18)
- Provisional shell depth allowance increased from 62 to 70 mm to provide additional internal volume.
- Reserved lower-central battery volume enlarged to 104 × 82 × 24 mm (placeholder envelope).
- Controller/display envelope moved upward in the space study to improve separation from the battery reserve.
- Four 18650 placeholders remain illustrative; pack retention, electrical protection, charger, wiring, and clearances are not designed or validated.
- OpenSCAD internal mockup R2 was rendered to STL successfully; PNG render attempt failed due to headless OpenGL/X-server limitation. A separate schematic space preview is available for review.
- This revision is a layout study, not verified assembly or print-ready CAD.


## Actual component selection and dimension integration
See [`COMPONENT_SELECTION_DIMENSION_INTEGRATION_R1.md`](MECHANICAL/COMPONENT_SELECTION_DIMENSION_INTEGRATION_R1.md) for candidate parts, measured envelopes, power architecture and remaining verification items.
