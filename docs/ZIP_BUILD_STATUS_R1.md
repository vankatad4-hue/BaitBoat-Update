# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current committed snapshots — Remote ver018 / Boat ver006
- Remote marker: `Remote/releases/VERSION` = `018`.
- Boat marker: `Boat/releases/VERSION` = `006`.
- Both archives exist in the branch: `Remote/releases/Flytec_Remote_ver018.zip`, `Boat/releases/Flytec_Boat_ver006.zip`.
- Manifest: `releases/SHA256SUMS_ver018_006.txt`.
- Manifest SHA256 entries: Remote `610b1c973f6eb5e86eafabfed0190ba0e0638b1bc2cac666a36a44a22ea1ece3`; Boat `52f7a289773d1a5a058b3a8edc5d7be7d9cc9b4c2783355bf343d5625d1ea2fd`.
- Archive files and manifest were checked in GitHub; both ZIP payloads could not be fully extracted/verified locally in this session. Treat as generated snapshots with recorded checksums, not independently re-verified.

## Remote ver019 pending
Remote receive/ACK/telemetry integration and review documentation changed after ver018. Remote marker is now `019`; ver019 archive/checksum generation and verification are pending. Boat remains ver006.

## Technical validation boundary
These archives are source/documentation snapshots only. Full ESP32 firmware build, native schematic/ERC/DRC, PCB, RF/antenna, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
