# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current committed snapshots — Remote ver021 / Boat ver008
- Remote marker: `Remote/releases/VERSION` = `021`.
- Boat marker: `Boat/releases/VERSION` = `008`.
- ZIP files are present: `Remote/releases/Flytec_Remote_ver021.zip`, `Boat/releases/Flytec_Boat_ver008.zip`.
- Manifest: `releases/SHA256SUMS_ver021_008.txt`.
- SHA256 entries: Remote `2f1c0cc8ba2c699832fc333fbe81210ff52d0677fa2e915e383f6e15eb212708`; Boat `edd283e3c17eff2da5ef8b3d9ec7e4e63f8a70fa19a193493b25f5cd1af05e82`.
- Archive paths and manifest were confirmed in GitHub. Remote ZIP payload was not retrieved for local extraction; Boat ZIP payload was retrieved but not extracted and compared to checksum. Independent integrity verification remains pending.

## Next snapshots — Remote ver022 / Boat ver009
Markers have been bumped to Remote `022` and Boat `009`; paired ZIP/checksum generation and verification are pending.

## Technical validation boundary
Archives are source/documentation snapshots only. Full ESP32 firmware compile, native schematic/ERC/DRC, PCB, RF/antenna, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
