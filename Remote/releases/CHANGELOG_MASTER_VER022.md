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
