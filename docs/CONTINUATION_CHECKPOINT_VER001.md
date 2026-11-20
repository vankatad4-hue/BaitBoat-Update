# Flytec V080 Retrofit — Continuation Checkpoint VER001

**Checkpoint date:** 2026-11-20  
**Canonical repository:** https://github.com/vankatad4-hue/BaitBoat-Update  
**Branch:** `hardware-ready-dual-radio-r1`  
**Purpose:** A complete restart point for future ChatGPT/project work. Read this first, then the linked status records before editing.

## 1. Overall status / honest readiness
This is an active design and software scaffold, **not hardware-ready, not compiled as a complete target firmware, and not operational**. No motors or solenoids may be powered/connected for operation. Exact controller/display revisions, wiring/pin maps, secure key provisioning, Boat application executor, failsafe, native schematics/PCB, and validation are still gates.

## 2. User-selected architecture (do not reopen without reason)
- Keep Flytec V080 hull, existing two 12V 550 motors, main switch and battery indicator; replace control electronics.
- Boat carrier concept: 77×79×14mm envelope (preliminary), ~30mm lid clearance. Removable/keyed connectors; separate high-current motor/solenoid paths from low-voltage logic/sensor paths. DHT11 excluded.
- Battery reported 3S4P Li-ion / 20A BMS; verify actual pack/cells/wiring.
- Solenoids: bait FW0630 009 DC12V, hook FW0630 001 DC12V; electrical/thermal characteristics and driver protection still need measurement.
- Boat controller candidate Heltec Wireless Tracker ESP32-S3 + UC6580 GNSS; exact revision/pins not confirmed.
- Remote: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6; not yet bought. Existing handheld enclosure concept is being internally redesigned; R5/R6 CAD are draft only.
- Remote controls: two sticks; two 2×4 physical button banks (independent bait/hook buttons); separate OK and front power; dual external antennas (2.4GHz nRF and EU868 E22); no rear/shoulder buttons.
- Primary radio nRF24L01+ PA/LNA, initial 250kbps; backup EBYTE E22-900T22S UART LoRa. 500m is a target only.
- LoRa is duty-cycle-limited and safety-restricted; no bait/hook release or mission-start via backup. EU/Bulgaria RF compliance/airtime still needs verification.
- Mission intent: manually drive, save/select waypoint (up to 16), autopilot, planned bait/hook actions, return home. During autopilot manual override selectable. Link-loss return only when GNSS/heading/battery/geofence health are valid; otherwise safe stop.
- Sonar target: CHIRP-style echogram/fish arches on same 4.3-inch screen; currently only demo graphics, no sonar sensor/data processor.
- Maps: SD offline tile renderer/index and Map Manager scaffolds; optional Google online tiles via authorized API remains unimplemented. Google tiles must not be saved/offline. ONLINE currently unavailable; source preference is scaffolding only.

## 3. Latest repository snapshot
Recent branch commits, at checkpoint:
- `9f7b03440c` update master status R9
- `12bdeddfbe` update remote source README ver029.13
- `7d9a4e1f1c` supersede master changelog VER019
- `725b310736` consolidated changelog VER020 source cleanup
- `cff5d25cfe` clamp waypoint distance math
- `8eac84b49e` fix escaped newlines in remote UI source

Remote package marker is `028`; Boat marker is `014`. These are **package version markers**, not proof of verified firmware releases. Do not bump merely for documentation; before a deliverable ZIP release, update the relevant marker(s), check workflow, ZIP integrity, SHA256 and archive contents, then record evidence.

## 4. Current UI work (unreleased ver029 development)
- Main operation UI: Bulgarian labels, compact status header, map/sonar equal split, footer controls/manual/autopilot/return/mission/calibration/STOP, safety/joystick status.
- Map pane is still source-dependent: offline SD tiles; online Google option is not implemented. Map screen/renderer source is not hardware-tested.
- Sonar plot is responsive split/fullscreen demo with grid, illustrative arches and bottom trace; unmistakable DEMO/not-connected text. No real depth/fish data.
- Mission UI: 16 slots, Home, selected coordinates and GPS-derived distance/bearing only with fresh fix, save/select/clear, BAIT/HOOK plan flags. Start is safety-blocked; no boat executor.
- Calibration UI: endpoint/centre, deadzone, raw/normalized/status and neutral exit; ADC reader fails closed until hardware.
- Settings UI: localization pass in Bulgarian; still needs LVGL font/text-fit review.
- Offline Map Manager: GPS/manual center, 1km square, zoom14–19, estimate/coverage/pack/import progress scaffolds.
- SVG previews: `Remote/source/UI_PREVIEW_MAIN_VER029_9.svg`, `UI_PREVIEW_MISSION_VER029_9.svg`. These are source-created concept previews, not LVGL render screenshots.
- Checklist: `Remote/docs/UI_BUILD_VER029_9.md`.

## 5. Remote firmware scaffold
Transport: nRF24/E22, SC16IS750 bridge, bounded RX, CRC/HMAC framing and command codecs, link manager/failover, heartbeat, ACK tracker, authenticated receive dispatch and partial telemetry freshness/safety.
Input/UI: MCP23017 physical scanner/debounce; joystick ADS1115 reader/calibration/preferences; UI screens and fail-closed release guard.
Map: projection, SD XYZ RGB565 renderer, bounded cache, source policy and coverage index/pack validation.
**Not ready:** keys are unprovisioned and return false; authenticated TX/control disabled; exact board pin map unresolved; target compile and hardware checks pending. STOP UI is not a validated safety system.

## 6. Boat firmware scaffold
nRF/E22 transports, authenticated RX/queue, duplicate cache, portable payload encoder/decoder, session echo, per-radio sequences and queue-level ACCEPTED/DUPLICATE/BUSY ACK. No main application integration, command executor, actuator output, telemetry sensor source, EXECUTED ACK, GNSS/IMU driver, mission/autopilot, return-home or watchdog execution. Boat GPIO macros remain disabled/TBD pending exact board revision and pin audit.

## 7. Electrical/mechanical state
Remote wiring, capture sheets, pin assignments, power drafts/BOM candidates, connector CSV and CAD exist as documents/drafts. Native KiCad schematic/PCB/ERC/DRC not complete. R6 handheld CAD is not physically fitted/validated.
Boat Cytron MDD20A candidate outline (88.9×78.74mm) exceeds preliminary 77×79mm carrier area; separate placement/mechanical revision required. Motor startup/stall current and solenoid coil data are missing. Battery/charger/converter thermal validation missing.

## 8. Release/archive workflow correction
Old workflow regenerated ZIPs under unchanged version names on development pushes. Guard now commits versioned archives only when a VERSION marker changes; routine commits produce workflow artifacts. Workflow run #880 succeeded after the guard was added (head 9f7b0344). The documentation synchronization triggered runs #888 and #889; both completed successfully (#888 at c11f9048c1, #889 at e68257eff1). Run #889 is the latest observed success. The branch's existing ver028/ver014 ZIPs have not been restored from an earlier immutable baseline and **must be treated as unverified snapshots**. No release integrity claim.
Latest workflow:
https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44748284171

## 9. Next tasks — ordered
### A. Source consistency/compile preparation (audit R1)
- Audit findings recorded in `docs/SOURCE_CONSISTENCY_AUDIT_R1.md`.
- Remote/Boat session-start retry correction committed; not integration-tested.
- Establish pinned build configuration and exact board/dependency revisions; add Remote CI host tests; add Boat dual-radio integration tests; verify session retry behavior.
- No target compile or complete host test execution has been claimed.

### B. Target compilation
- After board/dependency selection and build manifest, compile all Remote and Boat source modules, resolve diagnostics and retain logs.
- Confirm LVGL 9.6 API compatibility, display driver calls, board macros, SPI/I2C/UART conflicts and source inclusion.

### C. LVGL Pro UI verification (user will test later)
- Load 800×480 / LVGL 9.6 profile, inspect all screens and touch actions.
- Validate Cyrillic glyphs, font, clipping, ≥40px practical touch targets, child event propagation, split/fullscreen, repeated screen lifecycle, heap/PSRAM and timer cleanup.
- Capture screenshots and a defect list; fix before release.

### D. Protocol and safety tests
- Fake-radio integration tests for session markers, stale epoch, cross-radio duplicate IDs, queue full BUSY, ACK envelope/HMAC/CRC and queue ACCEPTED versus EXECUTED.
- Challenge/response secure session, persistent replay prevention, key provisioning review.
- Boat application state, watchdog, STOP/clear-STOP and failsafe truth table; keep outputs abstract/fail-closed.
- Add telemetry source/freshness; connect ACK lifecycle to application results.

### E. Hardware/electrical
Identify exact Waveshare and Heltec revisions; audit pin/resource conflicts and continuity. Measure battery, motor stall/start current and solenoid coil electrical/thermal data. Complete schematic/netlist/footprints/ERC, power/connector/fuse/thermal review, PCB/DRC, fit check.

### F. Integration only after gates
Map live GNSS/boat state and mission execution; CHIRP sonar driver/data contract; authorized online map provider; HIL with dummy loads; independent failsafe review; controlled-water testing only after safety approval.

## 10. Source audit update — 2026-11-20
- Audit report: `docs/SOURCE_CONSISTENCY_AUDIT_R1.md`.
- Remote now retries authenticated session-start markers independently (nRF 500 ms, E22 3 s, duty-limited) until per-link confirmation.
- Boat responds to repeated same-session markers to recover lost replies.
- These fixes are source-only and remain uncompiled/unintegrated. Key provisioning is still disabled, so the path is fail-closed.
- Branch commits for this audit: Remote retry `849ceb09c16956ce6218006ee50314365663961a`; Boat duplicate response `636f7a9f74f59e4005e5bb5eb753deddedca196d`; audit document `2823e27496340638145838548e389bf69cb67551`.

## 11. Resume instructions
When continuing in a new chat:
1. Open this checkpoint and `Remote/docs/PROJECT_MASTER_STATUS.md`, `Boat/docs/PROJECT_MASTER_STATUS.md`, `docs/PROJECT_PROGRESS_REVIEW_R1.md`, `docs/ZIP_BUILD_STATUS_R1.md`, `Remote/releases/CHANGELOG_MASTER_VER022.md`.
2. Check current branch head and workflow status; don't assume the listed SHA/run is still latest.
3. Work on the next ordered task, inspect the exact source/docs first, commit updates to branch, and update the checkpoint/changelog/status.
4. Never say compiled/tested unless a successful build/test run or user-provided evidence exists.


## Documentation sync addendum
- Updated root README, Remote/Boat master status, project progress review, ZIP status, import status, release policy, source READMEs and historical ver005 status files to point to this checkpoint and align current markers.
- Consolidated changelog is VER021. Old VER020 was superseded.
- Last observed branch HEAD: `c11f9048c1` (historical build status notes). Packaging run #888 pending; run #880 previously successful.


## Compile-prep follow-up — Remote portable CI added
- Added `.github/workflows/remote-host-tests.yml` and documented its scope in `Remote/tests/README_release_guard.md`.
- Tests run under strict warnings: release guard, scanner syntax, payload codec, ACK tracker and telemetry freshness.
- Workflow result not yet observed. These tests are host-level only; they do not build the full LVGL/ESP32 firmware.
- Next: confirm CI result and fix; then build configuration/dependency pinning after exact hardware revisions are confirmed.


## Target build selection follow-up
- User confirmed the Remote target: **Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948** (not 4.3B or BOX).
- Hardware selection is fixed. Purchase/physical inspection remains outstanding.
- `docs/TARGET_BUILD_CONFIGURATION_R1.md` records the selection and dependency pinning rationale. Do not freeze the build manifest until official demo dependencies are reconciled with LVGL 9.6 and versions are verified.
- Boat candidate Heltec Wireless Tracker remains revision/pin unresolved.


## Waveshare target confirmed and compatibility note — 2026-11-20
- User confirmed Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948; selection is fixed. Physical verification/purchase remains outstanding.
- Official legacy Arduino examples use LVGL 8.x, while the official newer LVGL9 example path uses a BSP-based ESP-IDF example (reported dependency LVGL 9.5.0). This is a version/API split, not proof of incompatibility.
- Next: inspect Remote `display_port.cpp` and project LVGL config against the official LVGL9 BSP; verify 9.6 compatibility and then pin a reproducible dependency/build manifest. Do not arbitrarily downgrade LVGL or select legacy demo dependencies.

## Latest continuation update — 2026-11-20

- **Display target confirmed by user:** Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948. Do not switch to 4.3B or BOX.
- LVGL 9.6 display-adapter source audit: `Remote/docs/LVGL_9_6_DISPLAY_ADAPTER_AUDIT_R1.md`.
- Candidate build pins/config: `Remote/docs/TARGET_BUILD_CONFIGURATION_R1.md`.
- Target build workflow: `.github/workflows/remote-target-build.yml`. Its first GitHub Actions run was observed in progress at checkpoint; do not state compile success until checked.
- Remote host-test CI run failed at release-guard unit-test fixture; the fixture was corrected in `Remote/tests/release_guard_unit_test.cpp`, with rerun pending.
- Exact vendor API compatibility remains to be proven by target compilation: LCD drawBitmap return/completion semantics, Touch::readPoints API, board init/begin, LVGL display/input calls.
- Display adapter still requires physical verification of orientation, refresh/tearing, SD and backlight. Keep boat actuators and radios disconnected.
