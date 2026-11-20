# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current committed snapshots — Remote ver019 / Boat ver006
- Remote marker: `Remote/releases/VERSION` = `019`.
- Boat marker: `Boat/releases/VERSION` = `006`.
- ZIPs: `Remote/releases/Flytec_Remote_ver019.zip`, `Boat/releases/Flytec_Boat_ver006.zip`.
- Manifest: `releases/SHA256SUMS_ver019_006.txt`.
- SHA256 entries: Remote `658813195aef55cc8ebd26083491fa1b64de69878ce211c1d5f0d80e413c1332`; Boat `637e8460ab62ea0a675e6bfa8d02ee1dba42e43c52bc4dc54e177950334db799`.
- ZIP files and manifest are present in GitHub. The binary payloads could not be fully extracted and independently checked locally in this session; checksum verification against the bytes is pending.

## Current changes — ver020/007 pending
Remote master status and Boat package bookkeeping were refreshed after ver019/006. Markers have been bumped to Remote `020` and Boat `007`; paired ZIPs/checksum generation and verification are pending.

## Technical validation boundary
Archives are source/documentation snapshots only. Full ESP32 firmware compile, native schematic/ERC/DRC, PCB, RF/antenna, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
