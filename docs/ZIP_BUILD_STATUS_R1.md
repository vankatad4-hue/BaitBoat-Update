# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current development snapshots — Remote ver023 / Boat ver011
- Markers: Remote `023`, Boat `011`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver011.zip`.
- Manifest: `releases/SHA256SUMS_ver023_011.txt`.
- The ver010/023 manifest values are historical and do not apply to Boat ver011. New manifest values will be recorded after the package workflow completes.

## Independent integrity review
- Boat ver010 ZIP was verified historically. Boat ver011 archive has not been generated/verified yet.
- Remote ver023: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver023 is status-document synchronization only.
Boat ver011 source adds cross-radio replay classification and portable host tests; archives pending rebuild. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.
