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
