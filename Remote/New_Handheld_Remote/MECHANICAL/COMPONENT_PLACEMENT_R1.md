# New Handheld Remote — Mechanical Refinement & Component Placement

**Revision:** Mechanical refinement R1  
**Status:** Layout proposal; no physical component fit has been verified.

## Purpose
Convert the preliminary handheld remote concept into a placement plan that can guide CAD refinement and a first printed fit shell.

## Envelope target
- Width: 310 mm (target, not locked)
- Height: 190 mm (target, not locked)
- Depth: 62 mm (target, not locked)
- Front panel: 3.2 mm nominal
- Rear wall: 3 mm nominal
- Rounded corner radius: 18 mm nominal
- Display aperture placeholder: 106.8 × 67 mm
- Layout coordinate system: origin at front-panel center; X positive right; Y positive up; Z outward from rear shell.

## Front control placement (draft coordinates)
| Component | X | Y | Footprint / spacing | Status |
|---|---:|---:|---|---|
| Touch display | 0 | -3 | 106.8 × 67 mm opening placeholder | Verify actual board/bezel |
| Power button | 0 | +82 | 16 mm opening placeholder | Verify switch |
| Left joystick | -105 | +58 | 24 mm opening placeholder | Verify body, mounting and throw |
| Right joystick | +105 | +58 | 24 mm opening placeholder | Verify body, mounting and throw |
| Left bank | -112 | -30 | 2 × 4, 21 mm pitch, 14 mm placeholder holes | Verify buttons/caps |
| Right bank | +112 | -30 | 2 × 4, 21 mm pitch, 14 mm placeholder holes | Verify buttons/caps |
| Antenna A | rear, left/upper | — | 8.5 mm hole placeholder | Verify connector and bend clearance |
| Antenna B | rear, right/upper | — | 8.5 mm hole placeholder | Verify connector and bend clearance |
| USB-C service port | lower edge, centered | — | 13 × 7 mm placeholder | Verify connector board and cable |

## Internal layout zones (proposal)
1. **Battery zone:** four matched 18650 cells (1S4P) in a secured lower-central compartment. Pack dimensions, cell holder/retention, protection and thermal clearance must be measured. No cell may be compressed by shell closure.
2. **Main controller/display zone:** ESP32-S3 main board behind the screen or upper-central cavity. Maintain access to USB data, display FPC/cable and mounting points. Actual board must be selected before PCB placement.
3. **nRF24 radio zone:** near antenna A; short RF feed and a clean, dedicated low-noise supply. PA/LNA module heat and RF keepout to be reviewed.
4. **Wi-Fi/Bluetooth antenna path:** independent feed to antenna B; the selected ESP32-S3 module/board must explicitly support external antenna.
5. **Power zone:** charger, protection, power-path and regulators near USB-C and battery, but shielded/segregated from joystick signal wiring and RF feed.
6. **Input wiring:** button/joystick cables routed along side regions and away from switching-power and motor-like high-current paths.
7. **Serviceability:** removable modular boards/connectors, strain relief, labelled cable routes and screw access; avoid trapped cables and inaccessible fasteners.

## Mechanical constraints and risks
- The display is the largest front-to-back stack item; measure screen PCB, connector and cable bend radius, not only visible glass.
- Joystick bodies may extend below the front panel and can collide with battery cells or controller boards.
- Button caps/switch bodies need clearance from display bezel and neighboring controls.
- Battery cells must be secured independently of PCB mounting and protected from screws, sharp edges and shell seams.
- Antenna connectors and cables need clearance; avoid routing antenna cables over battery/power switching components.
- USB-C opening must align with the chosen board/connector and permit cable insertion.
- Add screw bosses, perimeter mating flange, gasket groove and cable strain relief only after component dimensions are known.
- No waterproof rating is claimed. Splash resistance requires gasket/port/seam design and testing.

## Fit-validation sequence
1. Select or obtain exact display, joystick, switches/buttons, radio module, antennas, battery cells and charger/power board.
2. Measure each component including PCB, connectors, mounting holes, cables, and height.
3. Revise parametric CAD footprints and rear internal zone volumes.
4. Export front and rear test STLs; verify render and mesh.
5. Print front panel first, test button cap spacing, joystick reach, screen fit and hand grip.
6. Print rear shell; test depth, battery retention, board mounting, antenna and USB access.
7. Only after physical fit, lock dimensions and proceed to PCB design.

## Status
This document and CAD coordinates are preliminary. The component placement is a concept envelope, not a verified assembly. No battery pack assembly or charging circuit is approved by this document.
