# New Handheld Remote — Preliminary Concept

**Status:** Preferred concept / preliminary design only. Not final CAD, PCB, firmware, or purchased hardware.

## Design baseline
- New ergonomic handheld controller inspired by modern game controllers.
- Central 4.3-inch landscape touchscreen for map, sonar, telemetry, and settings.
- Two analog joysticks positioned **above** the button clusters.
- **16 physical buttons total**, split equally: 8 left and 8 right.
- Each side uses a **2 columns × 4 rows (2×4)** button grid.
- Plain functional labels; no PlayStation symbols.
- No shoulder buttons and no rear buttons.
- Ergonomic lower grips.
- Two rear antenna exits, both functional and electrically independent:
  - Antenna A: nRF24L01+ PA/LNA boat-control/telemetry radio.
  - Antenna B: ESP32-S3 Wi-Fi/Bluetooth, only with a board/module variant that supports an external antenna.
- Battery concept: **1S4P Li-ion, 4 × 2500 mAh matched cells = 10,000 mAh nominal pack capacity**, USB-C charging.
- Dark graphite/black enclosure with blue accents is the preferred visual direction.

## Preliminary button assignment

### Left bank (2×4)
| Position | Function |
|---|---|
| L1 | MENU |
| L2 | HOME |
| L3 | OPTIONS |
| L4 | PAUSE |
| L5 | STOP |
| L6 | WP SELECT |
| L7 | SAVE WAYPOINT |
| L8 | NAV / navigation command |

### Right bank (2×4)
| Position | Function |
|---|---|
| R1 | BAIT |
| R2 | HOOK |
| R3 | OK / CONFIRM |
| R4 | BACK |
| R5 | SONAR |
| R6 | MAP |
| R7 | CAM (reserved; depends on camera feature) |
| R8 | LIGHT (reserved; depends on boat lighting hardware) |

Button labels and functions are preliminary and can be remapped without changing the physical 2×4 layout. Bait/hook activation must have deliberate safety confirmation/hold behavior. STOP must override propulsion commands and inhibit release actions; exact STOP latch/reset behavior will be defined in the safety specification.

## Preliminary mechanical envelope (not final)
- Width: approximately 290–320 mm
- Height: approximately 175–195 mm
- Depth: approximately 55–70 mm
- Display opening/fit, joystick centers, button spacing, grip ergonomics, antenna spacing and mounting details require CAD refinement and prototype testing.

## Dual-antenna RF note
Two antennas are intended for two separate RF systems; they are not diversity antennas for a single nRF24 radio. Both systems operate in the 2.4 GHz band, so antenna placement alone does not guarantee coexistence. RF separation, ground-plane/layout, filtering, radio scheduling/coexistence and regulatory EIRP must be evaluated. Do not connect both radios to a shared antenna.

## Power note
A 1S4P pack built from four 2500 mAh cells has 10,000 mAh nominal capacity. With 3.6–3.7 V nominal cell voltage, pack nominal energy is approximately 36–37 Wh. Actual charging, protection, fuse, cell matching, thermal design, regulator sizing, and pack assembly safety must be engineered before build. USB-C charging implementation is TBD.

## Next design phase
Create a dimensioned front/rear/side template and CAD mock-up. Validate display and joystick component footprints, button spacing and thumb reach, antenna placement and internal battery/electronics volume before selecting final parts or producing PCBs.


## New hardware-design additions
- Dedicated physical power ON/OFF button centered above the display, separate from the 16 function buttons. Electrical implementation (soft power latch vs direct switched power) remains to be selected.
- USB-C service port planned on the lower enclosure edge. It is intended for 1S pack charging and ESP32-S3 firmware/data access through one connector. This requires a validated USB-C sink/CC implementation, ESD protection, a charger with suitable power-path/load-sharing behavior, and USB data routing compatible with the selected ESP32 module. Do not assume charging and programming can safely occur simultaneously until the power-path circuit is designed and tested.
- Hardware design workspace: `Remote/New_Handheld_Remote/`. It contains an initial OpenSCAD fit template, mechanical print plan, modular PCB architecture, connector interface plan and schematic checklist. These are preliminary design files, not fabrication-ready CAD/PCB/schematics.
