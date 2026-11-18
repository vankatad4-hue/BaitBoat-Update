# New Handheld Remote — Internal Component Placement Mockup R1

**Status:** Spatial study only. No actual component footprints or fit have been verified.

## Mockup zones
- Display/interface board: centered behind the display aperture; placeholder envelope 130 × 80 × 8 mm.
- Main ESP32-S3 controller: upper-center placeholder envelope 80 × 50 × 10 mm. Actual location must avoid joystick bodies, display connectors, antenna feeds and cable bend radii.
- nRF24L01+ PA/LNA module: left/upper RF zone, close to Antenna A; placeholder envelope 32 × 22 × 12 mm.
- USB-C charging/power-management board: lower-right service zone, near USB-C port; placeholder envelope 55 × 35 × 10 mm.
- Input/cable strips: along the left and right side zones; illustrative placeholders only.
- Battery: four 18650 cells, shown as a 2×2 group in the lower-central zone. Cell model is 65 × 18 mm envelope; pack restraint, wiring, protection, spacing and clearances are not modeled.
- Antenna connector stubs: rear upper corners, to visualize RF cable routing.

## Important conflicts to resolve
1. The display PCB and main ESP32 board may overlap in the central depth stack; actual board thickness, mounting, connector and cable geometry must be measured.
2. The joysticks extend into the rear cavity and may collide with the controller board or RF modules.
3. The nominal 62 mm shell depth is a target only. The depth stack, cable bends and cell restraint may require changes.
4. The battery zone is illustrative. Cells require mechanical retention, insulation, protection, fuse/temperature considerations and safe charging design; this mockup is not a battery holder or pack design.
5. Antenna separation and RF coexistence must be assessed for both 2.4 GHz systems. The diagram does not demonstrate RF performance.

## CAD usage
Open CAD/internal_layout_mockup.scad in OpenSCAD. This file is a separate visualization of internal placement zones; it does not replace the front/rear shell CAD or define fabrication geometry.
