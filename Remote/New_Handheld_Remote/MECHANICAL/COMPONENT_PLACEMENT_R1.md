# New Handheld Remote — Mechanical Refinement & Component Placement

**Status:** Placement proposal; no physical fit verified.

## Envelope target
- Width 310 mm, height 190 mm, depth allowance 70 mm (provisional).
- Front panel 3.2 mm, rear wall 3 mm, corner radius 18 mm (initial modeling values).
- Display aperture 106.8×67 mm placeholder.
- Coordinates use front-panel center as origin; X positive right, Y positive up.

## Front controls
| Component | X | Y | Placeholder |
|---|---:|---:|---|
| Display | 0 | -3 | 106.8×67 mm aperture |
| Power button | 0 | +82 | 16 mm opening |
| Left joystick | -105 | +58 | 24 mm opening |
| Right joystick | +105 | +58 | 24 mm opening |
| Left button bank | -112 | -30 | 2×4, 21 mm pitch |
| Right button bank | +112 | -30 | 2×4, 21 mm pitch |

## Internal zones
1. Battery: lower-central 1S4P pack reserve; target volume 104×82×24 mm. Retention and insulation must be designed independently.
2. Display/controller: Waveshare ESP32-S3-Touch-LCD-4.3 board; exact revision, connectors and cable bend radius must be measured.
3. Radio: nRF24L01+ PA/LNA near its single external antenna exit; dedicated 3.3 V regulator.
4. ESP32 RF: onboard antenna region on Waveshare board; preserve keepout and avoid metal/battery/cables.
5. Power: TPS61088 5 V boost and TPS63070 3.3 V radio regulator near battery/power distribution, outside battery reserve and away from RF feed.
6. Controls: side cable paths for joystick/button signals, separated from switching loops.
7. Service: keyed connectors, strain relief, access for screws and removable modules.

## Antenna/connectors
One external nRF antenna exit; ESP32 antenna is onboard. Do not design an unused second antenna bulkhead. Verify antenna cable and connector clearance and RF performance in final enclosure.

## Risks / validation
Screen board stack, joystick bodies, button travel, pack clearance, cable routing, grip ergonomics, RF keepout, shell depth and splash protection remain unverified. Use real component samples and a printed fit prototype before locking CAD. The enclosure is not waterproof.
