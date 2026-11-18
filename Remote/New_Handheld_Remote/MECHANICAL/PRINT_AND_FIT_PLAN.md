# Mechanical Print and Fit Plan

## Prototype assumptions
- Outer envelope: 310×190×70 mm provisional.
- Front panel: 3.2 mm; rear wall: 3 mm (initial CAD assumptions).
- Display opening: 106.8×67 mm placeholder.
- Power button opening: 16 mm; joystick openings: 24 mm; button openings: 14 mm placeholder.
- One nRF antenna exit: connector-specific hole TBD.
- USB-C lower-edge service opening: 13×7 mm placeholder.

## Print approach
Print front panel and rear tray separately. Use nonconductive filament for initial fit; RF must be tested in final housing. Add bosses/ribs and battery restraint only after component placement is verified.

## Fit sequence
1. Print front template; test display opening and control spacing.
2. Fit actual screen board, joystick and switch samples.
3. Print rear tray and check shell depth, battery reserve and grip.
4. Check nRF antenna connector/feed clearance; maintain Waveshare onboard ESP32 antenna keepout.
5. Add mounting bosses, cell restraint and cable guides.
6. Validate splash protection and thermal behavior separately.

This enclosure is not waterproof. All dimensions and cutouts are placeholders until actual parts are measured.
