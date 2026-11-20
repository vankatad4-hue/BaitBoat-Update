# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Last tracked package snapshot — Remote ver024 / Boat ver014
- Archive markers: Remote `024`, Boat `014` (source now Remote `025`).
- ZIPs: `Remote/releases/Flytec_Remote_ver024.zip`, `Boat/releases/Flytec_Boat_ver014.zip`.
- Manifest: `releases/SHA256SUMS_ver024_014.txt`.
- Boat ver014 SHA-256: `5988d3ac4d90ad00ff1967d5958c1b73b4e95482d26dc9d740e7e39a7aeb964b` (matches manifest). Remote ver024 manifest SHA-256: `34307b961a7e966dccea172a2b48d7a957436aef856d1b76f2bfa72d635e4efa`; binary unavailable for independent verification.

## Independent integrity review
- Older Boat ver011–013 archives are historical snapshots; ver014 supersedes them.
- Remote ver024: archive and manifest are present, but the ZIP binary could not be retrieved through the connector. Its checksum is not independently verified.
- Deterministic package workflow was updated and a local command smoke-test passed. GitHub workflow logs are not accessible in this session; archive/manifest outputs are present.

## Package scope
Remote ver024 adds touchscreen mode request buttons, joystick diagnostics and safety/ACK feedback UI.
Boat ver014 adds the portable fail-closed command policy and policy tests. Archive integrity independently verified. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.


## Remote ver024 UI/input update
- Remote version marker advanced to ver024 for touchscreen mode requests, live joystick status, compact safety/ACK footer, and stale/unknown battery display.
- ZIP/checksum generation is handled by the repository workflow; output and checksum still require workflow verification. No firmware compile or hardware test.


## Remote ver025 mission UI change
- Remote version marker is now ver025. Source includes mission/waypoint planning UI and SVG preview.
- New release notes: `Remote/releases/RELEASE_VER025.md`.
- The ver025 ZIP and checksum are not verified/available yet. Keep ver024 archive as the previous snapshot; do not claim a ver025 package is ready until it is rebuilt and verified.


## Remote ver026 source milestone
- Source marker advanced to ver026; release notes `Remote/releases/RELEASE_VER026.md`.
- Adds NVS waypoint/home persistence and local relative map overlay.
- ver026 archive and checksum are not verified; ver024 remains the last confirmed package snapshot (Remote ZIP itself still not independently verified).
