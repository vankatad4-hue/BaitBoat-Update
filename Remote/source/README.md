# Remote Source — Current Development Status

Updated 2026-11-20. The GitHub repository is canonical; this source is a development scaffold, not a tested release.

## Versioning
- ZIP: `Flytec_Remote_verXXX.zip`.
- Preserve older archives; current marker `releases/VERSION` is `026`.
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
- `boat_ui.*`, `settings_ui.*`: UI scaffold; map and sonar displays are not operational data views.
- `offline_map.*`, `map_projection.*`: tile validation/read and projection utilities, not a complete geographic map renderer.
- `boat_telemetry.*`: partial telemetry state application with local group freshness, coordinate sample pairing and primary-link safety provenance; wired to Remote receive dispatch.
- `radio_payload_codec.*`: strict eight-byte HEARTBEAT/ACK/TELEMETRY payload validators; integrated in authenticated receive dispatch.
- `radio_payload_encoder.*`: portable payload encoders, host round-trip tests, mirrored on Boat.
- `radio_ack_tracker.*`: bounded command lifecycle tracker (waiting/accepted/executed/rejected/timeout); wired to non-manual command send and authenticated ACK receive.

## Bench test sketches
- `hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `hardware_test/E22_SC16IS750_Bench/E22_SC16IS750_Bench.ino`
- Fixed diagnostic probes only, CRC checked, no authenticated commands or actuator payloads. See `hardware_test/README.md`.
- Bench tests remain uncompiled/unverified.

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

- Hardware verification checklist: `Remote/docs/MISSION_NVS_TEST_PLAN_VER026.md` (not yet run).
