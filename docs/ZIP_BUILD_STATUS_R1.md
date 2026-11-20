# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current development snapshots — Remote ver023 / Boat ver012
- Markers: Remote `023`, Boat `012`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver012.zip`.
- Manifest: `releases/SHA256SUMS_ver023_012.txt`.
- ver011/023 manifest values are historical. ver012 archive checksum will be recorded after workflow completes. Remote ver023 binary remains unavailable for independent verification.

## Independent integrity review
- Boat ver011 ZIP was retrieved and independently SHA-256 verified against the committed manifest. Archive structure parsed: 65 entries, sorted names, normalized DOS timestamps, no nested ZIPs; VERSION, ver011 release note and 4 host-test C++ files are present.
- Remote ver023: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver023 is status-document synchronization only.
Boat ver012 contains cross-radio replay classification, portable command queue and host tests, and ACK sender. ZIP build pending. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.
