# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current development snapshots — Remote ver023 / Boat ver011
- Markers: Remote `023`, Boat `011`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver011.zip`.
- Manifest: `releases/SHA256SUMS_ver023_011.txt`.
- Boat ver011 SHA-256: `710be85bd4092125aee667b2884a122c9dc71d7dae3995a3f722c0eef12db1e1` (matches manifest). Remote ver023 manifest SHA-256: `636a1801a4f7755d6754eebc899615964898e12c8a1aca90b2cb5f9044baa199`; binary unavailable for independent verification.

## Independent integrity review
- Boat ver011 ZIP was retrieved and independently SHA-256 verified against the committed manifest. Archive structure parsed: 65 entries, sorted names, normalized DOS timestamps, no nested ZIPs; VERSION, ver011 release note and 4 host-test C++ files are present.
- Remote ver023: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver023 is status-document synchronization only.
Boat ver011 contains cross-radio replay classification and portable host tests. Boat archive verified; Remote archive remains unverified. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.
