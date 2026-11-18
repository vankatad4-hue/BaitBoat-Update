# GPS location and offline map integration

## Hardware/data path

The GPS receiver is installed on the boat, not the handheld remote. The selected boat controller/GNSS board is the Heltec Wireless Tracker V1.2 (ESP32-S3FN8 + UC6580 GNSS). GNSS UART routing, protocol, update rate, power-enable behavior and antenna configuration must be confirmed against the exact V1.2 schematic and tested board; do not carry over HGLRC-specific UBX settings. Heading/motion data comes from the selected GY-91-style MPU-9250 + BMP280 module over I2C initially. Verify the exact breakout pinout, voltage and address before wiring.

Boat-side firmware responsibilities (not yet implemented in this remote archive):

1. Parse UBX NAV-PVT and validate fix type, coordinates, satellite count and horizontal accuracy.
2. Read MPU-9250 accelerometer, gyroscope and magnetometer; calibrate the magnetometer, correct mounting orientation and validate magnetic interference with motors and solenoids operating.
3. Build a radio telemetry message containing latitude/longitude, fix type, satellite count, horizontal accuracy, heading, speed and a sequence/timestamp.
4. Transmit it over the selected radio link with integrity checking.

Remote responsibilities implemented in this revision:

- `BoatTelemetry` now carries GPS fix type, satellite count and estimated horizontal accuracy.
- `telemetryUpdate()` timestamps the local receipt of a telemetry update.
- `telemetryApplyToBoatState()` accepts a position only when the receiver reports a 3D fix, coordinates are in range and telemetry is no older than 2 seconds.
- The main screen displays latitude, longitude, satellites, estimated accuracy and heading. The boat marker remains hidden without a fresh valid fix.

The selected nRF24L01+ PA/LNA radio transport is still a placeholder. Therefore, no live coordinates can reach this UI until the nRF24L01+ packet decoder calls `telemetryUpdate()` with a validated packet.

## Offline map decision

Use offline map data on a microSD card in the Waveshare remote. Do not depend on Wi-Fi or a public online tile server while operating the boat. The map viewport in this revision is only a clearly-labelled diagnostic grid and boat-position marker; it is not a real geographic map yet.

The current remote display adapter now attempts to mount the Waveshare SD interface using GPIO11/12/13 and the board-owned CH422G EXIO4 card select. The GPIO6 dummy-CS arrangement is documented in `OFFLINE_MAP_STORAGE.md` and still requires physical-board verification.

The open-source `0015/map_tiles` project supports LVGL 9.x and RGB565 tile files, but its published installation targets ESP-IDF component projects. It is not a drop-in Arduino IDE library. We should either port its small tile-loading layer to Arduino FS/SD APIs or write a narrow Arduino-native loader; do not add it unchanged to this sketch.

## Map behaviour to implement next

- Boat-up orientation (the selected interface preference): keep the boat marker centred and rotate the map using a calibrated compass heading.
- Street/standard and satellite map styles, with a user-selectable layer.
- Zoom in/out and recenter-on-boat controls.
- Saved waypoints and home marker; waypoint coordinates must be stored independently from the map tiles.
- Map tile cache in PSRAM; SD reads must not block the LVGL UI task.
- Missing tiles must show a clear no-map-data state and must never be mistaken for land/water geometry.

## Safety and validation

GPS position alone is not proof of a safe route or a valid heading. Autopilot and return-home must require fresh GNSS data, valid compass heading, a valid home point, acceptable position accuracy, and boat-side watchdog/failsafe checks. If GPS becomes stale or invalid, the remote must visibly report it and the boat-side controller must independently decide its safe response.

## Map renderer implementation stage (2026-11-16)

The current working archive now includes `map_projection.h/.cpp`, a hardware-independent Web-Mercator projection module. It converts WGS84 latitude/longitude to slippy-map world pixels and back, and rotates viewport offsets using compass heading. This is the geographic math foundation for a boat-up renderer and can be unit-tested without a Waveshare display or a lake-specific map pack.

Important current limitation: this module does not yet draw map tiles in LVGL. The existing `boat_ui.cpp` grid remains a diagnostic placeholder. The next implementation step is a tile cache and RGB565 viewport renderer, followed by zoom/style/recenter controls and home/waypoint overlays. The map source and coverage must be selected and packaged separately; no map tiles are fabricated or downloaded from the public OSM tile server.


For the complete project decisions and current readiness boundary, see [`PROJECT_MASTER_STATUS.md`](PROJECT_MASTER_STATUS.md).
