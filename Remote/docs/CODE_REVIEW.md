# Code review — 2026-11-16

Reviewed baseline: `Flytec_Remote_LVGL_9_6_0_Sonar_Settings_Update.zip` from the Library.

## Changes made in this revision

1. **Settings startup null-widget access:** the initial Settings open path previously attempted to set the state of Display-category widgets before those widgets existed. Removed that access; the stored backlight state is applied directly, and the Display category restores its own widgets when opened.
2. **Settings screen lifetime:** Settings is now built once and reused on later opens. This prevents allocating a new full-screen object on every visit and prevents the virtual keyboard pointer from referring to an earlier Settings screen.
3. **Wi-Fi responsiveness:** scanning and connection status are polled with LVGL timers. The previous synchronous scan could stall the LVGL loop and make touch appear unresponsive.
4. **NVS wear:** sonar slider values are saved on release rather than on every drag-value event. Dropdown and switch changes are still saved when changed.
5. **Timeout feedback:** the displayed timeout label is refreshed from the saved preference and explicitly says automatic dimming is not active.
6. **Projection input safety:** Web-Mercator conversion rejects zoom values above 22 before any bit shift. A non-finite heading is passed through without contaminating map coordinates.
7. **Tile request validation:** invalid map-style enum values are rejected rather than silently treated as street tiles.
8. **GPS link validity:** the remote now requires the telemetry packet's radio-connected flag as well as GPS connected, 3D fix, valid coordinate bounds and freshness (≤2 seconds) before setting `gpsValid`.
9. **Documentation:** updated README, added the full master project record, and corrected obsolete SD-card status text in the GPS/map plan.

## Verification performed

- C++ syntax-only check passed for the hardware-independent core modules (`boat_state.cpp`, `boat_telemetry.cpp`, `boat_commands.cpp`, `boat_radio.cpp`, `joystick.cpp`, `map_projection.cpp`) and `settings_ui.cpp`, using lightweight host-side API stubs. This checks syntax and local type consistency only; it does not validate actual Arduino/LVGL/Waveshare library signatures or link behaviour.
- Map projection module compiled with `-Wall -Wextra -Wpedantic -Werror` against a minimal Arduino type stub.
- Map projection numeric checks passed for a Varna-area test coordinate at zooms 0, 5, 12, 18 and 22, including forward/inverse round trips, invalid zoom rejection and the 90-degree boat-up offset case.
- Brace-balance check passed for all 25 C++ source/header/sketch files.
- Full Arduino IDE compile and physical board test were not performed.

## Remaining issues / engineering work

### High priority before hardware use

- The Waveshare display adapter has not been compiled against the exact installed ESP32 Arduino core, `ESP32_Display_Panel`, `ESP32_IO_Expander` and LVGL 9.6.0 libraries. The compile-time LVGL version guard is not a build verification.
- Verify the CH422G EXIO4 SD-card chip-select plus GPIO6 dummy-CS arrangement on the actual board. Confirm SD read reliability while the display is refreshing.
- Verify the RGB LCD flush callback's `drawBitmap()` completion semantics. An asynchronous panel copy would require a correct flush-complete callback before LVGL reuses its buffer.
- The physical display touch orientation, touch calibration, backlight polarity and startup behaviour remain unverified.
- Add real joystick ADC hardware and calibrated pin assignments only after checking all Waveshare pin conflicts and the chosen analogue front-end. The current hardware reader intentionally returns false.
- Implement a real radio transport, packet framing, CRC/authentication, sequence checking, acknowledgements, timeout handling and a boat-side watchdog. The selected nRF24L01+ PA/LNA module is only a placeholder; pin assignments, RF24 driver, packet framing, authentication, sequence checking, acknowledgements, timeout handling and boat watchdog remain unimplemented.
- Do not allow remote STOP-clear or motor/solenoid actuation until boat-side safety interlocks and acknowledgements exist and have been tested.

### Functional incompleteness

- No boat-side firmware is included. Heltec Wireless Tracker V1.2 GNSS parsing, MPU-9250/BMP280 acquisition and calibration, telemetry transmitter, autopilot and return-home behaviour remain boat-side work.
- The map panel remains a diagnostic grid. The current offline-map module only validates a manifest signature and reads one uncompressed RGB565 tile into PSRAM; it has no cache, renderer, pan/zoom controls, map style UI, or waypoint overlays. Tile data is not bundled.
- `offlineMap_readTile()` allocates a 128 KiB buffer for each call. A bounded reusable cache is needed before continuous map rendering; SD reads must not block LVGL.
- The manifest currently validates only the `FLYTEC_MAP1` prefix. A production map pack should add version, style, zoom coverage, tile dimensions/format, attribution/licensing and checksums.
- Sonar settings are local NVS preferences only. No CHIRP sonar module, command protocol, acknowledgement or real echogram renderer is integrated.
- Display night theme and screen timeout are saved preferences only. Variable brightness is not implemented; current backlight control is ON/OFF through CH422G.
- Controls, Map and System settings categories remain placeholders. Joystick calibration is not persisted.
- Wi-Fi password is stored in NVS. Configure ESP32 flash encryption and review the credential-storage threat model for a production remote.
- The Wi-Fi scan and connect flows are asynchronous now, but still need physical testing for reconnects, scan failure, AP loss, and switching Settings categories during a scan/connection attempt.
- `telemetryApplyToBoatState()` freshness and fix checks are not a substitute for a navigation-ready gate. Before autopilot, require a defined maximum horizontal accuracy, valid compass heading, valid home point and boat-side failsafe.

## Safety note

This remains a development baseline and must not be used to operate the boat. Keep motors and solenoids disconnected during display/UI bring-up. Test STOP, radio loss and failsafe behaviour on a bench with current-limited supplies and no propulsion load before any water test.


## 2026-11-20 — target compile preparation

- Target board confirmed: Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948. Variants 4.3B/BOX excluded.
- Source-level LVGL 9.6 adapter audit recorded in `LVGL_9_6_DISPLAY_ADAPTER_AUDIT_R1.md`; exact library API signatures remain pending target compile.
- Added target Arduino CLI workflow and candidate dependency pins. The first run is still in progress; no compile success is claimed.
- Remote host CI was inspected: first run failed at release-guard unit test because its fixture did not populate the new fail-closed state fields (`emergencyStopConfirmed` and `actuatorSafetyReady`). The test fixture was corrected to initialise the safety state explicitly. The next CI run is pending.
