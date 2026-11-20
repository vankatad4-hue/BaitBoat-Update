# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current committed snapshots — Remote ver020 / Boat ver007
- Remote marker: `Remote/releases/VERSION` = `020`.
- Boat marker: `Boat/releases/VERSION` = `007`.
- ZIP files: `Remote/releases/Flytec_Remote_ver020.zip`, `Boat/releases/Flytec_Boat_ver007.zip`.
- Manifest: `releases/SHA256SUMS_ver020_007.txt`.
- SHA256 entries: Remote `0f077cbfb5d79b5dfbc1713575b9575607e6d66a6b98d7365cb62e36784288b8`; Boat `55b5b9c69fb7f8210fd512e9f2c725f5b6b2fbc2b8529f7a5873bd3328dc7736`.
- Archive paths and manifest were confirmed in GitHub. Remote ZIP payload was not retrieved for local extraction; Boat ZIP base64 was retrieved but not extracted and checksum compared. Treat as generated snapshots with independent integrity verification pending.

## Next snapshots — Remote ver021 / Boat ver008
- Markers are `021` and `008`.
- Manifest `SHA256SUMS_ver021_008.txt` is present, and Boat ver008 ZIP is present.
- Remote ver021 ZIP was not found in the branch at this check. The paired snapshot is incomplete pending the packaging workflow.

## Technical validation boundary
Archives are source/documentation snapshots only. Full ESP32 firmware compile, native schematic/ERC/DRC, PCB, RF/antenna, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
