> **CURRENT R30H-13 PlatformIO source migration (2026-11-23):** All **208 active** Boat/Remote source files covered by [repository-wide audit](R30H_13_PLATFORMIO_SOURCE_MIGRATION.md); 17 exact shared protocol pair checks. Remote obsolete direct-SPI RF24 code archived, dead Remote RF24 dependency removed, Boat nRF TX 120 ms software watchdog/fail-closed transport handling added. VS Code multi-root tasks and optional local licensed LVGL font staging available. Both endpoints remain PlatformIO **compile-only**, independent physical STOP unverified; all 18 R30H-10 + 12 R30H-12 physical evidence gates unsigned. Versions Boat **014** / Remote **028** frozen. CI results must be read per last commit; target build PASS is not hardware acceptance.

> **R30H-11 — source-only radio-bridge addition, not a release:** Remote NXP SC18IS606-EVB I²C-SPI candidate driver added; current RF hardware-gate flags all remain 0 and neither VERSION marker changed (Boat 014 / Remote 028). Development ZIP packaging does NOT imply physical RF or production acceptance. [Technical bridge handoff](../Remote/docs/REMOTE_R30H_11_ISOLATED_NRF_SPI_BRIDGE.md).

> **R30H-10 documentation/GPIO reconciliation (2026-11-22):** Boat version marker remains 014, Remote marker remains 028. [Packaging run #45066535756](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066535756) completed for the GPIO source revision as a **development artifact**, not a certified immutable VER015/VER029. Neither marker was bumped; no firmware was uploaded or hardware approved. Current GPIO authority is [joint review](R30H_10_GPIO_RESOURCE_EVALUATION.md), and latest project authority is the [continuation checkpoint](CONTINUATION_CHECKPOINT_VER001.md). Independent STOP/current-zero and 18 unsigned physical gates still prohibit release promotion.

# ZIP Build / Integrity Status — 2026-11-20 checkpoint

## Current package markers (not operational release approvals)
- Remote: `Remote/releases/VERSION` = **028**
- Boat: `Boat/releases/VERSION` = **014**
- Latest source UI work is unreleased ver029 development. Do not change markers for routine documentation synchronization.

## Archive integrity caution
The previous GitHub Actions workflow repeatedly regenerated archives under unchanged version names on development pushes. Consequently, branch ZIPs named ver028/ver014 must be treated as **unverified snapshots**, not immutable verified releases. Their original historical integrity cannot be inferred from the current filenames or manifests.

## Workflow safeguard
- Workflow: `.github/workflows/package-flytec-zips.yml`
- Development pushes build ZIP/checksum artifacts and upload them (90-day retention).
- Versioned archive copies are committed only when a push changes Remote/Boat VERSION marker(s).
- Previous guard validation: run **#880 success**, head `9f7b0344`, with preserve step skipped because no VERSION changed. Documentation synchronization runs **#888 and #889 succeeded**; #889 is the latest observed run (head `e68257eff1`). These were packaging workflow successes, not target firmware builds.
- Run: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44748284171

## How to issue a new snapshot/release safely
1. Inspect changes and tests; select which project folder(s) belong in package.
2. Increment the relevant VERSION marker by exactly one (Remote 029 and/or Boat 015, if the contents are being released).
3. Push the marker bump; wait for the package workflow.
4. Inspect run logs/artifacts; verify ZIP integrity (`unzip -t`), file list, no nested archives, and SHA-256 against manifest.
5. Download the produced artifact and independently compare its checksum. Confirm archive content matches commit/marker.
6. Update this status file and relevant BUILD_STATUS / changelog with run URL, commit, exact hash and test evidence.
7. Never claim compiled firmware, hardware-ready or operational based only on ZIP packaging.

## Older recorded evidence
Previous docs report historical ver005 artifact checks and ver024/ver014 snapshots; however, those records predate the workflow archive overwrite issue and are not a substitute for verifying the currently present branch binaries. Preserve historical notes, but rely on a fresh clean package for current release evidence.

## Validation boundary
No complete ESP32 target compile, LVGL Pro simulation, native KiCad ERC/DRC, hardware, RF, power/thermal, actuator or water validation is confirmed. No motor/solenoid operation.


## Development compile-fix snapshot — 2026-11-20
- Remote target compile run #11 failed at C++ compilation. Three source fixes were committed (LVGL event accessor, map route precise point type, Map Manager copyChunk declaration).
- Rebuild pending. These are development commits; release markers remain Remote 028 / Boat 014. No release ZIP regenerated, and existing branch ZIPs remain unverified snapshots.


## Target Compile #14 / #15
Compile #14 (`44776348680`) failed in C++ source compilation due to missing `g_map_attribution` declaration. Source correction commit `9aa20b0d987f94ea5b49e46509ae0ba10cc9d36b` adds a private 256-byte buffer. Target Compile #15 pending. ZIP/package workflow success must not be treated as firmware target compile success.


## Latest target build (2026-11-20)
- Target compile run #19 / ID 44781879101 passed on commit 3f189cb1ac65a764aec678893f1df857d113c815. Full Remote sketch compiled/linked and firmware artifact uploaded.
- Corrected the application partition selection to `PartitionScheme=huge_app` for ESP32-S3 16MB flash/OPI PSRAM; workflow exports binaries.
- Sketch 1,430,957 bytes (45% of 3,145,728-byte app partition); global RAM 178,136 bytes (54%, 149,544 bytes remaining). No C++ errors/warnings reported.
- Build configuration is CI-verified for compile, but exact physical SKU/revision, LCD/touch/SD/Wi-Fi runtime and hardware API behavior remain unverified. This is not a release qualification.
- Next: audit Wi-Fi/settings timer lifecycle, map import validation and renderer memory/cache behavior; then re-run target build after fixes.
