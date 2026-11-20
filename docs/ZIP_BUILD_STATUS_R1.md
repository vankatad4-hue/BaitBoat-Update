# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current committed snapshots — ver016/006
- Remote marker: `Remote/releases/VERSION` = `016`.
- Boat marker: `Boat/releases/VERSION` = `006`.
- ZIP files are present in the branch: `Remote/releases/Flytec_Remote_ver016.zip`, `Boat/releases/Flytec_Boat_ver006.zip`.
- Manifest: `releases/SHA256SUMS_ver016_006.txt`.
- Manifest SHA256 entries: Remote `4390eb977c7dd6c72618d86ec7065ccb9285b46b9878649b4aabff91d2577452`; Boat `8ba0ef78680afa7ed13d904a2432f3b398d77ade87dbd7020490851fcdbfdfa9`.
- Manifest and archive presence were checked in GitHub this turn; the Remote binary content could not be retrieved for independent ZIP integrity verification. Do not label this pair fully re-verified.

## Current changes — Remote ver017 pending
Remote CAD/button scanner and documentation changed after ver016. The version marker is now `017`; ver017 ZIP/checksum generation and verification are pending. Boat files are unchanged after ver006.

## Technical validation boundary
These archives are source/documentation snapshots only. Arduino compile, native schematic/ERC/DRC, PCB, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
