# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current development snapshots — Remote ver023 / Boat ver013
- Markers: Remote `023`, Boat `013`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver013.zip`.
- Manifest: `releases/SHA256SUMS_ver023_013.txt`.
- Boat ver013 SHA-256: `7ed9a51925a3c2692ed95bb0d1cf71564dccea405f369bc8acca94feb4a5072c` (matches manifest). Remote ver023 manifest SHA-256: `636a1801a4f7755d6754eebc899615964898e12c8a1aca90b2cb5f9044baa199`; binary unavailable for independent verification.

## Independent integrity review
- Boat ver011 archive was independently verified historically. ver012 supersedes it.
- Remote ver023: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver023 is status-document synchronization only.
Boat ver013 contains host-test build cleanup and portable host test sources. Archive integrity independently verified. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.
