# Consolidated development changelog — VER022

**Date:** 2026-11-20  
**Branch:** `hardware-ready-dual-radio-r1`  
**Scope:** Source consistency audit and compile preparation. Development update only; no package marker bump.

## Source audit
- Added `docs/SOURCE_CONSISTENCY_AUDIT_R1.md` with findings, severity, current integration boundaries, verification plan and target compile gate.
- Confirmed missing reproducible target build manifest/dependency lock; exact board SKUs/revisions and dependency APIs are unresolved.
- Confirmed no Remote host-test workflow, and Boat portable tests do not cover full dual-radio port/session/ACK integration.
- Boat sketch remains a non-actuating placeholder. No application executor, real telemetry, failsafe executor or actuators.
- Secure key provisioning remains disabled/fail-closed; E22 airtime tracking is estimated/RAM-only; online map provider is not implemented.

## Source correction — session-start reliability
- Remote sends authenticated session-start markers with per-link retry until confirmation: nRF primary retry 500 ms; E22 backup retry 3 seconds and subject to duty limiter.
- Boat now responds to repeated authenticated same-session markers so a lost reply does not leave the remote waiting indefinitely.
- Commits: Remote `849ceb09c16956ce6218006ee50314365663961a`; Boat `636f7a9f74f59e4005e5bb5eb753deddedca196d`.

## Validation status
- Source review performed; fixes are not target-compiled or integration-tested.
- No complete host test run is claimed in this update.
- No firmware release ZIP regenerated; Remote VERSION marker remains 028 and Boat VERSION remains 014.
- Current branch ZIP files remain unverified snapshots.

## Next
1. Confirm exact Remote display SKU/revision, Boat board revision and pins.
2. Pin board/core/library versions and add reproducible target build configuration.
3. Add Remote CI for portable tests and Boat fake-radio integration tests for session retries/ACK/queue.
4. Compile all source modules, fix diagnostics, then LVGL simulator and hardware bring-up gates.

## Hardware selection confirmed
- User confirmed Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948 for the Remote. 4.3B/BOX alternatives are excluded.
- Selection is fixed; physical verification on receipt is pending. This does not change release markers.

## Dependency compatibility investigation
- User-confirmed Remote target: Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948.
- Official older Arduino demo path references LVGL 8.x; official newer LVGL9 path is BSP-based (LVGL 9.5.0 example). The project currently requires LVGL 9.6.x.
- Build manifest remains unfrozen pending source/API compatibility verification. No compile claim or marker bump.

## Target build preparation and adapter audit

- User confirmed Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948. No board variant substitution.
- Added source-level LVGL 9.6 adapter audit and candidate build configuration documents.
- Added GitHub Actions Arduino CLI target compile workflow with candidate pins (ESP32 core 3.0.2, LVGL 9.6.0, Display Panel 1.0.0, IO Expander 1.0.1, esp-lib-utils 0.1.2). Build run is in progress; versions are not yet validated as a resolved set.
- Inspected the first Remote host-test CI failure: release-guard fixture did not set required fail-closed safety fields. Corrected fixture; re-run pending.
- Display-port assumptions needing compile confirmation: vendor `drawBitmap()` return type/copy completion; `Touch::readPoints()` signature; board begin/init API; LVGL flush/buffer/input signatures. Touch orientation and physical SD/backlight behavior remain hardware-test gates.

### CI diagnostics and fixes
- Target compile attempt #1 failed at Arduino sketch discovery (folder/main-INO naming mismatch), not C++ compilation. Workflow staging was corrected and log artifact capture added.
- Host release-guard test initially had a mismatched reduced BoatState struct in its test fixture; corrected to link the real state source.
- Payload codec test contained an invalid latitude vector that represented 50° rather than 91°. Test vector corrected; rerun pending.

### CI test fixes and results
- Host run #7 passed: release guard/scanner syntax, radio payload codec, ACK tracker and telemetry freshness. Fixes included real BoatState layout in the test, a correct 91° latitude invalid vector, and braces around telemetry pair timestamp conditionals to satisfy strict warning checks.
- Target run #3 reached sketch compilation but failed because LVGL configuration was not provided. Added tracked `Remote/source/lv_conf.h` from LVGL v9.6.0 template (project pool 128 KiB) and workflow installation step. Target retry pending.

- Target compile attempt #4 exposed missing RF24 Arduino library (`RF24.h`). Pinned RF24 1.4.11 in the target workflow. Retry pending.

- Source hardening: flush callback now guards a null LVGL display handle before signaling flush completion.

- LVGL 9.6 target compiler findings fixed: echogram line points use `lv_point_precise_t`; live joystick/safety footer label pointers are declared. Full compile retry pending.


## Target compiler diagnostics and source fixes — 2026-11-20
- Target build run #11 reached project compilation and failed with diagnostics in joystick calibration UI, map renderer and map manager.
- Corrected the LVGL9 event target accessor to `lv_event_get_target_obj()`; updated route line points to `lv_point_precise_t`; restored the Map Manager `copyChunk()` helper that had been commented out by a literal escaped newline.
- Exact installed candidate versions observed: ESP32 Arduino core 3.0.2, LVGL 9.6.0, ESP32_Display_Panel 1.0.0, ESP32_IO_Expander 1.0.1, esp-lib-utils 0.1.2, RF24 1.4.11.
- Commits: `1afd17d5`, `00840087`, `7213f04e`. Retry compile pending; no compile success claimed.


## Corrective rebuild launched — 2026-11-20
- Target compile run #14 (ID 44776348680) is in progress on head `7213f04e`, including all three fixes above. Host-test run #14 on that SHA passed.
- Await compiler completion before updating readiness or claiming full target compile.


### Documentation synchronization HEAD
- Branch HEAD after checkpoint/status synchronization: `04d9a9ff59ac10894a80951b2d5a548f66f9f22e`. No firmware source changes occurred after target compile run #14 started; it compiles source SHA `7213f04e4d18f3bc4a546f4c4febfe0bbedffc59`.
