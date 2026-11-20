# Remote R6.3 — Preliminary Internal Component Placement Plan

**Status:** Layout planning only. Electronics have not been purchased and dimensions are not yet verified.

## Layout diagram and CAD view
- [R6.3 internal layout SVG](../CONCEPT/RENDER_R6_3/Remote_Internal_Layout_R6_3.svg)
- CAD source has an `internal` visualization mode showing provisional envelope blocks.

## Coordinate reference
Use the R6.3 CAD origin at the enclosure center; X is left/right, Y is up/down on the front view, Z is depth from the front face. Front controls are already placed in R6.3.

## Component zones
| Zone | Planned placement | Notes |
|---|---|---|
| Touch controller | Directly behind display, centered | Waveshare ESP32-S3-Touch-LCD-4.3; board/cable/connector clearance to be verified |
| Left joystick module | Behind left joystick opening, near (-108,+66) | Keep ADC wiring away from RF feed and switching-power traces |
| Right joystick module | Behind right joystick opening, near (+108,+66) | Same module type preferred |
| Primary radio | Upper-left interior, antenna feed to left/top exit | nRF24L01+ PA/LNA; SPI/control harness; keep coax short |
| Backup radio | Upper-right interior, antenna feed to right/top exit | E22-900T22S 868 MHz version; safety-only link; separated from primary |
| Battery | Lower-center compartment | 1S4P 18650 pack; exact holder/pack dimensions and protection arrangement TBD |
| Charging/protection | Lower bridge near USB-C | Pack BMS/protection is assumed external to carrier only after its exact pack is selected; charger must be specified separately |
| Power conversion | Lower-right or lower-center side zone | 1S input boost/regulation rails; converter thermal and current capability TBD |
| Button wiring | Side banks routed along side walls | Keyed connector harness; release controls use independent GPIO/logic lines |
| USB-C service | Lower bridge opening | Mechanical fit and cable strain relief must be checked |

## Preliminary zone coordinates and placeholder footprints
All values are CAD envelope estimates, in mm; center coordinates use the front-view X/Y convention.

| Module | Center (X,Y) | Placeholder envelope (W×H) |
|---|---:|---:|
| Waveshare touch display/controller board | (0, +7) | actual board 106.1×67.8; reserve envelope 108×72 (tight provisional) |
| Left joystick module | (-108, +66) | 36×36 |
| Right joystick module | (+108, +66) | 36×36 |
| nRF24 PA/LNA | (-123, +25) | 42×26 |
| E22-900T22S backup radio | (+126, +23) | 26×16 (module only; connectors/feed clearance additional) |
| 1S4P pack | (0, -61) | 84×36 |
| Charger/protection | (-65, -74) | 30×20 |
| DC/DC converter | (+71, -71) | 38×24 |
| Power distribution | (0, -32) | 44×18 |

The display and E22 module dimensions are from manufacturer documentation; other blocks remain **not measurements of selected products**. Some zones may overlap the display-board, battery or switch/connector envelopes; this layout is only a placement study. Full collision checks require exact parts and 3D dimensions.

## Preliminary stack-up strategy
- Touchscreen/controller PCB sits immediately behind the display window.
- Joystick assemblies are placed behind their apertures; provide fastener and stick travel clearance.
- Battery is retained in a removable cradle at the lower center, with cell insulation and mechanical restraint.
- Radio modules are kept near their antenna exits and away from converter inductors, battery-current wiring, and display flex cables.
- Power and high-current battery/charging paths remain on one side/lower zone; low-level ADC, SPI, and button signals route separately.
- Antennas and connectors must not be blocked by battery, metal fasteners, or grip structures.

## Electrical/mechanical checks before final CAD
- Decide the rail architecture from the 1S4P pack: boost/regulation, display-board input, RF supply filtering, and current capacity must be calculated before choosing converter footprints.
- The charge/protection block shown is not a finished charger or BMS design; the exact pack configuration and protection/charging solution remain open.

## Electrical/mechanical checks before final CAD
1. Obtain exact dimensions, connector locations, and mounting patterns for the display board, joysticks, nRF PA/LNA, E22, USB-C/charger, converter, and battery pack.
2. Check both radio antennas' physical clearance and RF isolation. A single dual-band antenna is not assumed.
3. Confirm battery-cell configuration, pack protection, charging current/temperature sensing, and safe enclosure ventilation/thermal limits.
4. Confirm PCB/cable bend radii, connector access, joystick full travel, release-key guards, and assembly/service sequence.
5. Build an internal CAD layout with collision/clearance checks only after dimensions are confirmed.

**No internal components are considered fixed or manufacturing-ready in this revision.**
