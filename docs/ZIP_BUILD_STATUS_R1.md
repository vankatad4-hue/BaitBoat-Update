# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current development snapshots — Remote ver023 / Boat ver010
- Markers: Remote `023`, Boat `010`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver010.zip`.
- Manifest: `releases/SHA256SUMS_ver023_010.txt`.
- Current manifest values:
  - Boat: `9c3736c411bab99436665908375460fb283eac2625c4511e864c335f53e721d2`
  - Remote: `f890e77c64704e7f8902b53ae4253433d53bca9ebfa2f7dbe2f11e8fb1514716`

## Independent integrity review
- Boat ver010 ZIP: retrieved and independently SHA-256 verified against the manifest. ZIP central directory and local headers parse; 60 entries; sorted names; normalized DOS timestamps (1980-01-01); no nested ZIPs; encoder, host test, session/ACK doc, source, release note and VERSION are present.
- Remote ver023: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver023 is status-document synchronization only.
Boat ver010 includes payload encoders/host tests and session echo, per-radio sequence guards, pending-command deduplication and queue-level ACK scaffold. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.
