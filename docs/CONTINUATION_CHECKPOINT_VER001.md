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

### Stage 4–5 portable foundation (not live transport)
Added identical Remote/Boat eight-byte mission payload codecs, validated 8 lake IDs / 16 waypoints / four independent release bits / 3–10m radius, signed E7 target/home coordinates, transaction assembly, CRC commit, expiry, duplicate and cancel handling. Candidate receiver requires caller-verified auth/session/replay and stopped state. Added non-actuating Remote feedback lifecycle with guarded transitions, timeout and STOP-required status. Portable test covers codec and lifecycle. These modules are NOT registered with live radio dispatch or UI; no mission starts. Explicitly blocked non-manual mode commands until live synchronized mission transport and safety gates are available. Actual protocol integration, feedback UI, override behavior, key provisioning, Boat executor remain open.

### Stage 6–8 partial implementation
Boat codec no longer compares unsynchronized sender/receiver uptime; host test deliberately returns false from the obsolete freshness stub and checks local receipt timestamp. Auth provisioning remains fail-closed; session replay/secure provisioning require separate review. Added read-only Remote diagnostics (RAM/PSRAM, SD/lake, radio links, ACK, STOP, key presence), settings back timer cleanup; map touch pan, tap zoom caption to recenter, per-lake backed-up zoom, correct panned route origin, 4x3 tile cache to cover full width, image descriptor magic/stride and retry cooldown. Fixed demo sonar point-array lifetime. Added map projection host tests. Remaining boat-up rotation, asynchronous tile reads, physical checks, per-lake remaining preferences, complete Cyrillic font/keyboard support. No release marker bump. Prior source 81dd926 passed Remote host #48 (8 groups), Boat host #14 and packaging #985; target #49 was running before this commit.

### Additional regression tests
Added real scanner + release guard simulation using mock I2C registers, OK ownership, simultaneous release keys, STOP and I2C-failure rearming; extended lake storage tests for independent zoom preferences and failed promotion rollback. Pending new host CI. No firmware changes in this test-only commit; target build should remain active.


## Eleven-stage execution handoff — 2026-11-20
**Current firmware source:** 3d363f07c84c24cb12c5ecd7be2c52b5f15e8897; test-only follow-up 7ec7a7a5f313f8d330233d1941ca7ff4ff0ca6e5.
- Remote host #50: PASSED, all ten groups (44795330911). Includes actual map manifest/coverage parsers, projection/pan, 128-waypoint mission storage, matching portable Remote/Boat mission codecs and lifecycle, physical input/OK/release arbitration and lake zoom persistence. Boat portable #15 PASSED (44795188700). ZIP snapshot #986 PASSED (44795188825). Target #50 (44795188918) was still installing pinned ESP32 core; no full compile result yet. Do not push unnecessary firmware changes that cancel the target run.
- 1: #45 escaped duplicate Wi-Fi state declarations fixed (2db6318); complete latest target compilation and remainder of branch audit still open.
- 2: shared strict map manifest and coverage parser/count checks tested (bec5550); mock recovery passed, real SD and runtime/cache/import failure testing outstanding.
- 3: physical waypoint, settings, map, sonar, back and OK actions connected; scanner STOP/fault/release arbitration tested (70c7f9e, 7ec7a7a). Hardware and UI integration validation outstanding.
- 4: mirrored eight-byte candidate mission protocol/assembler (81dd926), host tests passed. Authenticated live dispatch, ACKs, transaction and release-mask UI integration still open.
- 5: non-actuating mission feedback state machine tested. No live autopilot, pause/resume/return execution, accuracy gating or override integration. Non-manual command requests explicitly blocked pending synchronization.
- 6: diagnostic screen and Boat unsynchronized-clock bug corrected (3d363f0). Secure key provisioning, authenticated session/replay redesign and radio bench testing open; key store deliberately fails closed. Boat command execution must enforce local queue age.
- 7: north-up touch pan/recenter, per-lake SD zoom, panned route origin, enlarged 4x3 tile footprint, descriptor fixes and retry cooldown implemented. Boat-up rotation, async tile loads, full preferences and hardware display tests open.
- 8: diagnostics, settings back/timer cleanup, sonar demo point lifetime corrected; Cyrillic font asset/keyboard, full localization and display layout validation still open.
- 9: Remote release remains 028; 029 NOT released. Boat remains 014. Snapshot workflow success is not release certification. Bump only after software gates.
- 10: physical unit not available; boat integration/actuator/range tests not performed.
- 11: sonar module/interface not selected or verified; demo only; no real driver or data.
Next action: read target #50 result/log and correct actual errors, then continue stages in the approved order. Recheck CI on exact new source and update this handoff; no claim that all eleven stages are complete. 


## Target Compile #50 — verified successful (2026-11-20)
Supersedes the pending #50 statements above. GitHub Actions run 44795188918, job 136492949945, completed compile/link and firmware/log uploads successfully. 1,435,681/3,145,728 bytes flash (45% reported); globals 178,808/327,680 bytes (54%), 148,872 bytes remaining. ESP32 core 3.0.2, LVGL 9.6.0, Display Panel 1.0.0, IO Expander 1.0.1, esp-lib-utils 0.1.2, RF24 1.4.11. No C++ error/warning diagnostics found; esptool SyntaxWarning and Node/Actions deprecations are nonblocking. Host #50 (44795330911) verified all ten groups successful; this is a previously completed run, not a fresh rerun. Packaging #986 (44795188825) emitted Remote 028 and Boat 014 snapshot artifacts for 3d363f07c84c24cb12c5ecd7be2c52b5f15e8897, plus checksum artifact; archive contents not independently verified. Repository has subsequent mechanical work; do not assume these runs cover every subsequent commit. No release marker increment or actuation enable. Next: compare current branch to tested source, finish code audit and extend map import interruption/cache/runtime tests; re-run CI for any source fixes. Keep mission dispatch and actuator outputs fail-closed. Physical validation and real sonar remain outstanding.


## Stage 2 follow-up — bounded import index
Commit 2ec3a0a5b54fd0114157ed5306ce876207b38903 bounds staged coverage index size before synchronous parsing (including blank lines), rechecks index/manifest sizes before copying and cleans up on rejection. Fresh target, host and snapshot CI required; results not yet verified. Next: observe new CI, add oversized/blank-line importer regression and continue interruption/cache/runtime audit. Remote 028 / Boat 014 unchanged.


## Stage 2 installed-index and initial-publication regression — 2026-11-20
- abd61e48c48dc0eef00c3ab643a101a7212d4de9: installed coverage reader now rejects indexes larger than (6000+1)*48 bytes before allocation/parsing.
- 0425ee14974a3edb456c8d2b1d5b108b43a389d5: host test oversized blank-line padding, invalidation of prior coverage, recovery on valid reload.
- 3d4048c0349594bb9e52c75cf4d221676a21598e: host test initial A/B map publication short write and failed rename followed by reboot; prior implicit slot must survive.
- New test and build outcomes not observed. Commit workflow lookup returned no runs (connector only lists PR-triggered runs); do not infer push CI outcome. Next: verify actual push CI in Actions, add integrated importer copy/validation fault tests and renderer lifecycle tests, then proceed with remaining stages. Target #50 remains historical last verified target. Remote 028 / Boat 014 unchanged; no ZIP regeneration or hardware claim.


## Stage 2 renderer lifecycle and copy bounds follow-up
- a59c8d421619807918972abb3c17115f66ecc563: guard duplicate viewport creation and updates after deletion; detach LVGL image sources/drop cache entries before freeing tile memory; release tiles when map unavailable or viewport slots out of bounds. Target compilation and LVGL deletion-order/cache behavior require verification.
- 9b212be2cf5c1cdefa87728b2dad2164731b38f4: bound source size at copy open, reject chunk exceeding expected file or total pack bytes, require exact total before staged map publication. Integrated copy fault tests remain outstanding; exact byte count does not prove content integrity.
- Fresh CI results unavailable via PR-only lookup; local clone blocked by DNS/network. No new test pass or archive synchronization claimed. Next: verify push CI; extract/test copy state machine, exercise viewport create/delete/recreate under LVGL, then cooperative tile reading with bus ownership and cancellation. Remote package 028 / Boat 014 unchanged.


## Cooperative SD reads and map-copy regression continuation
- 546b550 / 836ab34 / a648892 implement a single cooperative tile reader, 4096-byte reads per renderer update, cancellation on obsolete viewport/zoom/style/profile or catalogue reset, no publication of partial buffers. Actual SD latency and LVGL lifecycle remain unverified; synchronous compatibility API retained.
- 4e33cd0 / 50c5df1 / 4c1b4f2 extract actual map-copy logic into map_copy.h/.cpp and connect Map Manager; 6468090 adds success, short-write, bounds, truncate and cancellation tests; 7580110 registers eleventh host group. Results pending; push CI lookup returned no runs through PR-only connector. #50 remains last verified target, not evidence for new source.
- Reviewed physical scanner and mission interfaces: independent actuator requests remain guarded, nonmanual mode explicitly refused, portable mission assembler/lifecycle not yet wired to authenticated dispatch. Next: verify push target/11 host groups, address diagnostics, then complete physical input integration and authenticated non-actuating mission preparation/feedback before enabling real execution. No package marker bump or ZIP sync.


## Physical input safety and mission sender continuation
- c53b95d / 18bf1bd: manual motor control requires fresh neutral rearming after invalid inputs, safety/link inhibition or failed send, confirmed manual mode and actuator readiness. Temporary autopilot override stays disabled until acknowledged Boat-side transitions exist. 1027027 / 62d9972 inhibit manual releases in nonmanual/failsafe states at both guard and command boundary.
- a165c5f / 1c63879: held-key startup requires STOP and all inputs released, with mocked startup regression; 6f8ce63 adds release inhibition tests. Joystick transport/runtime tests and actual hardware remain outstanding.
- 0811811 / 2292f18 add portable mission upload sender with exact transaction/opcode ACK matching, bounded retry/timeout, safety inhibition and fake receiver integration tests. Not attached to live radio or mission UI; no autonomous movement enabled. Next: verify latest target and all 11 CI groups, then define matching authenticated wire message/ACK dispatch on both peers, enforce session/replay and stopped-state gates, connect mission UI. No packaging bump or ZIP sync; last verified target #50. Push CI lookup remains limited to PR runs.


## Mission preparation transport and navigation continuation — 2026-11-20
- Remote/Boat mirrored `mission_wire.h` and `radio_protocol.h` define dedicated mission PREPARE (8) and ACK (9) messages keeping 32-byte authenticated frames. Boat `boat_dual_radio_port.*` dispatches only primary-link session/replay-checked preparation to a non-actuating assembler with application-owned stationary/STOP gate defaulting to null/deny; Remote `boat_radio.*` drives the upload and accepts matching authenticated ACKs. Both key stores still intentionally deny all TX. Existing 8-bit session epoch is not adequate final replay protection; no secure pairing or hardware certification.
- `mission_ui.cpp` now has preparation-only PLAN action with independent per-upload four-channel selection (all off by default; legacy marks are not mapped), preparation feedback, confirmation, local cancel and edit guards. Prepared plans expire independently; STOP invalidates local/Boat preparation. No mission START or actuator execution enabled. Relevant commits 763221c, 9cb3628, 68cb549, feb84db, f4724be, 6a1ded4.
- Verified locally using strict g++ flags and exact Git blob SHA matches: Remote mission protocol/upload/lifecycle/ACK regression PASS (test 4cdcc547669b7172bf52f7682fd3ede571bcff9a); Release guard PASS (test a2a6662931d0330bc84d1db0402a389d60b994f6); Physical scanner test initially failed at restart OK count because earlier ordinary OK was counted, fixed test baseline f62926985e903a64d8074e370918431804992969 and reran successfully (test blob 712e24efa6615ca190f0fc9472e2ff278f051b77). These are portable tests, not full CI or ESP32 tests.
- Boat `mission_navigation.h` adds pure distance/bearing/heading error and conservative arrival criterion distance + estimated horizontal accuracy <= requested radius (3 m default), requires fresh fix/healthy heading/battery/geofence. It is not a steering controller. Host test registered in Boat CI (6b8b4a3); local C++11 strict run PASSED on exact blobs a260353a7afbdff84e2a9f94ea10024399a45b73 (navigation) and f2004291e2d4119fe244ea24f92b1b1c63b97215 (test), mirrored codec blobs d0077f6727f9ec975d5c2fe95927bab4879999a4 and 651ea8faf239fd41e19f3f3b09668b94be2aae4f.
- Next: retrieve latest push-triggered target compile and full Remote/Boat host CI results, fix diagnostics before more target integration. Local `git ls-remote` fails DNS and Arduino CLI unavailable. No fresh full target evidence (last verified #50); remaining map-copy/loader and LVGL lifecycle tests, authenticated session redesign, Boat application supervisor, mission execution feedback and actual navigation/actuation remain open. Remote 028 / Boat 014 unchanged. No archive regeneration verified. No user design decision required for this software work; build evidence is the immediate blocker.


## Workflow audit and development continuation — 2026-11-20
- Repository-wide Actions run listing now accessible through authenticated GET. Target #74 (44801485451) failed at missing lv_image_cache_drop declaration; commit 0f4c47b added upstream 9.6 header but #75 (44802976345) failed due to Arduino include root already being lvgl/src. Commit 692f959 fixes to <misc/cache/instance/lv_image_cache.h>; #76 (44803266207) running at time of audit. Never treat previous #50 as current.
- Remote host #90 (44803265844, job 136520557058) passed all 12 groups including production SD tile reader/cancellation/short-read/missing-card tests and mission, release, map copy/recovery. Boat host #26 (44803347880, job 136520835967) passed all 8 including new simulation-only execution supervisor. Boat #29 (44803586651, job 136521642320) passed all 9 including actual OpenSSL HMAC domain-separated session transcript and bounded replay counter tests. No physical actuator integration. Session V2 candidate lives in Boat/source/radio_session_v2.h, not integrated; key store remains fail-closed.
- Corrected packaging workflow 74700d2: event SHA checkout, per-side exact +1 checks, immutable existing ZIPs, no rebase onto newer branch; #1044 (44803056623) passed normal snapshot build, and #1048 (44803265820) succeeded with SHA-specific artifacts. Release-publication path not exercised; no bump 028/014. Archive contents/inner checksums not independently downloaded.
- Added separate Bulgarian UI resource generation workflow and script using DejaVu Sans and pinned lv_font_conv 1.5.3. Generated resource not yet linked to LVGL or keyboard and workflow result unverified. Existing default Montserrat font does not cover Cyrillic. Do not claim screen validation.
- Next: confirm #76, inspect new font workflow, integrate tested session handshake and font into both target applications; complete real Boat supervisor/feedback, joystick/UI runtime, SD performance, release verification. Hardware, pinout, secure physical key provisioning and real CHIRP transducer remain external gates. Do not enable actual motor/solenoid execution or bump versions until acceptance criteria are met.


### Target #76 acceptance evidence (supersedes running status above)
- Run 44803266207, job 136520558870, source 692f959fcd76430d21e79c75ddeb183d8be8a8b4: PASSED. Program 1,441,429/3,145,728 bytes; globals 178,968/327,680 bytes; 148,712 bytes remaining. No C++ errors in retrieved logs; toolchain esptool SyntaxWarning and Node deprecation notices remain. Firmware artifact 14662810189 and compiler log artifact 14662560853 uploaded successfully. Remote host #90 and Boat #29 passed 12 and 9 groups respectively; Boat-only additions after source revision do not constitute a new Remote target build. This is compilation evidence only, not hardware certification. Version markers remain 028/014. Next integration gate: session V2 live transport/provisioning, actual Bulgarian font/keyboard integration, execution supervisor wiring and physical acceptance; archive publication remains gated.


### Additional verified workflow results
- Bulgarian font resource #1 (44803930186, job 136522841000): generation and source artifact upload PASSED; not linked or rendered on target. Offline Map Tool Tests #3 (44803930041) PASSED. ZIP snapshot #1053 (44803930029, source bc1ea6fff22e36b60a3e40870864762c0f65982f) PASSED archive creation, integrity tests and upload; artifacts Remote 14662426210, Boat 14662431183, checksum 14662371258. Artifact digests describe outer Actions ZIPs, not inner archive hashes. No release marker bump; immutable publication gate remains untested. Next: wire secure handshake into both endpoints and integrate generated Bulgarian font; verify actual execution supervisor and hardware gates before releasing 029/015.


## Session V2, Bulgarian UI and execution-control continuation — 2026-11-20

- 9ca1af5 integrates generated DejaVu Cyrillic font into the LVGL default theme in target CI, with Montserrat icon fallback, font artifact/license and EN/BG lower/BG upper keyboard layout. Local Arduino build requires generating/copying the font resource per Remote/docs/BULGARIAN_FONT_BUILD_R1.md. Resource workflow #2 (44805083427) and offline map tools #4 (44805083408) passed. Actual rendering/keyboard usability not hardware-tested.
- f249e76 implements mirrored mutual HELLO/CHALLENGE/PROOF/CONFIRM handshake with real OpenSSL HMAC host tests, replay/refused authentication and duplicate-proof confirmation checks. 67e2643 connects both actual adapters to derived session keys, rejects legacy handshake, uses bounded nonwrapping replay counters, resets mission/queue/ACK/telemetry at session loss or rekey, and stops queuing obsolete LoRa joystick commands. 8880fd2 authenticates the physical link flag, rejects copied frames on the wrong link, and adds flag regressions. Primary nRF24 is required to establish new sessions; backup shares an established session only. Secure provisioning reader is guarded by explicit opt-in, encrypted NVS, flash encryption and secure boot; normal target builds fail closed, the opt-in secure SDK path is untested, and no actual key or hardware entropy validation has occurred. See docs/SECURE_PAIR_PROVISIONING_R1.md.
- 0c31980 adds actual non-actuating Boat diagnostic sketch polling the radio and a separate Boat ESP32-S3 target build. Unassigned pins, no mission safety provider or actuator execution; queue acknowledgements are not execution. First Boat target #1 (44805621575) failed missing ManagedLink declaration; f9c6bde corrects it. Remote target #79 (44805520905) failed on LVGL keyboard control enum conversions; 8880fd2 corrects explicit casts. Superseded intermediate builds cancelled. Current Remote target #81 (44806126603) and Boat target #4 (44806126280) are running on 8880fd25b8df9fd84cd3549bc8f8f38d516df1b1; no target pass claimed for new changes yet. Last verified Remote target remains #76.
- c2b565d adds mirrored 8-byte mission-control and feedback codec plus portable BoatMissionController (stage, start, pause, resume, cancel, temporary override neutral confirmation, permanent manual request, duplicate request handling, expiry). Tests are simulation only; no authenticated radio dispatch, physical mode confirmation, manual override motor lease or release safety executor yet. Future live integration must distinguish prepared plan from execution, invalidate on STOP, require current trusted Boat inputs and hardware watchdog, and never infer acknowledged movement from a queued operation.
- Verified latest host CI on 8880fd2: Remote #95 (44806126364) passed 12 groups; Boat #35 (44806126309, job 136530479362) passed 11 groups including actual HMAC, handshake and mission control/override simulation. Packaging #1062 (44806126387) passed snapshot build/upload; archive publication not yet tested, inner hashes not independently checked. Remote 028 / Boat 014 retained; no release bump.
- Continuation: verify both target builds #81/#4 and fix any failures; after compile acceptance wire mission-control request/feedback to authenticated primary radio and UI with a default-deny Boat application provider, add trusted input/STOP and freshness checks and temporary override command lease across actual joystick/Boat executor, then package frozen 029/015 only after acceptance. Hardware and CHIRP remain pending. No physical provisioning or actuator enablement approved or performed.


## QWERTY Bulgarian UI, mission execution radio integration and override lease — 2026-11-20
- 5b638ac and 7cadc20 implement touch-adapted phonetic QWERTY Bulgarian keyboard shared by Settings and lake name editing; lower/upper rows cover all 30 letters, English and symbol/number modes retained. Offline tool tests #5 (44807388840, job 136534858451) passed all 8 including explicit keyboard row and glyph coverage assertions. Actual physical LVGL keyboard usability remains unverified; this is not a BDS compliance claim.
- 215a239 and 8c2377b implement authenticated primary-link mission control, periodic phase feedback and dedicated stick input messages, Remote request/ACK client (bounded retries retaining logical ID; missing ACK unknown), a receiver-local 200 ms override lease (nominal 50 ms Remote samples), neutral rearm before override and resume request after displacement then neutral; physical stationary/STOP and release interlocks still required on Boat. No actual motor output is wired. 7cadc20 adds mission overlay for START/PAUSE/RESUME/CANCEL/temporary override/manual/STOP/clear STOP and guards lake/waypoint changes. d229cb1 requires fresh matching primary telemetry before confirmed manual transition and retains local STOP until fresh confirmation; adds completion token checking and queue clearing at mission start. b1f4926 fixes successful upload handoff and duplicate request side effects/ACK after STOP. 1f0d585 stages Remote client header into canonical shared-wire test directory; cross-peer integration simulates request, lost ACK, override neutral, 200ms expiry and abort. No actual RF or actuator tests; no trusted mission input provider in diagnostic Boat sketch. The live session-reset return-home behavior remains an explicit unresolved requirement: session timeout/rekey currently aborts the supervisor, so prolonged link loss does not yet guarantee autonomous return. Do not enable execution before watchdog, sensor, propulsion, release, session-loss-return and mode telemetry integration are independently verified.
- Previous Remote target #81 (44806126603) cancelled by source update, not a pass; Boat #4 (44806126280) passed 320945/1310720 program bytes, 19968/327680 globals. Boat #5 (44806845960) passed; Boat #6 (44807388836, job 136534858206, d229cb1) passed 333569/1310720 program bytes, 20072/327680 globals. Latest Remote target #87 (44807620544) and Boat #7 (44807620473), both source b1f49260ab29f266d8e0e925209e253465cad162, remain in progress at checkpoint. #76 remains last verified Remote target. Superseded Remote builds cancelled by subsequent firmware changes. No newly observed compile errors in this batch.
- Remote host #101 (44807620465, job 136535625521) passed 13 test groups; Boat host #39 (44807676162, job 136535821192) passed 13 (including the revised cross-peer simulation and stick lease). #39 differs from firmware revision by test/workflow-only commit 1f0d585. ZIP snapshot #1069 (44807620683, source b1f4926) passed creation and upload: Remote artifact 14664446356, Boat 14664526045, checksum 14664511165. These are development snapshots; inner archive contents/checksums not independently downloaded. Remote 028/Boat 014 unchanged; no immutable 029/015 publication until acceptance. 
- Next: verify #87/#7 target completion and diagnostics, reconcile later builds, test secure pairing and real application provider/actuator watchdog with disconnected power, complete autonomous return on prolonged link loss and confirmed manual/mission status paths, verify LVGL/SD real runtime. Release acceptance not complete; no production key provisioning or hardware actuation approved or performed.

### Boat target #7 verification (supersedes running status above)
Boat diagnostic target #7 (44807620473, job 136535626232, firmware source b1f49260ab29f266d8e0e925209e253465cad162) PASSED; program 333597/1310720 bytes, globals 20072/327680 bytes, leaving 307608. Remote #87 still in progress at last check. Release version markers remain unchanged. No actuator hardware or live RF validation claimed.


## Autonomous return-home and local hardware safety bridge — 2026-11-20

- Remote target #87 (44807620544, job 136535790034, Remote firmware source b1f49260ab29f266d8e0e925209e253465cad162) PASSED, superseding #76. Program 1,457,365/3,145,728 bytes, globals 179,464/327,680 bytes, remaining 148,216. Later commits in this batch change Boat source only. No new Remote compiler errors observed. Remote host #101 passed 13 groups; map/keyboard tool #5 passed 8.
- 2452159 implements one-way local diversion to Home of an already-started mission, clears releases and keeps Home navigation authorization across session expiry/rekey without retaining the old keys, transport queues or stick leases. Old mission transaction stays local during detached return; new-session mission/control/stick traffic and ordinary actuator requests are rejected, authenticated/physical STOP takes precedence. Never starts a prepared-only mission. Invalid navigation/battery/geofence, physical STOP or an unconfirmed release pulse prevent continued movement. Full dual-link loss diverts, rekey during active mission also conservatively diverts to Home; no automatic resume of outbound. The return controller test is a software simulation of session expiry policy, not a live transport/driver test. Actual obstacle avoidance and path/geofence sensing remain hardware/algorithm acceptance items; a valid GPS fix alone cannot guarantee a safe water route.
- b239438 implements portable locally timestamped trusted sample validation, normalized differential navigation mixer candidate, 200 ms manual propulsion lease, one-at-a-time tokenized release with explicitly commissioned pulse durations. 21e0ee7 integrates the application bridge and a driver contract that requires independent electrical cutoff/watchdog; no physical driver registered in the default diagnostic sketch. Physical 100 ms output lease is a contract, not measured hardware behavior. Sample limits 100 ms safety, 2 s GNSS and 300 ms heading, and limits in tests are provisional engineering/fixture values, not commissioned coil parameters.
- 9627f72 adds actual-output feedback validity gates, requires a new local sample after deenergization before release completion, confirms manual transition only with stationary/idle feedback and emits authenticated measured status (including detached return failsafe) through the application bridge. Ordinary manual-release execution, full GNSS/heading/position telemetry generation and final board-specific input/output drivers are not yet implemented. No production key provisioning, eFuse changes or actual actuation performed.
- Boat host #40 (44808224304) passed 14 groups with new link-loss tests; #42 (44808489252, source 21e0ee7) passed 15 including watchdog/release tests. Boat target #10 (44808489093, job 136538752483, source 21e0ee7) PASSED: program 336,465/1,310,720 bytes; globals 20,216/327,680, remaining 307,464. Latest source 9627f726a9327b6e2a5ab0e13ecafc3617126766 triggers Boat target #11 (44808772228), host #43 (44808772480), ZIP #1076 (44808772344); pending at time of checkpoint. Last verified snapshot #1075 (44808489113) generated both development ZIPs. No version marker bump (028/014); immutable 029/015 publication not authorized by current acceptance gates.
- Acceptance remains incomplete: actual board design pinout/driver mapping, independent watchdog/STOP electrical verification, calibrated GNSS/compass/stationary detection, motor direction/current and all four coil pulse/idle measurements; actual secure pairing with keys and entropy on selected devices; Waveshare keyboard/touch/live SD timing, complete live telemetry and water trials. See Boat/docs/ACTUATOR_DRIVER_ACCEPTANCE_R1.md and docs/SECURE_PAIR_PROVISIONING_R1.md. No physical evidence available. Next: verify latest Boat target/host/package results and fix errors, complete manual release and live telemetry application paths, test bridge with a mock driver, then obtain exact physical hardware and perform disconnected-power/bench validation; keep releases gated until acceptance.

### Final host and archive evidence
Boat host #43 (44808772480, job 136539600288, firmware source 9627f726a9327b6e2a5ab0e13ecafc3617126766) PASSED all 16 groups including measured status confirmation and strictly newer post-pulse sample. ZIP #1076 (44808772344, job 136539600263, same source) PASSED snapshot creation and integrity/upload checks. Boat target #11 still running; last verified Boat build #10. Archive markers remain 028/014, and inner archive contents not independently downloaded. No hardware validation.


## Manual release, sensor telemetry and application integration — 2026-11-20

- Boat target #11 (44808772228, job 136539599468, source 9627f726a9327b6e2a5ab0e13ecafc3617126766) PASSED; program 338,069/1,310,720 bytes; globals 20,224/327,680 bytes; 307,456 remaining. No new compiler errors. Remote target #87 remains last verified Remote target (Remote source unchanged in this batch).
- 38fb447 integrates all four independent manual releases into the existing shared one-at-a-time pulse supervisor, checks stationary/idle/interlocks and primary authenticated link, prevents motor output during a pulse, sends terminal completion ACK only after post-pulse local idle confirmation, separates transport generation from cached actions, and rejects non-new manual release IDs after queue acceptance. A repeat completed ID may get a duplicate ACK without another pulse. Physical mechanical release is not proven by coil idle. No actual physical driver registered. dd66dda adds neutral rearming after release and STOP on a new session unless a detached return remains active, strengthens GNSS status validity gates.
- 38fb447 adds locally timestamped coherent battery/heading/speed and matching lat/lon pair telemetry from trusted driver samples, periodic status at 200 ms and sensor batch at 500 ms. Invalid/stale fields use unknown sentinels or omit coordinate pairs; no synthetic GPS coordinates are transmitted by diagnostic firmware. Actual GNSS/compass/battery board-specific acquisition remains unimplemented/unverified. Waypoint index currently unknown (255) in sensor batch. Real RF performance and remote status rendering require physical tests.
- Boat host #44 (44810506477) passed 17 groups; #45 (44810626275, job 136545934705, source dd66dda3ddc887c07da8863bb73ef38288c90c94) PASSED 18 groups, including new telemetry codec round trips and actual production application bridge compiled against a mocked radio and physical driver. Test covers all four releases, concurrent request denial, backup denial, old idle sample rejection, post-release neutral rearming, manual lease expiry, loss of primary during pulse, session rotation, telemetry and failed reads. It does not test live RF dispatch, electrical cutoff, actual mechanisms or water.
- Boat target #12 (44810506870) cancelled by later source update, not a failure/pass. Target #13 (44810626250) still running at checkpoint. ZIP #1080 (44810626184, job 136545935490) PASSED exact-source snapshot creation, unzip integrity and artifact upload; immutable publication skipped, no version bump. Inner archives not independently downloaded.
- Added docs/PHYSICAL_COMMISSIONING_R1.md, staged 18 physical acceptance checks and evidence form. All physical results NOT RUN. Existing release fixture timings and steering gains are NOT real coil/driver settings. Separate electrical watchdog and exact board driver implementation remain mandatory; pairing and touchscreen/SD hardware test, real geofence/water trial and CHIRP integration remain open.
- Next: verify Boat #13, fix errors if any; audit remaining actual hardware adapters and real release ACK timeout against measured coil pulse lengths; complete board-specific integration and physical acceptance with locally measured evidence. Do not claim operating readiness or publish official VER029/VER015 before acceptance. Current marker 028/014 retained.

### Final target verification
Boat target #13 (44810626250, job 136545998180, source dd66dda3ddc887c07da8863bb73ef38288c90c94) PASSED; program 339,993/1,310,720 bytes, global RAM 20,280/327,680 bytes, remaining 307,400. Firmware and compile log uploaded successfully. Remote firmware source unchanged since successful target #87. Both target builds and 18 Boat / 13 Remote host groups are verified for the current software revisions; physical commissioning remains NOT RUN, official releases remain 028/014. No physical readiness claim.


## Board-specific integration R4 — 2026-11-20
- Manufacturer public revision documentation reviewed: V1.2-specific authoritative schematic not located; V1.1 and V2 are not safe substitutes. Added conditional GPIO crosswalk and provisional companion I/O candidate with RP2040 GP assignments, sensor/actuator logical connector schedule and independent electrical watchdog design calculations in Boat/docs/BOAT_BOARD_INTEGRATION_R4.md and BOAT_HARDWARE_PIN_CONNECTIONS_R4.csv. No final pin freeze or actual wiring. New ADS1115 and companion controller are proposals; exact module and power cutoff require selection/engineering. TPS3431 150pF candidate timing 66.61ms nominal, approximate 59.8–73.5ms bounds before total cutoff; requires measured <=100ms including gate/feedback. Four separate coil connections documented, pulse/current/fuse ratings pending. No KiCad schematic or ERC completed in this batch.
- e640800 adds bounded local UC6580 checksummed GGA/GSA/RMC/GST parser requiring coherent UTC epoch and actual accuracy estimate, calibrated tilt-compensated heading math and ADC pack conversion. No HDOP fabricated as metres; GST output must be verified/enabled and estimated uncertainty commissioned. 1ebd347 adds actual Arduino UART/I2C register acquisition for UC6580, MPU9250/AK8963 and proposed ADS1115, and normalized reversible motor PWM/one-hot output planner. ca79245 strengthens coil channel-change/fault guards and adds optional read-only locally audited diagnostic sketch path. Default remains no GPIO/sensor or actuator activation. Heading/battery calibration initially disabled, no state-of-charge or safety flag fabrication. No companion firmware or high-current output GPIO adapter yet; no physical testing. Boat host #46 (44811977065) and #47 (44812118555) passed 19 and 20 groups respectively; latest #48 (44812326454), target #16 (44812326290), ZIP #1085 (44812329736) pending at this checkpoint. Last verified Boat #13, Remote #87 unchanged. Version markers Remote 028 / Boat 014 unchanged. Next verify #48/#16/#1085; resolve actual Tracker revision and I/O topology before native schematic, final hardware driver mapping, watchdog gate and current/feedback design, then physical acceptance.

### R4 final CI evidence
Boat host #48 (44812326454, job 136551764834, source ca79245cb6800637192514d3728826449b049593) PASSED all 20 groups. Boat target #16 (44812326290, job 136551916706, same source) PASSED: program 357,917/1,310,720 bytes, globals 20,600/327,680, leaving 307,080. Default diagnostic configuration compiled; locally commissioned optional sketch configuration and actual sensor I/O untested. ZIP #1085 (44812329736, job 136551778130) PASSED snapshot build, integrity checks and upload; archive publication skipped and inner archive contents not independently inspected. Remote #87 and Remote host #101 remain valid for unchanged Remote firmware. Physical tests NOT RUN; versions 028/014 retained. Next resolve V1.2 exact hardware documentation and candidate companion-I/O design approval, then capture native schematic/verify power budget and implement companion peripheral protocol/driver and compile-time signed pin profile.


## Owner design decision — Heltec-only MDD20A restoration (R5)
The owner explicitly rejected the RP2040 companion and restored the original Heltec Tracker + Cytron MDD20A direct PWM/DIR design. This supersedes the R4 companion recommendation and all companion development commits. Removed companion Pico SDK source, wired protocol/master/safety core, companion-only tests and Actions workflow from the active branch; Git history retains them. Kept Tracker GNSS/IMU/ADC acquisition, output planner, existing radio/session/mission/release/telemetry and independent electrical watchdog/STOP requirements. Added a fail-closed direct GPIO profile validator and regression; diagnostic sketch remains unassigned. New active R5 design and pin schedule supersede R4. No MCU pins, expander part, high-current cutoff, or PCB frozen: authoritative V1.2 pin map and exact GPIO budget unresolved. Four independent solenoids retained. Remote source unchanged; release version markers Remote028/Boat014 retained. Re-run Boat target, 21 host groups and ZIP packaging; record actual results. All physical acceptance gates remain open.

### R5 rollback verification
Commit a22886a7c0c59694a5a8c2d7bff3a519d1e4a047 updates 28 paths atomically. Recursive tree audit confirmed no companion-specific source, master link, safety core or workflow files remain. Prior abandoned Boat target #19 (44813593533) failed in carrier_link/carrier_master_port; those obsolete files have been removed rather than repaired for a rejected architecture. Boat host #52 (44814101535, job 136557839658) PASSED all 21 active groups, including direct GPIO profile regression. ZIP snapshot #1091 (44814101036) PASSED; official markers unchanged, inner archives not independently inspected. Boat target #20 (44814101124) still running at verification; last successful Boat baseline #16. No new compilation pass claimed.
Active R5 BOM added; old schematic/BOM references explicitly marked historical for corrected four-channel topology. Remaining: confirm exact Tracker revision, audit native GPIO count and optional non-programmable I/O expansion, capture direct MDD20A driver and independent watchdog/power-gate native schematic, implement physical driver only after pin freeze, then commission with disconnected actuator power. Remote untouched. No physical or ERC acceptance.


## R6 continuation — direct MDD20A HAL, pin budget, sonar deferred (2026-11-21)
- Boat target #20 (run 44814101124, job 136557839493, prior approved R5 source) PASSED. Program 358,009/1,310,720 bytes, global RAM 20,600/327,680 bytes (307,080 remaining); tooling emitted only external esptool/Node warnings.
- 3fa3799 adds `mdd20a_arduino_stage.*` (Arduino-ESP32 3.x LEDC 20 kHz/10 bit, two channels, direct PWM/DIR, initial PWM/permit/WDI LOW), and `mdd20a_direct_safety.h` with local measured feedback freshness and latched failure; **driver is NOT instantiated or attached to active Boat firmware, no GPIO selected and no power output authorized**. 3d1a755 makes motor→coil interlock enforce the existing shared output planner; d70028f corrects a test expectation at exactly the 30 ms post-release deadtime boundary. Full target and host verification of these new changes must be checked separately before asserting pass.
- Added R6 direct GPIO budget and electrical capture checklist: independent TPS3431/safety cutoff, Heltec native MDD PWM/DIR/WDI/permit, optional non-programmable I/O expansion only for coil/feedback, and strict signed pin map gate. Exact V1.2 PCB/schematic not supplied, final GPIO allocation blocked; current work is an interface skeleton, **not native KiCad schematic or ERC**. Power, motor and coil values remain commissioning candidates only.
- **Sonar is intentionally only planned** in Boat/docs/BOAT_SONAR_INTEGRATION_PLAN_UNDECIDED_R1.md. Owner has not selected a model. No sonar code, new display, GPIO, transducer wiring or chosen transport. One existing 4.3-inch 800×480 Waveshare screen, real CHIRP-like fish arch echogram, manual/autopilot, 50/50 map/sonar view and optional full-screen mode remain requirements; actual data protocol, electrical budget and high-throughput link depend on selection.
- Prior Remote #87 and unchanged Remote software remain verified; official markers Remote VER028 / Boat VER014 unchanged, no physical acceptance or release authorization. Next: check latest target and host CI, correct errors, freeze the exact Heltec V1.2 pin map and safety hardware/feedback, and capture a native MDD20A/power/coil KiCad design after verified choices. No physical actuation before bench proof.


## R6 verification and frozen sonar boundary — 2026-11-21

- **Boat ESP32-S3 target #22 PASSED**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865488754 (job 136728159017, source 3d1a75530f7e27dc0132de36a93de999f7ed0a8c). Program 358,121/1,310,720 bytes; globals 20,600/327,680 (307,080 remaining). Arduino core 3.0.2 source with direct MDD20A LEDC output HAL compiles; target #21 was superseded. No physical GPIO configured.
- **Boat portable host tests #55 PASSED all 22 groups**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865543808 (job 136728262617, source d70028f22e73d52b967bb2cb35057d443c2e2cc6). First intermediate test runs #53/#54 failed due incorrect motor/coil state and deadtime test expectations. The source and test were fixed in 3d1a755/d70028f; the final source passes regression. These were software test failures, not evidence of an actuated device.
- ZIP packaging #1096 PASSED on documentation commit 465caea4696c928100734f99919af1f2f0075f31: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865619611. Boat snapshot artifact 14691667268, Remote snapshot 14691388678, digest manifest artifact 14691647257. Inner ZIP payload and hashes have not been independently inspected outside CI. No official release.
- Confirmed Heltec-only architecture. Direct MDD20A stage remains **uninstantiated** in the Boat diagnostic sketch and gate/STOP/WDI feedback remains uncommissioned. It does not energize the actuator permit or auto-pet the independent watchdog. **Do not claim motor-driver physical integration or circuit safety acceptance.**
- R6 sonar planning is recorded **without selecting any sonar model or implementing sonar**. The one-Waveshare-screen full echogram and fish arch requirements remain, with sonar power/data connector, transport, parser and transducer unassigned. Dedicated model selection gate retained. No code, GUI, firmware, GPIO pin or hardware circuit was added for sonar.
- Unresolved engineering decisions before native KiCad/Electrical Rule Check (ERC): exact Heltec V1.2 authoritative schematic/pin audit; six truly free direct output-capable motor/permit/watchdog pins while preserving GNSS+nRF+E22+I²C; whether a non-programmable I/O expander is necessary for four solenoids plus physical feedback; actual independent watchdog cutoff, coil currents and fuse ratings. **No board purchase, physical acceptance or official release.** Remote #87 and versions Remote 028 / Boat 014 remain unchanged; planned 029/015 gated on hardware acceptance.


## Next-step engineering R7 — direct MDD20A feasibility and KiCad draft (2026-11-21)

- Current Heltec-only MDD20A design preserved. Added V1.2 conditional GPIO feasibility program and matrix. Native minimum for nRF primary + external E22 UART + I²C + MDD20A PWM/DIR + WDI + permit is 12 pins; optimistic unverified V1-family candidate pool 11. A GPIO expander for coils alone cannot resolve the minimum deficit. No actual Heltec GPIO pins allocated. Non-programmable I²C-UART bridge or expander for DIR are contingency studies, not approved selections. Source documentation: Boat/docs/BOAT_MDD20A_ELECTRICAL_GATE_R7.md.
- Captured editable **preliminary native KiCad** MDD20A control/watchdog sheet with custom embedded connector and TPS3431 symbols; first KiCad 9 netlist export succeeded. **First ERC reported 83 violations**, not an electrically approved/fabricable design. High-current switch/gate, 4 solenoid MOSFETs and final rails are not captured. Dedicated KiCad validation workflow and report artifact created; ERC must be reviewed and corrected before board release.
- Added measured-only pack current budget and hardware fault-latch requirements with software verification. No invented motor stall, coil current or fuse selection. Physical watchdog signal must latch actuator power OFF independent of MCU; 150 pF CWD is a candidate (66.61ms nominal, approximate 59.8–73.6ms bounds excluding switch/current decay). Every physical input/driver still default-denies.
- Sonar plan remains **unchanged and model undecided**. No sounder, firmware, UI, connector pin/power or code has been implemented this turn. Existing 4.3-inch screen and future full CHIRP-style fish arches remain requirements.
- No release increment or physical acceptance. Remote #87 and Boat #22 target compiler baselines remain last verified for unchanged application code. Host regressions and ZIP snapshots run in GitHub Actions; check the latest jobs for exact SHA before claiming results. Next: authorize a verified GPIO resource solution and complete KiCad/ERC, physical watchdog, power and four-coil circuits.


### R7 completion checkpoint — 2026-11-21

**Active architecture unchanged:** single Heltec Tracker + external Cytron MDD20A, nRF24 primary/E22 backup, four independent solenoid channels, no RP2040. Sonar model explicitly **undecided**, sonar code/wiring **not implemented**.

**Verified software and project checks:** Boat host #59 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714160) PASSED all existing portable regressions plus GPIO feasibility, pack-current safety and gate truth-table tests (24 groups by workflow stages). ZIP packaging #1103 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714149) PASSED for source SHA 358ed10687fbd09606aafbaf18a730cc0ef80e49. Boat target #22 remains last verified ESP32-S3 diagnostic source build; subsequent commits modify tests, docs, and preliminary CAD only. Remote target #87 remains last verified Remote build. Official Remote VER028 and Boat VER014 unchanged; 029/015 withheld.

**Native preliminary KiCad progress:** KiCad workflow #6 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866942940, source e2ec4f6c8cfc3852c78d1f475a7f57af1c7a6ed4) PASSED parser/netlist export and geometry/project-symbol checks after adding project-local `Flytec.kicad_sym`, `sym-lib-table` and `.kicad_pro`. Unresolved ERC violations reduced from 83 to 26 after grid alignment, then to **15** after project library setup (13 unimplemented release/feedback net connections and 2 unpowered TPS3431 power pins). **ERC production acceptance FAILS/NOT COMPLETE**. Remaining physical power gate, watchdog fault latch, high-current motor/coil circuits, fuse/protection, final footprints, true V1.2 pin assignment and PCB are not captured. A successful *preliminary* KiCad check must never be represented as zero-violation ERC.

**Verified GPIO shortfall:** provisional best-case pin pool 11, minimum native demands 12 with both external radios retained; even this assumes contested oscillator/LED/UART/flash pins are available. New active `Boat/tools/gpio_feasibility_r7.py` and matrix explicitly block unsafe assignments. I²C-UART bridge or direction expander remain optional proposals, neither selected or implemented. All physically driven GPIOs remain unassigned/disabled. No hardware or physical acceptance evidence; no sonar model or implementation. Next: decide a safe extra I/O topology after exact Heltec V1.2 pin documentation, fill unresolved nets/rails in native KiCad, verify a separately latched electrical cutoff, run zero-violation ERC and physical commissioning before releases.

## R8 follow-up — integrated Heltec SX1262, 2026-11-21

Owner-selected architecture: one Heltec Wireless Tracker ESP32-S3, external Cytron MDD20A with native two-channel PWM/DIR, nRF24 primary, and **Heltec onboard SX1262 SPI LoRa** (Boat E22/E227 UART retired; no Boat SC16IS750). No RP2040. Four independent bait/hook solenoid drivers and separately latched physical watchdog/STOP power gate remain required. Sonar stays on hold: no chosen model, hardware, firmware or circuit.

New Boat source: heltec_sx1262_link.* and heltec_sx1262_airtime.h; transport construction now selects integrated LoRa. All GPIO remain -1, V1.2 board audit/remote PHY/regulatory sign-offs false: no radio TX, motor or solenoid activation authorized. RadioLib 7.3.0 pinned for compile attempts, host GPIO/airtime tests added. No compile evidence for this R8 source until Actions checks finish.

GPIO demand drops from historical R7 12 to **10 external native controls**; 11 optimistic provisional candidates yields a count-only pass, but conservative uncontested pool of five still fails. Onboard SX1262 pins 8–14 remain internally occupied, and shared nRF SPI routing/header access is not signed. No pin freeze.

**Critical Remote incompatibility:** Remote remains E22 UART via SC16IS750. Its embedded E22 transparent RF packetization is not automatically compatible with a raw SX1262 SPI radio. The Boat's LoRa backup is intentionally unavailable until a matching RF protocol/remote revision is validated. At SF9 BW125, estimated 32-byte airtime ~247ms; 3s heartbeat and 6s freshness are not certified compatible with 1% duty. Regulatory and reset-persistent airtime enforcement unverified.

Remaining R7 KiCad ERC: **15 violations**, not production-ready. High-current latch, four MOSFET/coil drivers, rails, footprints, PCB and physical testing still outstanding. Current release markers Remote 028 / Boat 014 retained; 029/015 withheld. See Boat/docs/BOAT_INTEGRATED_SX1262_R8.md and Boat/docs/BOAT_LORA_HARDWARE_DECISION_R4.md.

Commit evidence: source 975088133dbd8c4e9cb81d3760a5b65292f7426f; GPIO/CI/documentation 557d601b1a561fd0a908e90d24839e12b3ab3072.

### R8 integrated SX1262 verification (2026-11-21)
- Boat ESP32-S3 **target compile #27 PASSED** on source SHA `9c96f937011b4af15179b2cb3c172df2bf0b1b83`; https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167311 . Arduino-ESP32 core 3.0.2, RF24 1.4.11, RadioLib 7.3.0. Program 417,037/1,310,720 bytes (31%), global memory 22,120/327,680 bytes (6%, 305,560 remaining). Diagnostic only: GPIO/radios uncommissioned, actuator power disconnected.
- Boat portable host tests **#64 PASSED** on same source SHA: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167361 . New test includes 10-signal GPIO feasibility, LoRa airtime and 1% interval model; simulated checks only.
- Snapshot ZIP CI #1110 **PASSED** on source SHA 9c96f937: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167366 . Newer docs-only snapshots also generated; official release markers remain 028/014 and their immutable releases were NOT authorized.
- Code fix after transport insertion: shared SPI selects inactive before bus init, then RF24 init and SX1262 init; RadioLib constructor requires `SPIClass&`, corrected. Commits `ca82b7dc`, `9c96f937`.
- R8 superseding purchasing list and pin schedule: `Boat/docs/BOAT_BOM_CANDIDATES_R8.md`, `Boat/docs/BOAT_HARDWARE_PIN_CONNECTIONS_R8.csv` (commit `2ed54e3`). Electrical capture drafting spec: `Boat/docs/BOAT_SAFETY_SOLENOID_CAPTURE_R8.md` (commit `7bb4968`). These are *not* native KiCad power/coil circuits.
- The **remaining blocker** is an authoritative V1.2 signed pin map and verified Remote-to-Boat backup LoRa PHY. Remote still runs E22/SC16IS750; Boat raw SX1262 is intentionally disabled. 10 vs optimistic 11 GPIOs is count-only. Independent physical fault latch, all four coil driver circuits and measured power budget are NOT complete; R7 KiCad has **15 open ERC violations**. No PCB/DRC, hardware acceptance, or sonar integration. Retain single Heltec + direct MDD20A without RP2040.

### Steering confirmation — twin-motor differential thrust (2026-11-21)
- The Flytec V080 **has no rudder, steering servo, or separate steering actuator**. Heading is controlled exclusively by differential speed and direction of the two existing 12 V 550 propulsion motors.
- MDD20A channel 1 drives the port/left motor; channel 2 drives the starboard/right motor, with independent PWM and DIR each. Both-forward equal thrust means straight travel; turning varies left/right thrust; optional pivot turns need bench/water safety approval. Motor polarity and propeller orientation must be verified on the real hull.
- Manual `TANK` mode commands left/right motors independently. Manual `STEERING` mode **mixes throttle + yaw into two motor commands**; it does not command a rudder. Autopilot navigation `headingErrorDeg` is likewise mixed to left/right motor requests in `Boat/source/boat_hardware_supervisor.h`, respecting the actuator safety plan.
- Existing source inspection: `Remote/source/joystick.cpp`, `Boat/source/boat_hardware_supervisor.h`, `Boat/source/carrier_output_plan.h` and `Boat/source/mdd20a_arduino_stage.cpp` already represent this architecture. No physical motor driver instantiated in the default diagnostic sketch; neither physical directional polarity nor turn dynamics have been commissioned.
- **No steering-servo GPIO, servo driver, rudder linkage, actuator budget, or additional mechanical steering assembly is allowed** in the Boat schematic/BOM. Preserve four native MDD20A outputs (PWM_L, DIR_L, PWM_R, DIR_R), no RP2040, independent watchdog/STOP and four separate coil channels. Sonar remains on hold.

## R8 implementation correction — autonomous forward-only turning
Code and host regression on 2026-11-21: `Boat/source/boat_hardware_supervisor.h` clamps autonomous navigation propulsion commands to [0, configured `maxNavMotor`] individually after heading-error differential mixing. **No negative per-motor autonomous commands** are issued, even for large heading error; positive bearing error accelerates left/port over right/starboard (right turn), negative error the reverse. Sharp-course changes can turn by propelling only the outside motor while the inside motor is stopped. This is safer than the previous automatic counter-rotating/pivot behavior until measured hull turn dynamics and reverse allowance have been signed.
Manual TANK/reverse and explicit joystick override remain independent signed-command paths, still protected by command lease, STOP and hardware safety requirements. Ground/water effectiveness, pinouts, polarity and cutoff are unverified. Automated regression is `Boat/tests/boat_hardware_supervisor_test.cpp`; real target and physical checks remain outstanding.

### R8 GPIO pin-allocation study and vendor connector cross-check
Created offline, non-energizing candidates A and B under `Boat/tools/boat_gpio_candidate_r8.py`: ten native signals with WDI on 17, independent permit on 4, nRF CSN on 5, I2C on 6/7, PWM on 15/16, directions on 43/44, and nRF CE on either 18 (board LED conflict) or 37 (DQS/flash conflict). These deliberately **do not** populate a physical firmware pin configuration. Each requires a signed exact V1.2 pinout and SPI-access audit. See `Boat/docs/BOAT_GPIO_PIN_STUDY_R8.md`; new host tests enforce unique pins, exclusion of internal SX1262/GNSS/TFT/USB wiring and fail-closed status.
MDD20A vendor Figure 3 uses named signals DIR1/PWM1/DIR2/PWM2/GND, not the retrofit carrier's logical PWM_L/DIR_L/PWM_R/DIR_R/GND connector order. A crossover harness mapping is documented in `Boat/docs/BOAT_HARDWARE_PIN_CONNECTIONS_R8.csv` and `Boat/docs/BOAT_DIFFERENTIAL_STEERING_R8.md`; do not wire as a pin-to-pin ribbon. Actual hardware orientation remains unverified.

### Automated harness crossover checks
`Boat/tools/mdd20a_harness_r8.py` plus host unit test now reads the R7 KiCad J2 connector's five **actual net labels** and enforces `PWM_L,DIR_L,PWM_R,DIR_R,GND`. It rejects same-order/incorrect Cytron vendor-label mating in favor of the explicit `PWM1,DIR1,PWM2,DIR2,GND` crossmap in `Boat/docs/MDD20A_CARRIER_CROSSOVER_R8.csv`. This validates only logical signal mapping; the vendor's physical socket order and motor winding polarity are unverified, and no coil/motor power is authorized.

### R8 differential-thrust / pin-risk / harness test verification — 2026-11-21
- Autonomous differential motor mixer now **forward-only** (no counter-rotating automatic pivot), while manual TANK signed reverse and joystick override remain separate supervised paths. Change SHA `ad487c77128fadd54d333362d36ab8eceb6a0855`: Boat target ESP32-S3 diagnostic build **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868652840; Boat host test **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868652827.
- Two structural-only Heltec V1.2 GPIO candidate plans (10 native signals; WDI17, permit4; nRF CE18 or 37) and a manufacturer-label MDD20A crossover harness check committed. No firmware pin values were assigned or actuators enabled. See `Boat/docs/BOAT_GPIO_PIN_STUDY_R8.md`, `Boat/docs/MDD20A_CARRIER_CROSSOVER_R8.csv`.
- GPIO candidate and host regressions **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868795058 (`9e7ed85`). Initial new KiCad J2 label parser matched J5 feedback labels as well, so the first harness regression `0d9e527` failed; corrected by bounding the connector's y-coordinate (`122141d`), and added `Boat/tools/**` to CI triggers (`fe08663`). Latest Boat host CI **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868963948, on `fe08663af3174003eedebddac0cb7eb1f8b9b682`.
- This is **source, offline-test and documentation evidence only**. Heltec exact V1.2 pin access/boot states and nRF/SX1262 shared SPI, watchdog latch, independent four-coil hardware and motor orientation are not physically verified. R7 KiCad still has 15 ERC errors; no fabricable high-current gate, coil sheets, PCB or hardware acceptance. Integrated Boat SX1262 backup remains intentionally disabled because Remote E22 RF interoperability is unverified. Sonar remains on hold, no hardware/software selected.
- Official Remote VER028 and Boat VER014 retained; VER029/VER015 remain withheld.


## R9 engineering continuation — native four-coil KiCad and independent safety model (2026-11-21)

**Completed and verified in connected GitHub branch:**
- Native editable schematic `Boat/hardware/Coil_Drivers_R9/Boat_Four_Coil_Drivers_R9.kicad_sch` with 26 parts: four independently fused 12V coil plugs, four low-side NMOS stages, four cathode-to-positive flyback clamps, four input series gate resistors and four pulldowns; separate supply J1 and hardware-limited one-hot pulse input J2. Project-local KiCad symbols and `.kicad_pro` are included. All footprints/rated power device selections explicitly UNASSIGNED.
- KiCad R9 workflow **PASSED**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44874476930 (`546eacf83a0cf3df24f2d1c6f087368505d146df`). Native KiCad XML netlist exported, exact device pin/net checks passed for **18 distinct nets**, and this **standalone low-side coil sheet reports zero ERC violations**. Previous run failed solely because the test needed to normalize KiCad's root sheet `/` local-label prefix, corrected on `546eacf`. This says NOTHING about approval of the entire electrical system.
- New software-only reference model `Boat/tools/safety_gate_latch_r9.py` enforces fault-latched stop with physical ARM **released-then-pressed** under healthy stationary feedback and tests a 4-channel one-hot non-retriggerable pulse limiter with time cutoff and stuck-high rejection. **Not an implemented physical latch/monostable circuit.** Boat portable host CI **PASSED**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44874405863 (`88bb983f488b6778e88222f9eaa76d1612e5f446`).

**Remaining blockers unchanged:**
1. Capture actual 3S pack fuse, verified 3S-to-5V supply, all motor/coil high-current rails and **normally-OFF load-rated high-side cutoff** in native KiCad, independent of Heltec; derive part ratings from measured two-motor stall, all four solenoid currents, pack/BMS waveform and temperature.
2. Capture actual TPS3431-driven hardware fault latch for STOP, WDO, UVLO, overcurrent, thermal; force latched OFF at boot, and make rearm require a distinct physical button edge, healthy rails, verified motor+coil zero output. An abstract simulator does not implement this.
3. Capture real hardware one-hot gate selector, non-retriggering maximum coil pulse, and current-zero feedback; never join R7 `SOL_REQ_*` straight to R9 `PULSE_*_LIMITED_TBD`. No direct MCU-to-coil gate!
4. Integrate native KiCad hierarchy and clear **15 old R7 ERC violations**. R9's standalone zero ERC does not reduce those. Next PCB/DRC, measurements, controlled dummy-load STOP/watchdog verification, and physical acceptance.
5. Authoritative Heltec V1.2 GPIO/shared-SPI physical audit and Remote E22↔Boat raw SX1262 radio PHY compatibility remain unverified. Boat LoRa backup and actuator GPIO remain disabled. Sonar unselected and untouched. No rudder, no RP2040. Official release markers remain Remote 028 and Boat 014; 029/015 remain withheld.


## R10 engineering checkpoint — independent STOP/witness, high-side candidate and cross-sheet safety (2026-11-21)

- Added native editable `Boat/hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch` with **23 generic component instances** and **35 exported KiCad electrical nets**. It comprises SN74HC08 safety-input AND tree, SN74HC74 positive-edge ARM/asynchronous-clear latch, SN74HC14 redundant STOP witness, a candidate PMOS high-side branch to BOTH MDD20A and four-coil supplies, a low-side MOS gate driver, main/motor/coil fuses, physical NC STOP source-return contact and second independently monitored NC witness. Actual monitored J3 STOP_OK requires a third mechanically coupled contact and independent conditioning. **Hardware is draft/not powered/not a signed safety system.**
- STOP single-monitor stuck-good hazard was caught in review: S1 opening Q2 source alone could allow automatic reenergization after physical STOP release if J3 STOP_OK stuck HIGH. Added S2 separate NC witness, candidate R5 pullup and SN74HC14 to clear the latch even with the other contact input stuck HIGH. Hardware triple-pole STOP mechanical selection/synchronism/required NC behavior and actual ARM debounce/release-qualified pulse remain unverified.
- Native R10 KiCad target run **#(44890983595) PASSED structural tests** on `e77facdc4edb789906429cbd215404858374e266` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44890983595): **35 nets**, no geometry warnings, **exactly TWO unresolved `power_pin_not_driven` ERC errors** (3V3 safety rail and return; no regulator approved). CI now **checks for exactly those two blocking power-source errors** and rejects new violations; this is not a zero-ERC or fabrication release.
- All **1,024 combinations** of the ten external conditioned inputs and the redundant STOP-contact scenarios are covered in host-only truth-table tests, including stuck ARM and permit. Boat host CI **passed** on `7d515e89` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44890701527), as did the cross-sheet interface host CI `7d894bf` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44891024408). A Python model is not hardware fault containment.
- `Boat/docs/BOAT_R7_R9_R10_INTERFACE_CONTRACT.csv` enumerates critical unverified signals. R7 has its own 10k WDO_N pullup; R10 R1 is now **DNP if R7 pullup fitted** to avoid an undocumented parallel pull-up. R10 coil rail connects to R9 `ACT_SOL_POS_AFTER_LATCH_TBD`; return mapping is PACK_RETURN to R9 ACT_RETURN only through a separately rated harness. R7 solenoid requests MUST NOT directly drive R9 gates: the independent one-hot hardware limiter and pulse time-out do not exist yet.
- Remaining ERC by **standalone sheet**: R7 control/watchdog **15 unresolved**, R9 low-side coil **0** (but not a complete actuator system), R10 STOP/power candidate **2 deliberate external-power-source errors**. These are separate schematics; no combined hierarchy, system-wide ERC, PCB DRC, measured thermal/stall/SOA, physical cutoff latency, final 3S fuse/regulators, ARM hardware, overcurrent/thermal/UVLO comparators, or production footprints.
- Keep exactly one Boat Heltec Wireless Tracker + separate MDD20A dual PWM/DIR differential thrust; **no rudder**, no RP2040, onboard Boat SX1262 currently blocked by unverified Remote E22 PHY interoperability. Sonar remains unselected/held with no implementation. No physical actuator activation. Official markers Remote VER028 and Boat VER014 unchanged; 029/015 withheld.

## R11 pulse-limiter continuation (2026-11-21)
R10 high-side safety cutoff already exists as a preliminary native KiCad draft. R11 adds a concrete non-retriggerable hardware pulse-limiter specification in `Boat/docs/BOAT_PULSE_LIMITER_R11.md` and host-only fail-closed timestamp discontinuity and exhaustive request-mask regressions to `Boat/tools/safety_gate_latch_r9.py` / `Boat/tests/test_safety_gate_latch_r9.py`. No R11 native circuit, physical pulse limiter, final component values, electrical commissioning or version bump is claimed. R7 15 ERC / R9 0 standalone / R10 2 external-power ERC remain; sonar held.


## R11 physical-timing candidate and verified KiCad continuation — 2026-11-21

- Created an actual editable KiCad schematic at `Boat/hardware/Pulse_Limiter_R11/Boat_Pulse_Limiter_R11.kicad_sch` with 17 components: CD74HC221 monostable, SN74HC175 channel latch, six SN74HC08 quad AND ICs, two SN74HC32 quad OR ICs, one SN74HC14 Schmitt inverter IC, three R/C passives, and three harness connectors. All footprints and timer R/C ratings remain unassigned.
- Native KiCad **PASS** run https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201259 on `cb8ed8546bff1c5555ddecc667c052365cbe61d1`: 54 named exported XML nets checked against exact pin numbers, 9 explicit no-connect pins, 1,024 static one-hot/window combinations and cross-sheet logical-name checks passed. The isolated R11 sheet has **exactly two outstanding power_pin_not_driven errors**: the external unqualified 3V3 safety rail and return. No additional ERC errors; neither 2 ERC errors nor isolated ERC proves system safety.
- Existing Boat host CI PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201349; ZIP packaging workflow PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201366. Previous R11 KiCad run failed a test assertion pin number (pin 13 vs real pin 12), corrected before passing.
- The new interface contract `Boat/docs/BOAT_R7_R9_R10_R11_INTERFACE_CONTRACT.csv` maps R7 raw release, R9 MOSFET gates and R10 safety, but **all physical inter-sheet connections remain UNBUILT**. R10 internal LATCH_Q is not wired out to an approved connector.
- **Blocking unsafe failure mode:** The external `RELEASE_EDGE_TOKEN_TBD` is **not implemented**. If this input remains HIGH, a held release request can trigger a second pulse after the 74HC221 window ends. Analog motor-/coil-current-zero comparators, independent 3V3/POR and rail-good logic, R/C pulse bounds, input filtering/setup/hold and actual actuator STOP/current-off testing are also missing. **Never connect/energize motors or solenoids from this schematic.**
- Remaining standalone ERC blockers: R7 **15**, R9 **0** (isolated low-side stages), R10 **2** (external power), R11 **2** (external power). No combined ERC, PCB DRC, validated high-current path, physical acceptance or release.
- Next: capture the *consumed* hardware release/zero-current rearm token (no autorepeat), independently qualify current-zero and logic supply, analyze race/glitch/fault timing, establish manufacturer pulse width/fuse/MOSFET ratings from measured hardware, integrate four sheets into native KiCad hierarchy and run system ERC. Sonar remains **planned only**; no sonar module selected. Single Heltec Boat controller with MDD20A differential motors/no rudder, no RP2040 and no Boat E22. Official Remote VER028 / Boat VER014 unchanged; 029/015 withheld.

## R12 physical consumed-release token — verified isolated KiCad candidate (2026-11-21)

- New native **editable** schematic `Boat/hardware/Release_Token_R12/Boat_Release_Token_R12.kicad_sch` (23 generic components) and project-local symbol library. Two flip-flops in SN74HC74 implement **RELEASE_ARMED** (set only after a separately RC/Schmitt-qualified all-requests-released and current-zero condition) and **FIRED_Q** (held during a pressed request, consuming RELEASE_ARMED asynchronously). CD74HC221 supplies a *separate short token* to existing R11 `RELEASE_EDGE_TOKEN_TBD`; six pairwise overlap terms reject multi-request patterns. R1-C4 timing/filters unspecified, all footprints blank. R12 output connector J3 is **NOT physically wired to R11 J2** yet.
- Actual KiCad CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745446 on `ee9ff4fbb122ce59ce2628007c3ad2f05d24ace8`: **53 precisely matched electrical nets, 11 explicit no-connect output pins, 256 static mask/ready tests and 7 synthetic temporal-model tests**. Only **2 unresolved isolated-sheet `power_pin_not_driven` ERC violations** remain for the unapproved 3V3/ground source; this is NOT zero ERC on full Boat. Boat portable host CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745373; ZIP packaging CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745305.
- Detailed interface mapping: `Boat/docs/BOAT_R7_TO_R12_INTERFACE_CONTRACT.csv`, with every unbuilt harness/status explicit. R7 SOL_REQ_x to R12 J1; R12 J3 pin1 to R11 J2 pin6; R11 J3 4 gates to R9 J2; R10 internal LATCH_Q is NOT exposed on an approved connector. R10 feeds R9 coil rail only by a proposed independent fused power connector.
- Critical remaining hazards listed in `Boat/docs/BOAT_R12_HAZARD_REVIEW.csv`: external POR may assert before 3 separate RC filter nodes settle, request bounce/race, non-zero coil current or failed current sensor, R12 token width longer than R11 main time, stuck-high token that can retrigger R11, U2 or high-side MOSFET failure, motor and coil real flyback, unverified 3S high current. **No actuator power may be connected.**
- Component gate: qualify *actual* 3S-to-logic regulator/POR, R/C release/debounce/reset-to-CLK and token-vs-main timing over tolerance+temperature, independent motor/coil current-zero comparators, physical R10 STOP latch and output connector, and a power-rated high-side actuator disconnect for both motor and solenoid supply. Integrate actual combined KiCad hierarchy/ERC/PCB/DRC and perform dummy-load fault acceptance.
- Standalone ERC status: **R7 15, R9 0 (partial low-side only), R10 2, R11 2, R12 2**. No system-wide zero ERC or fab authorization. Existing Boat and Remote pin maps remain uncommissioned; no RP2040, no rudder, no Boat E22. Sonar has no selected model and remains **planning only**. Official versions Remote VER028 / Boat VER014 unchanged; VER029/VER015 withheld.

## R13 RECOM ready-made 3.3 V safety supply (2026-11-21)

**User choice: use a premade regulator module instead of a custom 3.3 V feedback/inductor buck design.** Selected candidate **RECOM R-78E3.3-0.5** SIP3, datasheet REV 9/2025 https://recom-power.com/pdf/Innoline/R-78E-0.5.pdf. Vendor pinout **pin1 VIN, pin2 GND, pin3 VOUT**; input **6–28 V**, fixed output **3.3 V**, max **0.5 A**, **11.6 × 8.5 × 10.4 mm**, non-isolated. 3S 12.6 V maximum nominal charge is in range assuming measured input spikes stay within 6–28 V.

Created native **R13 KiCad** power interface `Boat/hardware/Safety_Power_R13/Boat_Safety_Power_R13.kicad_sch` with 9 components: fused protected battery-to-module VIN, supply/common-return input, VLOGIC_3V3/GND outputs to R10-R12, test interface, optional 10uF/35V input and 10uF/10V + 100nF output decoupling, optional 300 ohm/0.125 W preload (~11mA; RECOM recommends >2% min load for specified ripple). No fuse value or footprint approved; optional parts may be omitted only after ripple and EMC tests. Vendor 220uF maximum output capacitive-load condition includes all downstream decoupling. `Boat/docs/BOAT_R13_READYMADE_REGULATOR_BOM.csv` lists choices. Native exported KiCad netlist / ERC CI is `boat-kicad-power-r13.yml`.

**Safety limits:** this preassembled non-isolated regulator is **only for independent safety logic**, not nRF24 PA/LNA burst supply, not Heltec ESP32 input (which needs separately approved 5V), and never motor/solenoid load. Source is MASTER-switched pack protected by BMS/fuse, electrically **before** the R10 actuator high-side cutoff. Its output ground shares a low-current star reference with the battery; prevent high-current motor/coil return in safety harness. The RECOM device has **no separate PG pin** and short-circuit protection **automatically restarts**. A separate physical POR/brownout supervisor must assert reset on loss and require manual physical fault-latch rearm after recovery. The regulator alone is **not** an independent safety interlock. Input 3S pack transient, fuse, mounting/socket retention, load/thermal, EMI/RF noise, real 3.3V ramp/current and POR/UVLO timing are **unverified**.

The new R13 power sheet does NOT automatically clear older separate-sheet ERC: R7=15; R9=0 (partial low-side); R10=2, R11=2, R12=2 (external supply); combined KiCad hierarchy, PCB, rated high-side cutoff and physical loads remain unfinished. No sonar implementation, no RP2040/rudder/Boat E22. Keep official Remote VER028/Boat VER014, no physical actuation permission.

### R13 native KiCad completed structural verification (2026-11-21)

- Manufacturer ready-made **RECOM R-78E3.3-0.5** safety-only 3V3 regulator; native sheet `Boat/hardware/Safety_Power_R13/Boat_Safety_Power_R13.kicad_sch`, module pins **1 VIN, 2 GND, 3 VOUT**, input fuse F1, optional RC supply components, separated 3.3V safety connector/test connector, no production footprints.
- Actual CI KiCad 9 XML+ERC **PASS**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44931874988 on commit `0b5eb1f149a62c5c951429a5ab37a57f1b485847`. Validated **9 component instances / 4 exact electrical nets**, including series fused input and vendor supply pin numbers. **Exactly two remaining blocking external power-source ERC errors** (upstream switched 3S supply and logic common return); **zero extra ERC warnings**, no off-grid endpoints. The first R13 runs failed missing connector endpoints and off-grid placement; native 1.27 mm grid correction `0b5eb1f` supersedes them.
- R13 is physically unassembled and its source BMS/fuse, transient protection, 500mA load budget, ripple, PG/POR supervisor, EMC, connector/socket, RF separation and physical R10/R11/R12 connections have not been commissioned. RECOM module has **no PG pin** and can restart after a short-circuit; independent reset/latch must stay OFF and require deliberate manual ARM. The R13 sheet does not imply zero whole-Boat ERC and **is not fabrication-ready**. No servo/rudder/RP2040/Boat E22; CHIRP sonar choice remains open, no sonar firmware. Official release markers Remote VER028 / Boat VER014 unchanged.


## R14 ready-made four-coil module feasibility (2026-11-21)
User asked whether an NCD 4-channel solenoid I²C board or equivalent can replace our new custom PCB designs. **Yes, for the four power MOSFET channels**, with important constraints. NCD PR22-5/PR22-4 list 5.5A/channel, below the project's *provisional 6A-per-coil design allowance*, and the advertised 8W is transistor heat dissipation—not an 8W coil limit. Preferred **candidate** is Electrokit **EKM014** 4-channel MOSFET board: listed 10A/channel, load 5–50V, inputs accept ESP32 3.3V GPIO, built-in flyback, and 4.5–12V driver supply **VL from separate regulated 5V**, not RECOM 3.3V or raw 12.6V charged 3S. Supplier states a single 10A slow-blow fuse in shared VH. Actual coil current/thermal/timing and board mechanical dimensions (conflicting supplier listings) must be verified.

**No component purchase/design freeze yet**. New source: `Boat/docs/BOAT_R14_PREMADE_SOLENOID_DRIVER_EVALUATION.md` and proposed interface: `Boat/docs/BOAT_R14_EKM014_HARNESS_CANDIDATE.csv`. Keep the earlier R9 KiCad driver board as unused engineering fallback; do not fabricate it. The off-the-shelf FET module **does not replace independent hardware one-hot/nonretriggerable pulse limiter, power-cutoff latch, STOP, current-zero sensing and POR**. Must choose/qualify off-the-shelf modules for those, or retain a small safety interlock design. MDD20A continues driving two motors; no rudder/RP2040. Sonar unselected. Remote VER028 / Boat VER014 held.

## R15 — owner-mandated premade-module-only electronics (2026-11-21)
**SUPERSEDES all previous custom PCB/carrier fabrication plans on Boat and Remote.** New canonical policy: `docs/HARDWARE_MODULE_ONLY_POLICY_R15.md`. ACTIVE module plan: `Boat/docs/BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md` and `Remote/docs/REMOTE_MODULE_ONLY_IMPLEMENTATION_R15.md`, plus manufacturer-grounded master BOM `docs/MODULE_ONLY_BOM_R15.csv` and physically uncommissioned harness contracts in Boat/Remote docs. All R7–R13 KiCad drawings and Remote historical PCB design notes are archived only; no Gerber/PCB manufacture, not even a small safety board. 3D-printed plastic enclosure and mounts remain permitted. EKM014 four-channel driver preferred candidate (not purchased/accepted), MDD20A and RECOM safety regulator remain, ready-made Pololu rails and Adafruit MCP23017 + two ADS1115 proposed; **0x48** reserved for SC16IS750; proposal uses 0x49/0x4A joystick/diagnostics ADC boards. All critical independently rated premade STOP/power-cutoff, POR, pulse rearm, current-zero modules are **unselected/unsafe**, so physical actuator enable remains disabled. Boat onboard SX1262 vs Remote E22 compatibility remains unverified. Sonar unselected and not implemented. Remote VER028 / Boat VER014 held.

### R15 further work — premade safety candidates and bench acceptance
- New [safety candidate review](docs/MODULE_ONLY_SAFETY_SHORTLIST_R15.md): Duelco NST-3.2 **8–36VDC** emergency stop monitoring candidate **42042237**; Cebek I-33 **12V** non-retriggerable 0.1s–4min relay timing candidate but **not approved** because 0.1s min may exceed actual safe coil pulse and relay only **2A inductive**, not 6A coil. Electrokit EKM025 INA260 **±15A** / 0–36V/4 I2C addresses current telemetry candidate, **not independent cutoff**. High-current DC-rated contactor, physical onehot and rearm, true independent current-zero and brownout reset **UNSELECTED**.
- New [hardware acceptance checklist](docs/MODULE_ONLY_ACCEPTANCE_R15.md) prohibits using simulator or GPIO output as safety evidence, and [module pin/I2C map](docs/MODULE_ONLY_PIN_AND_BUS_MAP_R15.csv) preserves Remote SC16 0x48 vs ADS1115 0x49/0x4A and 16-key GPIO 0x20.
- Six legacy KiCad workflows now manual-only and marked ARCHIVED; R7–R13 are **historical references only**, not PCB fabrication targets. Module-only portable adapter is **host-only** (not compiled into active Boat target GPIO). No hardware, power rails or actuator load physically commissioned. Boat VER014 / Remote VER028 unchanged. Sonar planning only.

## R16 owner-confirmed four-channel module (2026-11-21)

**HARDWARE SELECTION FROZEN:** Electrokit **EKM014**, manufacturer/article **41024394**, is the final *model choice* for Boat LH/RH bait and LK/RK hook solenoid **low-side** switching. Exactly **ONE** four-channel preassembled module. It **replaces R9 custom-FET PCB permanently** in the active module-only project. This choice does **NOT** mean purchased, electrically verified, qualified, or safe to energize. No substitution to NCD or other coil driver without owner approval.

Read the dedicated [EKM014 R16 hardware and wiring contract](Boat/docs/BOAT_EKM014_SELECTED_R16.md), [R16 channel/wiring CSV](Boat/docs/BOAT_EKM014_PIN_WIRING_R16.csv) and [R16 commissioning test plan](Boat/docs/BOAT_EKM014_ACCEPTANCE_R16.md) (relative paths from repository root). R15 module-only policy remains mandatory for both sides. A dedicated *commercial* hardware one-hot and no-repeat maximum pulse path, independent DC power cutoff/STOP, rail/brownout supervision and independent current-zero verification are still required before connecting coils. Sonar still planned only; no RP2040/rudder/custom PCB. Remote VER028 / Boat VER014 remain held.

## R17 — premade actuator safety procurement and wiring gate (2026-11-21)

**OWNER-CONFIRMED Electrokit EKM014 41024394 unchanged; no custom electronic PCBs.** New [R17 manufacturer-checked safety stack](Boat/docs/BOAT_PREMADE_SAFETY_STACK_R17.md), [18-stage interface/handoff matrix](Boat/docs/BOAT_R17_MODULE_SAFETY_HANDOFF.csv), [21 hard physical release gates](Boat/docs/BOAT_R17_RELEASE_GATES.csv) and [11 named candidate/required-component decisions](docs/MODULE_ONLY_CANDIDATES_R17.csv). Status **ALL 20 applicable physical gates NOT VERIFIED**; one sonar item is NOT_APPLICABLE by prior decision. These are commercially preassembled module candidates, **not a safe/assembled installation**.

Vendor review: Duelco **NST-3.2 42042237 8–36VDC** stop **supervisor** CANDIDATE, manufacturer-rated only **3A at 24VDC resistive contacts** (not motor/coil rails); manual reset X1–X2 must not be bypassed (permanent bridge causes automatic restart). Albright **SW60C/SW60A 12V continuous-duty** DC inductive contactor with optional auxiliary feedback CANDIDATE, full order code/coil sag/feedback/thermal/fit unverified. **SW60P IP66 has NO available auxiliary contacts** per manufacturer datasheet, so it fails monitored-contact requirement unless a genuinely validated alternative feedback path is provided. Factory-built ShillehTek 4-channel programmable 0.01-s mode timer is only a **functional research candidate**, NOT proved to reject simultaneous requests/held-high repeats/brownout or safe pulse width, and NOT a qualified safety PLC. Independently hardware-timed, one-hot, no-retriggering coil gating, current-zero and qualified reset/POR are still UNSELECTED. Industrial safety modules appear too tall for original 30mm slot; measure another boat hull area before shopping.

**CI R17 module-only gate passed** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44935838255: checks frozen EKM014, 11 candidate statuses, 18 unbuilt handoff entries, 21 physical gates and pass-disabled safety without MCU. Prior archive KiCad workflows remain disabled; output GPIOs stay uncommissioned. Remote VER028, Boat VER014 held, CHIRP sonar not implemented.

## R18 — distributed modular placement and Bulgarian-first verified electronics sourcing (2026-11-21)

**Owner’s new mechanical rule:** Previous 77×79mm carrier and 30mm clearance are **NOT active constraints**. Separately mounted modules may be distributed along the Boat hull in suitable protected positions; actual full hull balance/cooling/waterproofing still must be surveyed. **NO custom electronic PCBs**, factory modules and removable harness only. Active placement: `Boat/docs/BOAT_DISTRIBUTED_MODULE_INSTALLATION_R18.md`.

**Purchasing policy:** Official Bulgarian electronics suppliers/manufacturers (**Farnell Bulgaria, Comet Electronics, Olimex, Ardboard**) first. Verified specialized EU electronics shops/manufacturers may supply exact models where BG not confirmed (**Electrokit** for owner-chosen EKM014, **Lemona EU/Cytron** for MDD20A). **AliExpress is conditional only with verifiable authentic seller/product**; no listing currently approved. **OLX/eMAG/general marketplaces and anonymous copycat stores excluded.** Active audited evidence/register: `docs/APPROVED_ELECTRONICS_SUPPLIERS_R18.md`, `docs/VERIFIED_ELECTRONICS_SOURCING_R18.csv`.

**Verified exact product listings:** Farnell BG **RECOM R-78E3.3-0.5 #2218602**, no live Farnell stock proof; Comet RS #10923552 exact same RECOM part **0 in local warehouse, distributor order only**, not guaranteed stock; **Olimex DCDC6-16-TO5 6–16V→5V/2A in stock at Bulgarian manufacturer** (USB-A 5V output/barrel input, installation/actual Heltec-VL pin audit required), Ardboard **Heltec Tracker v1.2** exact product listing (stock unconfirmed). Exact EKM014 and MDD20A remain selected but no approved **Bulgarian** seller confirmed; known exact listings at reputable EU specialists, delivery to BG checkout must still be verified. Independent STOP/POR/no-repeat/current-zero modules remain unselected and no powered actuator commissioning is authorized. Sonar unselected. Remote VER028 / Boat VER014 unchanged.



## R19 — status reconciliation (2026-11-21)
Read [`docs/R19_STATUS_RECONCILIATION_AND_NEXT_GATES.md`](R19_STATUS_RECONCILIATION_AND_NEXT_GATES.md) before using the later R1/R2 modular draft documents. R15–R18 remain canonical: distributed prefabricated modules, EKM014 frozen, independent hardware STOP/one-hot/no-repeat/POR/current-zero required and not yet selected. R2 is NOT a final wiring authorization and direct driver battery feeds or MCU-only solenoid gating are unsafe/incomplete. Exact Heltec v1.2 GPIO crosswalk, SX1262-to-E22 interoperability, complete purchase-ready BOM and independent safety-module procurement remain next blockers. Remote VER028 / Boat VER014 held; no physical actuator enable.


## R20 — GPIO/radio integration and purchasing register (2026-11-21)
New [Boat R20 GPIO/LoRa integration gate](../Boat/docs/BOAT_R20_GPIO_AND_RADIO_INTEGRATION_GATE.md) records signal budget and precise SX1262/E22 interoperability qualification steps; numeric v1.2 GPIOs remain UNASSIGNED and backup LoRa disabled pending proof. New [R20 project purchasing register](PROJECT_PURCHASE_REGISTER_R20.csv) consolidates Boat/Remote modules, known exact supplier links, alternatives, evidence status and purchase blockers; no live-stock guarantees. R15–R18 canonical module-only safety requirements still apply. No firmware output enable, purchase, hardware commissioning or release version bump. Remote VER028 / Boat VER014 held.


## R21 — verified revision-source boundary and radio interoperability test (2026-11-21)
Added [Boat R21 evidence and radio test plan](../Boat/docs/BOAT_R21_REVISION_EVIDENCE_AND_RADIO_TESTS.md). Official publicly located Heltec/Zephyr pin documents describe v1.1, **not authenticated v1.2**. Numeric GPIO freeze is therefore BLOCKED until exact v1.2 schematic and hardware evidence. EBYTE E22-900T22S is SX1262-based but includes UART modem firmware; raw SX1262↔E22 RF compatibility remains UNVERIFIED, backup link DISABLED. No firmware enable, bench acceptance or purchase. R15–R18 safety architecture remains canonical. Remote VER028 / Boat VER014 held.


## R22 — executable fail-closed GPIO and LoRa gates (2026-11-21)
Added [host-side Python gate](../Boat/tools/r22_interface_gate.py) with seven unit tests and [R22 instructions](../Boat/docs/BOAT_R22_EXECUTABLE_INTERFACE_GATES.md). Enforces evidence for v1.2 pin allocation, detects pin conflicts and rejects unproven SX1262/E22 radio configurations until bidirectional authenticated hardware tests. No CI or physical tests claimed; actuator enable remains prohibited, safety modules unselected, Remote VER028 / Boat VER014 unchanged. Next: integrate tests in CI, obtain exact v1.2 board schematic, complete COTS safety procurement and harness qualification.


## R23 — host test execution and GitHub Actions (2026-11-21)
R22 seven Python unittest cases were executed in a local isolated environment and passed (7/7; no physical hardware). Added [GitHub Actions R23 interface gate](../.github/workflows/r23-interface-gates.yml), running Python 3.11 syntax validation and all R22 tests on relevant pushes, PRs and manual dispatch. **GitHub Actions run status is not yet confirmed**; do not equate workflow file commit with successful CI. GPIO v1.2, LoRa RF compatibility, COTS safety hardware and physical acceptance remain blocked. Remote VER028 / Boat VER014 held; sonar deferred.


## R24 — host-only physical commissioning safety gates (2026-11-21)
Added [R24 fail-closed commissioning checklist tests](../Boat/tools/r24_safety_gate.py), six unit tests covering 18 mandatory physical acceptance fields, absence of assembled hardware and human review, onehot/no-repeat, and deferred sonar scope. Expanded [GitHub Actions interface gate](../.github/workflows/r23-interface-gates.yml) to run both R22 and R24 host tests (13 cases total). No GitHub Actions success asserted; the commit-linked PR-only run lookup returned no runs for the earlier R23 workflow. No real safety hardware selected or certified. No GPIO or LoRa physical validation. Remote VER028 / Boat VER014 held.


## R26 — GPIO budget feasibility blocker (2026-11-21)
Added [R26 pin-budget engineering finding](../Boat/docs/BOAT_R26_PIN_BUDGET_BLOCKER.md) and [four arithmetic unit tests](../Boat/tools/r26_pin_budget.py); GitHub Actions workflow expanded to run them. On provisional 6 externally usable GPIO candidates, the minimal separate-SPI nRF24 + I2C + dual PWM interface requires 9 pins (shortfall 3), excluding DIR/feedback. Heltec v1.1 documented header does not expose onboard SX1262 SPI pins; v1.2 physical access is unverified, so shared SPI is **NOT** an approved solution. No firmware, harness, or actuator enable. Next engineering gate: verified v1.2 manufacturer schematic and a commercially premade interface option with proven timing. Remote VER028 / Boat VER014 remain held.


## R27 — premade module GPIO recovery feasibility (2026-11-21)
Added [Boat R27 pin recovery option matrix](../Boat/docs/BOAT_R27_PIN_RECOVERY_OPTIONS.md). Candidate A is commercial NXP SC18IS602B I2C-to-SPI bridge, potentially reducing external native GPIO demand to five by handling nRF SPI/CSN via bridge; nRF CE and two motor PWM outputs remain native, while DIR/requests need qualified expansion. No module purchase, bridge driver, hardware bench proof, v1.2 schematic verification, or safety hardware qualification has occurred. Alternative PCA9685 and manufacturer-confirmed additional header pins assessed. Remote VER028 / Boat VER014 held, sonar deferred.


## R28 — official Wireless Tracker V2 validation (2026-11-21)
Reviewed official V2 March 2026 datasheet and HTIT-Tracker-FEM_V2.2 schematic; findings at [Boat R28 V2 manufacturer validation](../Boat/docs/BOAT_R28_HELTEC_V2_MANUFACTURER_VALIDATION.md). V2 has SX1262/UC6580 and a high-power LoRa RF front end reserving GPIO4/5/7; GPIO6/17/37 are conservative candidate external pins, not tested. Separate nRF SPI/I2C/dual PWM requires nine native pins, hence at least six short; SC18IS602B bridge alone still needs five pins, hence at least two short. **V2 is evaluated but NOT automatically adopted**, as it does not resolve GPIO. Current v1.2 remains provisional; exact pin mapping, COTS safety, LoRa interoperability, power acceptance and physical tests are unresolved. No release changes, Remote VER028 / Boat VER014 held, sonar deferred.


## R29 — Screenless Wireless Tracker V2 pin recovery (2026-11-21)
Owner confirms onboard TFT will not be used. [R29 screenless pin budget and proposed allocation](../Boat/docs/BOAT_R29_V2_SCREENLESS_GPIO_RECOVERY.md) identifies nine *candidate* native signals: SPI 41/42/40, CE/CSN 43/44, I2C 6/17, dual PWM 21/37; TFT CS38 and RESET39 held inactive. This potentially removes R26/R28 GPIO shortage and SC18IS602B requirement. **Not wiring-ready:** TFT remains electrically connected, GPIO21 also drives backlight, GPIO43/44 are boot UART, TFT CS boot default and all waveform/fault tests unresolved. No migration completed, no actuator energization, releases held.


## R30 — screenless V2 electrical conflict gate (2026-11-21)
[Review](../Boat/docs/BOAT_R30_SCREENLESS_V2_ELECTRICAL_GATES.md) and [static test](../Boat/tools/r30_v2_pin_conflict_gate.py) committed, with CI workflow entry. R29's nine unique GPIOs are NOT nine electrically independent pins: GPIO21 is TFT backlight and rejected for motor PWM pending isolation evidence; GPIO40–42 share TFT nets; GPIO43/44 are UART0 at boot. The candidate mapping remains **NOT APPROVED**. Need manufacturer net trace and physical boot/reset measurements, alternate PWM pin/module and independent actuator safety stack. No migration or release authorized.


## R31 — Actual firmware GPIO contract audit (2026-11-21)
[Boat R31 source audit](../Boat/docs/BOAT_R31_ACTUAL_FIRMWARE_GPIO_AUDIT.md) examined boat_radio_hw_config.h, mdd20a_arduino_stage.cpp, and firmware/Flytec_Boat.ino. Existing direct motor HAL requires **six** native GPIOs (PWM2, DIR2, permit, watchdog edge), so full direct native budget is at least **13**, not the nine in R26/R29 (which assumed offloading). Current boat sketch remains non-actuating. Screenless V2 has no approved complete mapping; GPIO21 backlight and UART/TFT shared nets remain blockers. Need qualified COTS expander and firmware HAL redesign, board evidence and physical tests before migration. Releases held.


## R32 — executable V2 screenless GPIO budget (2026-11-21)
Added [R32 V2 assignment solver](../Boat/tools/r32_v2_assignment_solver.py) with eight arithmetic/constraint tests and CI step. Source-backed motor HAL requires six direct control lines. On conservative screenless V2 candidate set excluding TFT backlight GPIO21, eight provisional GPIOs remain. Direct firmware demand 13; direction-only offload 11; direction+permit+watchdog offload 9 (still one short); SPI bridge plus direction offload 7 (arithmetically fits, but bridge timing, boot states and independent STOP not verified). Do not equate offloading MCU permit/watchdog to satisfying independent safety hardware. No physical or CI results asserted; releases remain held.


## R33 — actual premade bridge/expander sourcing (2026-11-21)
[Boat R33 sourcing and feasibility](../Boat/docs/BOAT_R33_COTS_BRIDGE_SUPPLIER_VALIDATION.md): NXP SC18IS602B is **discontinued**, superseded as preferred candidate by active SC18IS606 and factory-built SC18IS606-EVB (stock not verified). Comet BG lists MIKROE-3743 obsolete bridge (0 local, 7 distributor) and MIKROE-1838 MCP23017 expansion board (0 local, 24 distributor), with dated pricing. No purchase. RF24 library is not plug-compatible with I2C-to-SPI bridge; transport adapter and RF timing tests needed. Existing six-direct-GPIO motor HAL must be redesigned for expander. GPIO mapping and safety unresolved; no physical acceptance or releases.


## R34 — non-actuating I2C direction policy (2026-11-21)
Added [pure motor-direction planning header](../Boat/source/mdd20a_i2c_direction_plan.h), [C++ assertions](../Boat/tests/mdd20a_i2c_direction_plan_test.cpp), and CI compilation/run step. The planner refuses commands without commissioning, STOP, isolated motor supply, zero PWM and healthy bus. Direction commits require independent physical feedback, not just I2C ACK. **No I2C bus driver, no GPIO writes, no motor HAL integration and no physical acceptance**; these are host policy tests only. Attempted local compile was blocked by container DNS when retrieving GitHub files; CI result not verified. Bridge nRF transport remains unimplemented, official releases held.


## R35 — first MCP23017 direction register adapter (2026-11-21)
Added [MCP23017 direction adapter](../Boat/source/mcp23017_direction_adapter.h) and [fake-bus host tests](../Boat/tests/mcp23017_direction_adapter_test.cpp), plus CI test step. Adapter uses bank-0 MCP23017 registers, writes output latch low before enabling GPA0/GPA1, reads GPIOA back, refuses direction changes without explicit motor-power-isolated and PWM-zero flags, and fails closed on I2C errors. **Limitations:** GPIOA readback is not external physical direction feedback; external pull-downs, startup POR behavior, module-level voltage compatibility, real Wire transport, independent safety contactor, and V2 pin map remain unverified. Existing boat sketch stays non-actuating. Latest observed GitHub Actions R34 tests and diagnostic boat target succeeded; R35 result must be checked separately. No hardware-ready release.


## R36 — OWNER-SELECTED BOAT HARDWARE MIGRATION (2026-11-21)
**Latest decision supersedes earlier Heltec boat controller selection:** Boat uses modular ESP32-S3-DevKitC-1 + HGLRC M100 PRO GPS + external EBYTE E22 UART LoRa (exact board/GPS variant and E22 SKU require confirmation), plus nRF24 PA/LNA primary, MDD20A motors and owner-frozen EKM014 solenoids. See [R36 migration spec](../Boat/docs/BOAT_R36_MODULAR_ESP32S3_GPS_E22_MIGRATION.md), [updated boat_config.h](../Boat/source/config/boat_config.h), [radio defaults](../Boat/source/boat_radio_hw_config.h) and [purchase register](PROJECT_PURCHASE_REGISTER_R20.csv). Previous Heltec V1.2/V2 pin and SC18IS606 bridge investigations are historical, NOT active wiring. Existing onboard SX1262 BoatDualRadioPort implementation requires actual E22 UART migration; R36 is hardware selection/config/documentation only, not completed target software. All pins disabled, actuator supply disconnected, safety hardware unresolved; no release.


## R37 — E22 board adapter scaffold and remaining work (2026-11-21)
Added [external E22 UART board adapter](../Boat/source/e22_uart_board_port.h), with explicit audited pin and RF/voltage verification gates, and [R37 execution gate list](../Boat/docs/BOAT_R37_MIGRATION_EXECUTION_GATES.md). It is **NOT integrated into BoatDualRadioPort**, which still uses HeltecSx1262Link; existing E22 UART framing requires physical peer verification and airtime measurement. GNSS M100, exact ESP32-S3 pinout, safety stack, complete compile and hardware acceptance are pending. No actuator enable or release.


## R38–R39 — E22 migration and GNSS gates (2026-11-21)
R38 replaced BoatDualRadioPort's onboard HeltecSX1262Link with external E22UartBoardPort using existing E22UartLink and retained authenticated cross-radio command policy. Added fail-disabled audited E22 UART and nRF SPI pin gates in boat_radio_hw_config.h; no pins are enabled. R39 added [HGLRC M100 PRO profile gate](../Boat/source/hglrc_m100_pro_profile.h) and [host tests](../Boat/tests/hglrc_m100_pro_profile_test.cpp), registered in CI. [Wiring and safety acceptance](../Boat/docs/BOAT_R39_WIRING_AND_PHYSICAL_ACCEPTANCE.md) identifies missing exact board/SKU and factory-built independent safety stack. Host tests for earlier R38 change succeeded; R39 and target compilation were still running at last check. GPS real NMEA, E22 RF, pin freeze, actuator and safety physical testing NOT performed. No release.


## R40–R41 — GY-91 BMP280 weather + magnetometer calibration (2026-11-21)
- Implemented BMP280 chip-ID probe, factory coefficient decoding, compensated pressure Pa and temperature centi-C, stale-data rejection in Boat sensor port. HGLRC M100 PRO remains GPS source.
- Added shared eight-byte weather telemetry subtype 7 to Boat and Remote, freshness gating and Bulgarian weather readout in sonar panel.
- Added non-actuating magnetometer min/max sweep candidate (start/finish by serial c/f) and portable tests. No automatic calibration approval or autopilot enabling; approved offsets/matrix must be entered into signed heading profile after field verification.
- Added regression gates for BMP280 Bosch example, weather packet roundtrip and compass candidate. CI status must be checked on latest head; earlier R41 compile warning corrected. Prior portable suites passed, final target build and physical acceptance still pending.
- Relevant documents: Boat/docs/BOAT_R40_BMP280_REMOTE_WEATHER.md and Boat/docs/BOAT_R41_GY91_COMPASS_CALIBRATION.md.
- Continue with target compile, correct any errors, confirm real ESP32-S3/M100/E22/GY-91 GPIO wiring and independent physical STOP/coil safety. Actuator power remains disconnected until hardware commissioning.


## R42 — Figure-eight-only dual compass calibration (2026-11-21)
User-approved on-water figure-eight only, superseding R41 three-axis on-land operator procedure. HGLRC M100 PRO manufacturer confirms onboard QMC5883L magnetometer (I2C), but 2026 production reports some QMC5883P substitutions; R42 driver supports QMC5883L only, rejects unrecognized chip. Added separate GY-91 AK8963 and M100 compass horizontal sweep candidate collection, coverage tests and CI gate. No automatic apply/persistence, remote calibration screen or actual on-water verification. See [R42 figure-eight specification](../Boat/docs/BOAT_R42_FIGURE8_DUAL_COMPASS.md). Important separate blocker: M100 PRO GNSS uses UBX 115200, and existing GNSS parser must be checked/adapted; do not claim GPS fix works yet.


## R44 — Autonomous figure-eight requirement (2026-11-21)
User explicitly requires **NO manual driving** for compass calibration. Added portable `Boat/source/figure8_autonomous_motion.h` generating differential-thrust trajectory intent, fail-closed physical safety/primary radio/GNSS/geofence/watchdog gates, timeout, abort, override and host CI test. This is **not wired to the actuator driver**; the diagnostic firmware cannot actuate motors. See `Boat/docs/BOAT_R44_AUTONOMOUS_FIGURE8.md`. Remaining: closed-loop GNSS heading trajectory, authenticated remote start/abort UI/protocol, independent physical safety hardware, validated dual compass correction and atomic persistence, hardware water acceptance. Do not claim end-to-end automatic calibration finished.


## R45 — Owner-selected 32MB ESP32-S3-DevKitC-1-N32R16V (2026-11-21)
- Final boat controller SKU is Espressif **ESP32-S3-DevKitC-1-N32R16V**, WROOM-2, **32MB flash / 16MB PSRAM**; no Heltec or 16MB substitution. Updated `Boat/source/config/boat_config.h`.
- Critical datasheet: **GPIO47/48 are 1.8V**, unlike standard 3.3V GPIO; GPIO35/36/37 reserved by octal memory. See `Boat/docs/BOAT_R45_N32R16V_PIN_AND_POWER_PLAN.md` for **provisional** J1/J3 header pin schedule, radio/GPS/I2C/motor and independent safety feedback and protected power branches. No pins commissioned.
- HGLRC M100 PRO manufacturer says **UBX at 115200**. Added checksum-validated NAV-PVT parser `Boat/source/m100_ubx_nav_pvt.h`, wired into sensor telemetry for 115200 profile, regression `Boat/tests/m100_ubx_nav_pvt_test.cpp`, CI workflow. GNSS hardware output and real 3D fix remain untested.
- Added `docs/datasheets/R45_MANUFACTURER_PDF_REGISTER.md` with three verified direct Espressif PDF URLs and remaining manufacturer documentation. **Binary PDFs not uploaded** (GitHub connector supports UTF-8 text; container network fetch failed).
- CI portable R45 regression passed on observed run 44945861528; most recent target compile still running on observed run 44945819892. Physical hardware interlocks, complete actuator wiring, automatic closed-loop figure-eight, remote command/status, radio RF interoperability, water tests, final BOM purchase prices, official Boat release/ZIP **remain open**. Do not publish verified Boat release prematurely.


## R47 — Boat Wi-Fi OTA maintenance scaffold (2026-11-21)
- Owner requests boat firmware OTA by Wi-Fi or Bluetooth. Implemented optional **Wi-Fi HTTPS OTA** module `Boat/source/boat_wifi_ota.h` with physical maintenance/power-isolation/STOP, Secure Boot V2 and flash-encryption gates; disabled by default and not wired to active boat application. Portable regression `Boat/tests/boat_wifi_ota_preflight_test.cpp` and CI added.
- ESP32-S3 has Bluetooth LE but not Classic; BLE firmware transfer deferred. No unsecured ArduinoOTA upload endpoint enabled.
- OTA cannot be considered operational until secure boot/OTA partitions/rollback, local maintenance controls, credential provisioning, exact target compile and physical update/recovery tests. See `Boat/docs/BOAT_R47_WIFI_OTA.md`.
- R46 hardware driver, independent safety modules, remote calibration, autonomous GPS track and physical acceptance remain pending. Official releases held.


## R48 — five-stage integration gates (2026-11-21)
- Added `Boat/source/physical_driver_commissioning_gate.h`: local independent STOP/watchdog/motor+coil contactor feedback and current-zero safety preflight, with 100ms sample freshness and fail-closed single-coil pulse cap. This is an adapter prerequisite, not a commissioned `BoatPhysicalDriver` or independent electrical safety hardware.
- Added `Boat/tests/physical_driver_commissioning_gate_test.cpp` to host CI. Observed host run 44947095178 PASS; diagnostic target run 44947079254 still in progress when checked.
- Added `Boat/docs/BOAT_R48_INTEGRATION_ACCEPTANCE_MATRIX.md`: five-stage source status, hardware selection criteria, safety wiring and physical acceptance checklist. No exact independent safety parts selected or physical tests done. R45 pin schedule remains provisional. GPS-guided figure-eight, persistent calibration, Remote Start/Abort, integrated Boat+Remote target build and RF tests remain open. Boat VER014 remains official.


## R49 — EKM014 software pulse guard (2026-11-21)
- Added `Boat/source/ekm014_pulse_guard.h` and `Boat/tests/ekm014_pulse_guard_test.cpp`, registered in host CI. Single-coil, max 500ms configurable pulse, fresh power/current/independent limiter feedback and post-pulse current-zero before retrigger. **Uncommissioned and not wired to GPIO expander**; physical hardware limiter mandatory.
- `Boat/docs/BOAT_R49_EKM014_IMPLEMENTATION.md` details limitations and integration dependencies. CI latest run 44947273124 was in progress at last check. Boat/Remote full target integration, RF, independent hardware safety and water tests remain pending. Official Boat VER014 unchanged.


## R50 — physical driver adapter and regression repair (2026-11-21)
- Added `Boat/source/boat_physical_driver_adapter.h`, transport-neutral callback adapter with real-sample-only safety feedback, MDD20A motor and EKM014 single-pulse output contract, no default GPIO. Added `Boat/tests/boat_physical_driver_adapter_test.cpp` to CI.
- R49 CI run 44947273124 failed its EKM014 test; corrected stale timestamp test at commit 9c216df049eee2e30edbc85636c955e8b3f0845c. R50 CI run 44948760293 was in progress when checked; no pass claim.
- See `Boat/docs/BOAT_R50_PHYSICAL_DRIVER_ADAPTER.md`. No physical driver attached to running firmware, no independent safety hardware verified, no full Boat/Remote integration or physical acceptance. Boat VER014 remains official.


## R51 — ESP32-S3 GPIO validation (2026-11-21)
- Added `Boat/source/esp32s3_gpio_audit.h` and `Boat/tests/esp32s3_gpio_audit_test.cpp`, registered host CI. Blocks reserved octal flash/PSRAM, strapping, native USB, UART0 and 1.8V GPIO47/48, requires audited non-overlapping motor pins. See `Boat/docs/BOAT_R51_GPIO_AUDIT.md`.
- R50 diagnostic target compile 44948745058 PASS; host 44948760293 FAIL at R49 EKM014 regression. The timestamp regression correction was committed subsequently; no fresh host PASS observed at checkpoint update. No physical pinout approval, actuator enabling, or verified release.


## R52 — safety feedback and calibration record (2026-11-21)
- Corrected `Boat/tests/ekm014_pulse_guard_test.cpp` to refresh sample timestamp during the simulated active pulse; corrected physical adapter motor restart condition after coil pulse in `Boat/source/boat_physical_driver_adapter.h`.
- Added `Boat/source/figure8_calibration_record.h` with versioned CRC32, finite/range validation and valid-record dual-slot selection; `Boat/tests/figure8_calibration_record_test.cpp` added to host CI. **No NVS/SD storage backend yet**, no live calibration activation.
- Last confirmed pre-R52 CI: host 44951517101 PASS; ESP32-S3 diagnostic target 44951495536 PASS. R52 runs not yet verified. Details: `Boat/docs/BOAT_R52_CALIBRATION_AND_DRIVER.md`.
- Next: physically verified EKM014 and safety module wiring, commissioned BoatPhysicalDriver HAL, actual dual-slot NVS persistence, GPS-guided figure-eight and authenticated Remote Start/Abort, full target builds and hardware acceptance. Boat VER014 frozen; sonar deferred.


## R53 — dual-slot calibration storage interface (2026-11-22)
- Added `Boat/source/figure8_calibration_store.h` with dual-slot read/write callbacks, CRC-validated sequence selection, inactive-slot write and read-back verification; `Boat/tests/figure8_calibration_store_test.cpp` covers recovery and failures; CI updated. Documentation: `Boat/docs/BOAT_R53_CALIBRATION_STORAGE.md`.
- **No physical NVS/SD backend or activation yet**; no GPS-guided autonomous figure-eight, Remote Start/Abort, independent safety hardware or commissioned physical driver. R52 host 44951795285 PASS and diagnostic target 44951779135 PASS; R53 build 44954429335 in progress when checked.
- Next implement ESP32-S3 NVS backend and rollback tests, integrate authenticated calibration protocol and GPS trajectory only after hardware safety acceptance. Boat VER014 frozen, sonar deferred.


## R54 — external microSD boat module (2026-11-22)
- Confirmed architecture uses **external ready-made SPI microSD breakout**, since ESP32-S3-DevKitC-1-N32R16V has no integrated card socket. No custom PCB. Provisional SPI SCK=GPIO12, MOSI=GPIO11, MISO=GPIO13 shared with nRF24, dedicated SD CS=GPIO33 (NOT signed/verified). 3.3V-only logic, inactive CS and MISO tri-state must be verified. See `Boat/docs/BOAT_R54_EXTERNAL_MICROSD.md`.
- Added `Boat/source/boat_sd_module_profile.h`, `Boat/source/boat_sd_module_arduino.h`, `Boat/source/boat_sd_calibration_backend.h/.cpp` and `Boat/tests/boat_sd_module_profile_test.cpp` to CI. Dual-file calibration persistence adapter is implemented but **not attached to running firmware**; no SD card or shared-radio SPI hardware test. FAT writes are not power-loss atomic. Existing host and target CI status for R54 not yet verified.
- Next: verify purchasable 3.3V microSD breakout, DevKitC GPIO33 and nRF24 SPI arbitration; wire optional diagnostic SD initialization under signed pin audit; test actual card and calibration recovery; finish physical driver and independent safety hardware. Official Boat VER014 unchanged.
