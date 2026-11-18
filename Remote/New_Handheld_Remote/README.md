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


## Confirmed component direction R2
See [`COMPONENT_SELECTION_DIMENSION_INTEGRATION_R1.md`](MECHANICAL/COMPONENT_SELECTION_DIMENSION_INTEGRATION_R1.md) for updated nRF PA/LNA, boost, protection, fuse and thermal recommendations.


Power PCB design: [`POWER_DISTRIBUTION_DESIGN_R1.md`](PCB/POWER_DISTRIBUTION_DESIGN_R1.md)


## Net-level power schematic draft R1 — 2026-11-18
- Created `PCB/POWER_SCHEMATIC_NET_DRAFT_R1.md`: BATTERY_INPUT, TPS61088 5 V boost, TPS63070 3.3 V radio buck-boost, SENSE_CONTROL, connector sheet/net groups, provisional power budget and unresolved component values.
- This is a reviewed net-level draft, not a KiCad schematic or fabrication-ready design. Feedback/compensation, inductors, current limits, exact capacitors, fuse, reverse-polarity circuit and connector pins remain to be calculated/selected.


## First converter schematic sheet — pin/net map R1
- Added `PCB/TPS61088_PIN_NET_MAP_R1.md` with pin-to-net connections for the 5 V boost converter, functional external-component list, and caveats requiring package/pin verification.
- This is not yet a native KiCad schematic; local KiCad CLI/editor was not available for generating or ERC-validating a schematic file in this environment.


## TPS63070 radio regulator pin/net map R1 — 2026-11-18
- Added `PCB/TPS63070_PIN_NET_MAP_R1.md`, using TI datasheet Rev. B pin assignments and functional connections for the 3.3 V nRF24 PA/LNA supply.
- Confirmed pin numbers: L1=11, L2=9, VIN=12/13, EN=14, VSEL=15, PS/SYNC=1, PG=2, VAUX=3, GND=4, FB=5, FB2=6, VOUT=7/8, PGND=10.
- TI's stated operating range is 2.0–16 V input and 2.5–9 V output; up to 2 A under specified conditions. The remote's 3.3 V radio rail is still a design target, not validated capacity.
- Feedback values, inductor/capacitors, EN default, PS/SYNC mode, PG usage, footprint verification and layout remain open. Document is not a native KiCad schematic.


## Battery input and protection net map R1 — 2026-11-18
- Added `PCB/BATTERY_INPUT_PROTECTION_NET_MAP_R1.md` documenting the pack connector, provisional 3 A time-delay fuse, reverse-polarity protection, BAT_PROT distribution, battery sensing/NTC and test points.
- Pack protection/BMS is assumed but must be verified; charging remains out of scope. Fuse, connector, reverse-protection parts, bulk capacitors, ADC divider and thermistor interface remain TBD.
- Battery input is documented as a net-level plan only, not a KiCad schematic or battery-pack design.


## Power document consistency review — 2026-11-18
- Harmonized the battery distribution net name to `BAT_PROT` across power architecture, net draft and converter pin/net maps.
- Corrected component-selection notes: onboard ESP32 antenna is accepted; external ESP32 antenna requirement is waived.
- Charging circuitry remains out of scope. USB-C charging/power-path roles are not electrically finalized; no charger IC or charging current is approved.

## TPS63070 radio rail — component calculations R1
- Engineering calculation record: PCB/TPS63070_COMPONENT_CALCULATIONS_REVIEW_R1.md.
- Feedback pair proposed: 470 kΩ / 150 kΩ for 3.307 V nominal (0.8 V FB reference); inductor candidate Coilcraft XFL4020-152ME, 1.5 µH.
- TI-reference input capacitors; proposed 98 µF nominal output bank pending effective-capacitance/DC-bias verification.
- Radio branch budget: 200 mA output load → approximately 245 mA battery current at 3.0 V and assumed 90% efficiency. Whole remote preliminary estimate is approximately 1.13 A at 3.0 V under stated display/radio assumptions; not measured.
- Component selection and calculations remain subject to datasheet/package checks, DC-bias capacitance verification, schematic review, layout, stability and thermal testing. Native KiCad schematic/ERC not yet created.

## Datasheet correction and configuration review update (2026-11-18)
- TPS63070 adjustable single-rail configuration: FB2 grounded; VSEL held LOW (no dynamic voltage scaling); PS/SYNC HIGH to enable power-save PFM/PWM operation; EN pulled high through a 10 kΩ resistor to BAT_PROT for always-enabled rail. These are the design choices to implement, pending symbol/net review. If MCU shutdown control is added, it must safely override EN.
- PG is open-drain; 100 kΩ pull-up to 3V3_RADIO is a candidate for MCU status monitoring, not mandatory. Verify the intended MCU input and PG behavior before capture.
- Capacitor-bank proposal corrected: follow TI's adjustable 3.3 V typical application BOM (3×22 µF X6S 0805 + 10 µF 0603, nominal 76 µF), rather than the earlier 98 µF proposal. TI's filter table supports 1.5 µH with nominal 47/68/100 µF; the 76 µF typical-app bank is within the recommended combination table. Effective capacitance remains to be confirmed using manufacturer DC-bias/temperature data. Datasheet permits effective output capacitance 15–470 µF for nominal 1.5 µH.
- Selected 22 µF candidate: Murata GRM21BC81C226ME44L, 16 V X6S, 0805, ±20%, -55°C to +105°C. Exact DC-bias capacitance at 3.3 V is not stated in the basic part specification and must be verified from Murata's bias curves/approval data. Three units are specified by TI's reference BOM.
- The prior 98 µF nominal output proposal is superseded. See the calculation review and pin/net map for corrected baseline.
Sources: TI TPS63070 Rev. B datasheet, §§8.5.1, 9.2.2.1–9.2.2.3, Tables 2–4; Murata GRM21BC81C226ME44L reference sheet.
