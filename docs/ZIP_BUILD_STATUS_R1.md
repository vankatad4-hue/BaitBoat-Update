# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current development snapshots — Remote ver023 / Boat ver014
- Markers: Remote `023`, Boat `014`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver014.zip`.
- Manifest: `releases/SHA256SUMS_ver023_014.txt`.
- Boat ver014 SHA-256: `517fc1094b5d3a63c6bf94f75eaca20a5964846037c590f8ef03d4bf7dcde2c0` (matches manifest). Remote ver023 manifest SHA-256: `636a1801a4f7755d6754eebc899615964898e12c8a1aca90b2cb5f9044baa199`; binary unavailable for independent verification.

## Independent integrity review
- Older Boat ver011–013 archives are historical snapshots; ver014 supersedes them.
- Remote ver023: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver023 is status-document synchronization only.
Boat ver014 adds the portable fail-closed command policy and policy tests. Archive integrity independently verified. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.
