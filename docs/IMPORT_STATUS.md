# Repository Continuation / Import Status

**Canonical repository:** https://github.com/vankatad4-hue/BaitBoat-Update  
**Development branch:** `hardware-ready-dual-radio-r1`  
**Updated:** 2026-11-20

This repository is the authoritative project workspace. Do not start from older downloaded ZIPs; synchronize against the branch and read the checkpoint.

## Start here
1. [Continuation Checkpoint VER001](CONTINUATION_CHECKPOINT_VER001.md) — selected architecture, current implementation, verification limits and next steps.
2. [Project Progress Review](PROJECT_PROGRESS_REVIEW_R1.md) — engineering blockers and gates.
3. [Remote Master Status](../Remote/docs/PROJECT_MASTER_STATUS.md)
4. [Boat Master Status](../Boat/docs/PROJECT_MASTER_STATUS.md)
5. [Remote Consolidated Changelog VER021](../Remote/releases/CHANGELOG_MASTER_VER021.md)
6. [ZIP Integrity Status](ZIP_BUILD_STATUS_R1.md) and [ZIP Policy](ZIP_RELEASE_POLICY_R1.md)

## Layout
- `Boat/source/`: boat firmware scaffold.
- `Boat/docs/`: electrical/hardware/pin audits and boat status.
- `Boat/tests/`, `Boat/hardware_test/`: portable tests and diagnostic sketches.
- `Remote/source/`: remote firmware, LVGL UI, radio/input/map modules and previews.
- `Remote/docs/`: map, mission, UI, electrical and project status.
- `Remote/New_Handheld_Remote/`: CAD/mechanical/PCB documentation.
- `Remote/tests/`, `Remote/hardware_test/`, `Remote/tools/`: test assets, probes and map utilities.
- `docs/`: shared checkpoint, progress review, import and packaging records.

## Version and release rules
- Current source markers Remote 028 and Boat 014 are unchanged; ver029 is unreleased development.
- Existing branch ZIPs are unverified snapshots because the previous workflow overwrote version filenames.
- Packaging guard now only preserves a versioned archive when a release marker changes; see ZIP status for latest workflow evidence.
- Never infer hardware readiness from source or archive generation.

## Working rule
Inspect the exact source/document before editing; commit to the development branch; run available tests and report exact evidence; update checkpoint, statuses and consolidated changelog after substantial work. Keep safety gates fail-closed.
