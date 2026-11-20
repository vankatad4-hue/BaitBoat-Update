# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current generated development snapshots — Remote ver023 / Boat ver010
- Remote marker: `Remote/releases/VERSION` = `023`.
- Boat marker: `Boat/releases/VERSION` = `010`.
- ZIPs: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver010.zip`.
- Manifest: `releases/SHA256SUMS_ver023_010.txt`.
- Current manifest values:
  - Boat: `83991980e1286093126b67900f1ae02b8a614de0437483b79368454c95ca838e`
  - Remote: `37d62e5c09510891340f53f6dba0f0fb14160532624286da64e61fe013737b0f`
- Boat ver010 ZIP was fetched as binary, its ZIP directory parsed successfully (70 entries, no nested ZIPs), and its SHA-256 independently matches the manifest. Remote ver023 ZIP is present, but its binary content was not returned for local verification; its checksum remains manifest-only. Treat these as development snapshots, not operational releases.

## Package scope
Remote ver023 synchronizes status documentation with the Boat's partial session echo/queue-level ACK work; no Remote firmware functional changes.
Boat ver010 adds payload encoders and tests plus session epoch echo, per-radio sequence guards, pending-command deduplication and queue-level ACKs. The boat command executor and telemetry source are still missing.

## Validation boundary
ESP32 compile, native schematic/ERC/DRC, PCB, RF/antenna, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
