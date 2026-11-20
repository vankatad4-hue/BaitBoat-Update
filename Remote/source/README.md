# Remote Source — Current Development Status

Updated 2026-11-20. The GitHub repository is canonical; this source is a development scaffold, not a tested release.

## Versioning
- ZIP: `Flytec_Remote_verXXX.zip`.
- Preserve older archives; the packaged baseline is ver028. Current development source is unreleased ver029.13; package marker remains ver028. do not update `releases/VERSION` or create a release ZIP until packaging and verification are complete.
- Increment for every package change and follow `../../docs/ZIP_RELEASE_POLICY_R1.md`.

## Controller/display baseline
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948, 800×480 RGB, GT911 touch, CH422G expander.
- Arduino IDE; target LVGL 9.6.0 only. Board-support libraries are separate from LVGL.
- Confirm Arduino-ESP32 core, board menu settings, PSRAM and exact display libraries before compile.
- Display adapter is a first integration pass, not compile or physical validation.

## Remote hardware/radio baseline
- Primary nRF24L01+ PA/LNA, 2.4GHz; left external antenna.
- Backup EBYTE E22-900T22S UART, EU868 candidate; right external antenna.
- E22 UART bridge: SC16IS750 I2C address 0x48; I2C GPIO8/9. UART 9600 8N1.
- Provisional nRF CE GPIO6, CSN GPIO15, SPI MOSI11/SCK12/MISO13. GPIO6 sensor-AD path and GPIO15 RS485 TX are repurposed; isolate/disconnect onboard functions and verify exact board revision before wiring.
- nRF channel76, 250kbps, fixed32, CRC16, PA_LOW initial.
- E22 candidate channel18=868.125MHz, 62.5kbps, 32-byte subpacket, 10dBm, transparent; exact module variant and configuration readback must be confirmed.
- 1S4P 18650 battery proposal. The Waveshare PH2.0 input/charger recommendation, protection and simultaneous load behavior must be checked; do not assume the 2400mAh pack proposal is approved by the onboard charger.

## Code inventory
- `Flytec_Remote.ino`: app entry and setup.
- `display_port.*`: board LCD/touch and LVGL adapter.
- `boat_state.*`, `boat_commands.*`, `boat_radio.*`: application state/command scaffold.
- `nrf24_link.*`, `e22_uart_link.*`, `sc16is750_bridge.*`: radio transport and UART bridge.
- `radio_protocol.*`, `radio_command_codec.*`, `radio_key_store.*`: CRC/HMAC frame and command safety scaffolds.
- `dual_radio_manager.*`: link state/failover/recovery policy.
- `physical_button_scanner.*`: MCP23017 button scan/debounce; right-bank release keys and separate OK input feed the release-confirmation state machine. Board pin map is provisional.
- `release_button_guard.*`: two-step release guard, primary-link and boat interlock gate; native host test available.
- `joystick.*`, `joystick_calibration_ui.*`: calibration UI; hardware ADC reader currently fails closed.
- `boat_ui.*`, `settings_ui.*`, `mission_ui.*`, `joystick_calibration_ui.*`, `map_manager_ui.*`: UI implementation; sonar pane is an explicitly marked visual demo, not operational sonar.
- `offline_map.*`, `map_projection.*`, `map_renderer.*`: SD XYZ RGB565 tile read, projection, bounded tile viewport, waypoint overlays and validated source-attribution overlay; source is not hardware-validated.
- `map_source.*`: persisted AUTO / ONLINE / OFFLINE map source preference scaffold; online provider is deliberately unavailable and ONLINE selection falls back to SD.
- `boat_telemetry.*`: partial telemetry state application with local group freshness, coordinate sample pairing and primary-link safety provenance; wired to Remote receive dispatch.
- `radio_payload_codec.*`: strict eight-byte HEARTBEAT/ACK/TELEMETRY payload validators; integrated in authenticated receive dispatch.
- `radio_payload_encoder.*`: portable payload encoders, host round-trip tests, mirrored on Boat.
- `radio_ack_tracker.*`: bounded command lifecycle tracker (waiting/accepted/executed/rejected/timeout); wired to non-manual command send and authenticated ACK receive.

## Bench test sketches
- `hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `hardware_test/E22_SC16IS750_Bench/E22_SC16IS750_Bench.ino`
- Fixed diagnostic probes only, CRC checked, no authenticated commands or actuator payloads. See `hardware_test/README.md`.
- Bench tests remain uncompiled/unverified.

## UI milestone ver029.9
- Main UI has Bulgarian labels and an equal map/echogram split; the sonar plot is a visual DEMO only, labelled as disconnected and not sourced from sensor data.
- Mission planner has 16 slots and selected waypoint coordinates; distance/bearing is calculated only from a fresh (<2 s) GPS fix.
- Settings content is partially localized into Bulgarian. LVGL Pro rendering, Cyrillic font coverage and touch behavior still require checking.
- See `../docs/UI_BUILD_VER029_9.md` and SVG previews `UI_PREVIEW_MAIN_VER029_9.svg`, `UI_PREVIEW_MISSION_VER029_9.svg`.

## Safety / limitations
- Key storage returns false by default; authenticated communication/control is disabled.
- Remote session/replay protection, ACK lifecycle tracking and telemetry dispatch are implemented in the current scaffold; UI ACK display, boat-side state synchronization, command execution and ACK/telemetry generation remain incomplete.
- Joystick ADC input remains fail-closed pending hardware validation. Waypoint save/select/home and BAIT/HOOK planning markers are UI/NVS scaffolding only; no boat-side mission executor, autopilot, return-home execution, release control path, sonar integration or validated live telemetry is operational. Release GPIO scanning is coded but not physically verified; safety readiness remains false by default.
- Remote STOP is not an independently validated safety system.
- Do not connect actuator power or operate the boat.

## Canonical project record
Read `../docs/PROJECT_MASTER_STATUS.md` for the full project state, requirements, hardware selections, blockers and release gates.

## Joystick subsystem update

`joystick_adc.cpp` reads four ADS1115 inputs (provisional 0x4A/AIN0–AIN3). The main sketch uses this reader; failed reads suppress stick output. Calibration and control preferences are stored in ESP32 NVS and only a fully validated endpoint capture sets the calibration-valid flag. Controls Settings exposes layout, override, dead-zone and calibration entry. See `../docs/JOYSTICK_ADC_INTEGRATION_R1.md`.


## Remote radio receive integration R2
Authenticated receive frames are dispatched to strict payload decoders. Remote ACK lifecycle records are correlated and exposed to the UI API; telemetry is applied with local group freshness and primary-link safety provenance. Boat-side generation/execution is still missing, and the complete firmware has not been compiled. See `../docs/REMOTE_FIRMWARE_INTEGRATION_REVIEW_R2.md`.


## UI/input milestone — Remote ver024
- Main screen footer now provides touchscreen MANUAL, AUTOPILOT and RETURN HOME requests, alongside STOP and calibration navigation. Preview: `UI_PREVIEW_REMOTE_VER024.svg`.
- Live stick telemetry reports raw readings before calibration and normalized values/neutral-active state after calibration. ADC failure is displayed explicitly.
- Footer safety summary reports STOP confirmation state, low-battery/failsafe/interlock state and pending non-manual command ACK count.
- Battery display is unknown (`--`) until fresh percentage telemetry is available.
- This is feedback/request UI only. Boat application command execution remains absent, key provisioning is fail-closed, and the source has not been compiled for the exact Waveshare/LVGL target.


## Remote ver025 — waypoint/mission-planning UI scaffold (superseded by ver026 storage)
- Footer now includes a MISSION button opening a dedicated planning screen.
- ver025 initially used four volatile waypoint slots; ver026 replaces that storage with NVS persistence.
- HOME and waypoint capture require a fresh valid GNSS fix (<2 seconds).
- Selected waypoint can be tagged with planned BAIT / HOOK markers; they do not issue actuator commands.
- Mission start remains blocked. No mission execution, geofence, authenticated boat command or release sequence is implemented.
- Preview: `UI_PREVIEW_MISSION_VER026.svg`; updated main screen: `UI_PREVIEW_REMOTE_VER026.svg`. Target build and touch/hardware validation remain pending.


## Remote ver026 — waypoint storage/map overlay
- Mission plan waypoint/home coordinates and BAIT/HOOK marker flags are saved to ESP32 NVS Preferences with a version magic and CRC32 integrity check. Invalid/corrupt records are discarded; invalid coordinate entries are cleared.
- Save current fix and set home reject missing/invalid or >2-second-old GNSS fixes.
- Main map placeholder displays home and waypoint marker labels in a local relative view, with BAIT/HOOK suffixes. This is not a navigational map: no tiles, route, compass orientation or geospatial accuracy validation.
- Mission start stays disabled. No boat mission execution or release commands.
- Version source marker ver026; archive and checksum are not yet verified.

- Hardware verification checklist: `Remote/docs/MISSION_NVS_TEST_PLAN_VER027.md` (not yet run).


## Remote ver027 — 16 waypoints + SD map storage
- Waypoint plan expanded to 16 persistent NVS slots plus independent Home. UI presents four rows × four columns; each slot can be selected and saved. BAIT/HOOK flags persist.
- Save uses fresh GNSS (<2s) and reports NVS write success/failure. CRC validation retained. Marker array supports all 16, still on placeholder local grid.
- Integrated TF/microSD designated as offline-map data storage. Existing manifest/tile reader uses `/maps/manifest.txt` and RGB565 XYZ tile files; tile rendering and map pack generation remain not implemented.
- See `../docs/OFFLINE_MAP_SD_STORAGE_VER027.md` and test plan.


## Remote ver028 — offline renderer and SD mission persistence
- Integrated a Web-Mercator tile renderer reading SD XYZ RGB565 files, bounded 3×3 viewport cache, zoom buttons 5–19 and neutral missing-tile fallback.
- Overlay all 16 waypoint markers, Home, BAIT/HOOK tags and a straight route to selected WP or Home in Return-Home mode.
- Waypoint/Home data stored in `/missions/waypoints.bin` on SD with CRC and read-back validation; NVS backup/fallback and four-slot migration remain.
- Offline MBTiles conversion helper: `tools/mbtiles_to_flytec.py` (Pillow dependency; attribution required).
- Source remains uncompiled/unverified; map imagery is not bundled. North-up only; pan, layer switch, route planning and hardware tests remain pending.


## Map source UI scaffold — ver029 development
- Added a map-panel source button cycling AUTO → ONLINE → OFFLINE; preference is persisted in NVS.
- Status text explicitly reports SD map readiness and indicates ONLINE is unavailable with SD fallback. No Google API calls, online imagery, or tile caching are present.
- This is a source/UI scaffold only. Offline area download, licensed map-pack coverage metadata, Wi-Fi internet detection and authorized online-provider integration remain next tasks.
- New files: `map_source.h/.cpp`. Include them in the Arduino sketch build by virtue of their folder placement; confirm Arduino IDE compiles all translation units.
- No full firmware compile or display hardware test has been performed for this change.

- `tools/mbtiles_area_export.py`: crops an MBTiles archive to the planned square/zoom tile set and writes RGB565 files plus source-attribution manifest and completeness report. This is offline conversion, not network downloading; it requires Pillow and a licensed source archive.

- Offline pack validation now requires complete flag, attribution, licence URL and zoom metadata. Older packs must be regenerated; renderer shows the pack attribution label.

## Archive workflow safeguard
Development pushes no longer overwrite repository versioned ZIPs unless Remote or Boat `releases/VERSION` changes. Push/PR/manual runs may still upload a snapshot artifact for inspection. Release marker bump and package verification remain explicit release steps.

## Archive integrity note
The legacy packaging workflow previously regenerated the unchanged ver028/ver014 ZIP names on every source push, repeatedly committing them. The branch archives therefore must not be treated as immutable verified releases. The workflow guard is now in place and verified on a run; restoration of an earlier release archive is not yet done. Source remains unreleased development.

UI follow-up: sonar plot scales and redraws on split/fullscreen; remaining sonar option labels translated; waypoint distance math has floating-point clamping. Compile/test still pending.

Source cleanup: literal escaped line breaks removed from boat UI; waypoint calculation clamp added. Compilation remains pending.

## Continuation checkpoint
Read `../../docs/CONTINUATION_CHECKPOINT_VER001.md` for the complete current status and restart plan. Consolidated changelog is `../releases/CHANGELOG_MASTER_VER021.md`. Existing branch archives are unverified snapshots; latest workflow #880 succeeded, but no target build/LVGL Pro/hardware test is validated.
