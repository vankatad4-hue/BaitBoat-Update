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

### CI follow-up findings
- First target compile attempt stopped before compiling source due to Arduino CLI sketch naming requirement. Workflow now stages a correctly named temporary sketch folder and captures a compiler log artifact.
- Release-guard test's first failure was a test-only BoatState struct-layout mismatch; fixed to compile/link the real BoatState implementation.
- The following codec test failed because its invalid latitude test data encoded approximately 50°, not 91°. Corrected to 910,000,000 E7. CI retry pending.

### CI state update
- Remote host CI run #7 is **passed** (host-only): release-guard/scanner syntax, payload codec, ACK tracker, telemetry freshness.
- First target compile reached the actual Arduino sketch compiler but stopped on missing LVGL `lv_conf.h`; config file and workflow installation step added. The next target run is in progress; full firmware compilation remains unverified.

- Target compile attempt #4 progressed past LVGL config setup and failed at missing `RF24.h`. RF24 1.4.11 was added to workflow dependencies; compile retry pending.

- Display flush callback was hardened to avoid signaling LVGL through a null display pointer. Compile verification is still pending.

- LVGL 9.6 target compile revealed and fixed: precise line-point API type mismatch in the sonar demo plot and missing live footer-label pointer declarations. Compile retry pending.


## Latest target-build continuation — 2026-11-20 (run #11)

- Branch head before the fixes below: `cd36927c5051d3f7b72a65c125e6f2753520f1fe`. Remote target compile workflow run #11 failed during C++ compilation. This is the first compile that reached multiple project source files after the earlier setup/dependency fixes.
- Resolved tool/library versions observed in compiler output: Arduino ESP32 core 3.0.2; LVGL 9.6.0; ESP32_Display_Panel 1.0.0; ESP32_IO_Expander 1.0.1; esp-lib-utils 0.1.2; RF24 1.4.11. This confirms the workflow installed these versions; it does not confirm their compatibility.
- Compiler diagnostics: (1) joystick calibration event callback used `lv_event_get_target()` returning `void*`, now switched to LVGL9 `lv_event_get_target_obj()`; (2) map route point array used `lv_point_t`, now changed to `lv_point_precise_t`; (3) Map Manager chunk-copy helper was accidentally hidden inside a comment because a literal escaped newline followed its comment; corrected to a real newline so `copyChunk()` is declared.
- Fix commits: `1afd17d549ec49ef3e1fd206778277aab927cbb2`, `008400878dd007ffbacc96d717c81c81c4819d28`, `7213f04e4d18f3bc4a546f4c4febfe0bbedffc59`. A new target build should run on the updated branch; result is pending.
- Host CI run #11 passed on the prior source SHA `cd36927c`. The host workflow is not automatically evidence for the new fixes, though the fixes are primarily LVGL-target source/API and build-only.
- Next resume: check branch HEAD and target CI result; inspect compiler logs for remaining diagnostics; fix actual errors only; repeat until compile/link succeeds. Then audit warnings and resolved dependency inventory. Physical LCD/touch/SD bring-up remains not performed.
- The release markers remain Remote 028 / Boat 014. No release ZIP or SHA was generated for these development fixes. Current branch ZIP snapshots remain unverified.


### Rebuild status after corrective fixes
- Target compile run #14 (GitHub Actions ID 44776348680), head `7213f04e4d18f3bc4a546f4c4febfe0bbedffc59`, is currently **in progress**; dependency-install step is active at last check. It is the first build including all three corrective source changes.
- Remote portable host-test run #14 on the same head completed **successfully**. This only validates the portable host suite; the LVGL target compile is still pending.


### Documentation synchronization HEAD
- Branch HEAD after checkpoint/status synchronization: `04d9a9ff59ac10894a80951b2d5a548f66f9f22e`. No firmware source changes occurred after target compile run #14 started; it compiles source SHA `7213f04e4d18f3bc4a546f4c4febfe0bbedffc59`.


## Target Compile #14 → #15 checkpoint (2026-11-20)
- Target Compile #14 failed during C++ compilation in `Remote/source/offline_map.cpp`: `g_map_attribution` was referenced by manifest validation, initialization, and attribution retrieval but had no declaration.
- Corrected by adding a module-private `static char g_map_attribution[256] = {0};` alongside the other file-scope state. This preserves the existing copy/truncation handling and avoids exporting a symbol.
- Fix commit: `9aa20b0d987f94ea5b49e46509ae0ba10cc9d36b`.
- Compile #14 run: `44776348680`; outcome: failed. Target Compile #15 is requested after this fix; result is pending until the workflow run completes. Do not claim successful compile/link until verified.


## Current verified status — target compile + workflow correction (2026-11-20)

**Supersedes stale compile statements above:** Remote target compile run #19, GitHub Actions ID `44781879101`, succeeded on commit `3f189cb1ac65a764aec678893f1df857d113c815`. The target Arduino build compiled and linked successfully; firmware artifact upload succeeded. This is CI compile/link only—not board bring-up, RF, SD, touchscreen, or operational validation.

- Arduino-ESP32 core 3.0.2; LVGL 9.6.0; ESP32_Display_Panel 1.0.0; ESP32_IO_Expander 1.0.1; esp-lib-utils 0.1.2; RF24 1.4.11.
- FQBN configured for ESP32-S3 16MB Flash, OPI PSRAM and `PartitionScheme=huge_app` after correcting the default app partition size.
- Sketch: 1,430,957 bytes (45% of 3,145,728 bytes app space). Static/global RAM: 178,136 bytes (54% of 327,680), leaving 149,544 bytes.
- Compiler errors: 0; C++ warnings reported: 0. Non-blocking tooling warnings: esptool Python SyntaxWarning and GitHub Actions Node20 deprecation warnings.
- Target workflow: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44781879101
- Host tests run `44779439310` passed; source SHA `2289e84ad73e1931f18f357d0e437fed5e0d6c12`. Package workflow `44779439396` passed for that source SHA. Latest commit only changed target build workflow; no firmware source changed after those test/package runs.
- Build partition fix: `3f189cb1ac65a764aec678893f1df857d113c815`, workflow now exports firmware binaries and uses huge_app.

## Immediate next work
1. Runtime/lifecycle audit: Wi-Fi scan/connect timers and cancellation, screen teardown/re-entry, LVGL timer ownership.
2. Offline map import/index parsing: strict bounds, malformed/duplicate rows, zoom/tile coordinate validation, file-copy integrity, overflow-safe storage estimates, partial-import recovery.
3. Map renderer: cache/PSRAM exhaustion, tile descriptor stride/format, file lifecycle, allocation failure, viewport teardown/re-entry.
4. Fix confirmed defects; run target compile and host tests again, then synchronize development notes. No marker bump or ZIP release unless package source is intentionally frozen.
5. After software review, physical tests still gate LCD/touch, SD, Wi-Fi, controls, radio and safety.


## Handoff pointer refresh
Latest development changelog is `Remote/releases/CHANGELOG_MASTER_VER023.md` (VER023 development update; release marker still 028). Target compile #19 passed on 3f189cb1ac65a764aec678893f1df857d113c815. Latest branch head after documentation sync is to be checked on resume. Packaging workflow run #953 is triggered by the final documentation commit and was still running at last check; verify its outcome. Package workflow success is not a ZIP integrity/release claim.


## Eight-lake development checkpoint — 2026-11-20
Remote package marker remains 028; Boat 014. Source now has eight SD lake profiles, sixteen mission slots and separate Home per profile, checked active selection, recoverable name and mission records, and A/B offline map installation. Lake selection UI uses standard LVGL keyboard and confirmed STOP/manual/connected-radio/inactive-motor gates. Mission execution remains blocked; boat-side synchronization is not implemented.
Latest observed before this update: host #41 passed (6a3d88c), packaging #977 passed, target #42 running. Host #41 did not exercise the new SD storage. This update adds a mock SD host test compiling actual lake_storage.cpp: eight profiles, safe switching, name persistence, isolated paths, map marker rename failure, short writes and unavailable card. Results pending. The mock does not test actual FAT/SD durability, mission serialization or LVGL. Verify new CI and fix real failures before bumping the release marker. Hardware remains untested.


## Stabilization audit continuation — 2026-11-20
- Host #42 verified passed all five groups on 6a05b2d, including the actual lake_storage.cpp mocked SD test; ZIP packaging #978 passed. Target #43 was still running at review. Prior successful full target compile #19 is historical, not evidence for lake changes.
- Audited lake, mission, map reader/import/render, UI, startup and safety interfaces. Added mission_storage.cpp/.h (same legacy record layout), actual 128-slot isolated mission serialization/CRC/failure tests, record validation and mission-screen async deletion. Fixed stale coverage stats and protected A/B fallback marker before overwriting inactive map slot; copied files now reopened for size verification.
- Next: observe new host and target build, correct actual errors, audit remaining Boat/Remote modules (full branch-wide review is not complete); run map importer/coverage parser fault tests and hardware checks. Do not enable autonomous movement or claim release readiness. No version bump until build/test gates pass.


## Verified test handoff — 2026-11-20
Source SHA 8030ecab6cbccb64204e638b4c398b588681515e. Host #43 failed on missing sdMissionAvailable reference after extraction; corrective commit 8030eca replaced it with lakeStorage_ready. Host #44 passed (run 44793207359), including actual mission serialization for 128 waypoints, per-lake Home and CRC/rename/short-write simulation. Packaging #980 passed (44793207717). Target compile #45 (44793207383) is running; previous #43/#44 were cancelled by newer pushes, not passed. Do not change firmware merely to retrigger CI while the current target run is active.
Next: inspect #45 compiler result and fix actual diagnostic if any; extend coverage/import parser and fault-injection tests, audit remaining source modules in manageable batches (repository inspection is not an exhaustive line-by-line audit), validate UI lifecycle and SD runtime, then bump Remote package marker 028 to 029 only after software gates pass. Boat remains 014 and hardware/actuation unvalidated. Repository project marker docs/PROJECT.md is not a ChatGPT Project. Existing root legacy map and NVS-only legacy data require follow-up; standard keyboard is not a dedicated Bulgarian keyboard.


## User-approved eleven-stage completion roadmap — 2026-11-20
This is the controlling work sequence. Record exact source SHA, actual CI evidence, corrective commits and remaining blockers after every stage; do not repeat completed work. Hardware dependent gates remain separate. Remote version marker 028, Boat 014.
1. Finish target compilation and branch code audit: inspect actual errors, correct and obtain successful full target build; audit source in batches.
2. Map parser/recovery/runtime: test actual manifest and coverage parser, map import interrupted-write handling, stale coverage, rendering cache and timing; distinguish host simulation from real SD.
3. Physical buttons: wire all assigned actions to shared UI/controller operations with STOP/release priority and no duplicate actions; test debouncing and screen re-entry.
4. Full mission protocol: validated lake ID, waypoint and Home coordinates, independent release mask, transaction ID, prepare/commit acknowledgement, duplicate/replay handling and explicit cancel; keep transport disabled until both sides support it.
5. Autopilot management and feedback: authenticated mission state, start/pause/resume/cancel, 3 m requested arrival radius with accuracy gate, manual override policies, safe return/stop; simulation before any motor enable.
6. Radio authentication/diagnostics: secure per-pair key provisioning and recovery, session/ACK/replay tests, dual-link status, no hardcoded production keys; physical wiring and RF tests separate.
7. Offline navigation: pan, recenter, boat-up/north-up, style/zoom preference per lake, tile/cache/PSRAM limits, authorized offline pack import; optional online Google provider only with authorized credentials.
8. Settings/Bulgarian UI: consistent Bulgarian text/glyphs and Cyrillic keyboard, durable preferences, navigation and lifecycle tests, diagnostics.
9. Verify and package Remote VER029: full target and portable tests on chosen source, package/checksum and archive verification before release bump. Do not call the existing development snapshots released firmware.
10. Physical hardware and Boat integration: blocked pending real Waveshare unit, boat electronics, verified pinout, bench power/actuator isolation, boat target compile, telemetry and command executor, range and STOP/failsafe validation.
11. Real CHIRP sonar: blocked pending selected module and documented raw/profile data interface; implement and validate actual scrolling echogram with fish arches, depth, range/gain controls on the one 4.3-inch display. No fabricated sonar data.
Latest checked: target compile #45 failed (44793207383) in settings_ui.cpp line 31: literal backslash-n sequences before static declarations. Host #44 passed, packaging #981 passed. Current task starts with correcting this failure. 

### Stage 1–2 implementation update
Fixed target #45 escaped duplicate declarations in settings_ui.cpp (2db6318). Added shared map manifest parser used by importer and installed pack reader, count validation, bounded lines, fail-closed coverage stats, polar row clamp and dedicated actual parser tests. Full runtime, target and physical recovery are still pending. Continue from new CI; don't claim stage 1/2 complete until acceptance gates pass.

### Stage 3 input integration
Physical waypoint save/select and screen actions now share existing UI operations; BACK respects calibration exit/import guards, status screen read-only, ordinary OK only closes the waypoint screen; protected release confirmation consumes its own OK event. Input bus failure requests STOP and inhibits held keys until neutral; simultaneous release/STOP cancels release guard. Remaining: scanner hardware and comprehensive debounce/chord/OK integration tests, status-screen localization. Existing autopilot/RTH requests are not evidence of implemented missions. Host #46 map parser tests passed; target build #47 was running at audit. The next source push may supersede it.
