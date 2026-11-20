# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current committed snapshots — Remote ver017 / Boat ver006
- Remote marker: `Remote/releases/VERSION` = `017`.
- Boat marker: `Boat/releases/VERSION` = `006`.
- Both archives are present in the branch: `Remote/releases/Flytec_Remote_ver017.zip`, `Boat/releases/Flytec_Boat_ver006.zip`.
- Manifest: `releases/SHA256SUMS_ver017_006.txt`.
- Manifest SHA256 entries: Remote `14e19b8a9109f23e3e45aa4b38d2c744eb893cff2e81b657a0aae9de671a3e7d`; Boat `fb28589d9f74bf578f128637f6122930fa905926fc411db0658c791b990c040a`.
- Archive files and checksum manifest were confirmed in GitHub. The binary ZIP payloads could not both be retrieved for local extraction/integrity verification in this session; treat them as generated snapshots with recorded checksums, not independently re-verified.

## Remote ver018 pending\nRemote marker has been bumped to 018 for the front-layout preview and status/documentation changes. The ver018 archive and manifest have not yet been confirmed generated.\n\n## Technical validation boundary
These archives are source/documentation snapshots only. Arduino compile, native schematic/ERC/DRC, PCB, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
