# New Handheld Remote — Internal Component Placement Mockup R1

**Status:** Spatial study only; no actual component footprints/fit verified.

## Layout zones
- Integrated display/controller board: centered behind screen; provisional envelope only. Exact Waveshare revision and connectors must be measured.
- nRF24L01+ PA/LNA module: RF zone near the single external nRF antenna exit; placeholder dimensions.
- Power PCB: TPS61088 5 V display boost and TPS63070 3.3 V radio buck-boost near battery distribution, but outside cell reserve and away from RF feed.
- Inputs: joystick/button cable zones along sides.
- Battery: four 18650 parallel cells in lower-central zone, 1S4P. Cell dimensions and retention are not modeled as validated hardware.
- USB-C: lower service edge, cable clearance required.
- ESP32 RF: onboard antenna on Waveshare board; reserve antenna keepout. No external ESP32 antenna cable or bulkhead.

## Conflicts/fit checks
1. Board, screen, connectors and cable bend radius versus shell depth.
2. Joystick bodies versus battery/controller.
3. Cell pack fit, retention, insulation and clearance.
4. RF keepout and 2.4 GHz coexistence.
5. Power-converter switching loops and heat versus RF/input wiring.
6. USB-C, radio antenna feed and service access.

## CAD
The OpenSCAD mockup is a visualization only. It does not define a cell holder, production PCB placement, antenna performance or fabrication geometry. Physical fit and enclosure thermal/RF checks are required.
