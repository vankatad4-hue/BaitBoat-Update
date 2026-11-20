# ZIP Build Status — 2026-11-20

## ver005 artifacts — verified
PR workflow run #32 completed successfully and produced both versioned ZIP artifacts and a SHA-256 checksum artifact. Artifacts were downloaded, ZIP integrity checked, extracted, and SHA-256 values matched the manifest.

- `Flytec_Remote_ver005.zip` SHA-256: `6b5f37b65b240812facf00775f221b66c45d76604fba2b493d511785ece67654`
- `Flytec_Boat_ver005.zip` SHA-256: `d1db0d55cd9c95157deacc55002dded8177615572300a3721c23e9321f2beead`
- Workflow run: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44696686990

ver005 archives are downloadable from the current ChatGPT response. Their PR artifacts expire on 2027-02-18; local download copies are provided. The ver004 ZIPs in the repository remain preserved.

## ver006 — pending
After ver005 artifacts were generated, package status documentation and release version markers were updated. Both `Remote/releases/VERSION` and `Boat/releases/VERSION` now read `006`.

The packaging workflow was updated to commit generated ZIPs and checksum manifest to the branch on push events, and upload artifacts on PR events. Latest PR workflow run #53 is in `action_required` status and has no artifacts yet:
https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44697037116

Approve the pending workflow run in GitHub Actions, then verify its ZIPs/checksums before treating ver006 as generated/released.

## Technical validation boundary
These ZIPs are source/documentation snapshots only. Arduino compile, native schematic/ERC/DRC, hardware bench validation and operational safety tests have not been completed. No motor/solenoid connection or water operation.
