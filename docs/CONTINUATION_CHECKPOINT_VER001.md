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
