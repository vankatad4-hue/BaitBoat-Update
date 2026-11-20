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
- Previous guard validation: run **#880 success**, head `9f7b0344`, with preserve step skipped because no VERSION changed. Current docs-only push triggered run **#888 pending** at head `c11f9048c1`; check it before claiming latest workflow success.
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
