# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current generated development snapshots — Remote ver023 / Boat ver010
- Remote marker: `Remote/releases/VERSION` = `023`.
- Boat marker: `Boat/releases/VERSION` = `010`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver010.zip`.
- Manifest: `releases/SHA256SUMS_ver023_010.txt`.
- Current manifest values:
  - Boat: `b231232e3eebdb52f59d00a8fa4bc5e6c244ddc725bfa6a74a5f503ed0d709d0`
  - Remote: `5e2b0b5f644420103103789dd65f7d2d2c804cfc0f4f82d1df36b0adf4cf176d`
- The generated ZIPs and manifest are present in the branch. This session has not extracted the binary ZIPs or independently verified the hashes against downloaded bytes. Treat these as development snapshots, not integrity-verified releases.

## Package scope
Remote ver023 synchronizes status documentation with the Boat's partial session echo/queue-level ACK work; no Remote firmware functional changes.
Boat ver010 adds payload encoders and tests plus session epoch echo, per-radio sequence guards, pending-command deduplication and queue-level ACKs. The boat command executor and telemetry source are still missing.

## Validation boundary
ESP32 compile, native schematic/ERC/DRC, PCB, RF/antenna, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
