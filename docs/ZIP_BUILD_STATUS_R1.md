# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current development snapshots — Remote ver023 / Boat ver014
- Markers: Remote `024`, Boat `014`.
- ZIPs: `Remote/releases/Flytec_Remote_ver024.zip`, `Boat/releases/Flytec_Boat_ver014.zip`.
- Manifest: `releases/SHA256SUMS_ver024_014.txt`.
- Boat ver014 SHA-256: `5988d3ac4d90ad00ff1967d5958c1b73b4e95482d26dc9d740e7e39a7aeb964b` (matches manifest). Remote ver023 manifest SHA-256: `8f04f0ef2e21b6e10cdec913e585b827c0858b9e6829c78a1c65c4813b691ef2`; binary unavailable for independent verification.

## Independent integrity review
- Older Boat ver011–013 archives are historical snapshots; ver014 supersedes them.
- Remote ver023: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver024 adds touchscreen mode request buttons, joystick diagnostics and safety/ACK feedback UI.
Boat ver014 adds the portable fail-closed command policy and policy tests. Archive integrity independently verified. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.


## Remote ver024 UI/input update
- Remote version marker advanced to ver024 for touchscreen mode requests, live joystick status, compact safety/ACK footer, and stale/unknown battery display.
- ZIP/checksum generation is handled by the repository workflow; output and checksum still require workflow verification. No firmware compile or hardware test.
