# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current generated snapshots — Remote ver022 / Boat ver009
- Remote marker: `Remote/releases/VERSION` = `022`.
- Boat marker: `Boat/releases/VERSION` = `009`.
- ZIP files: `Remote/releases/Flytec_Remote_ver022.zip`, `Boat/releases/Flytec_Boat_ver009.zip`.
- Manifest: `releases/SHA256SUMS_ver022_009.txt`.
- Current manifest entries (from the branch):
  - Boat: `b3f93d6da408fcb09f498140b499033f762c6e7e792ea613204588a63642da52`
  - Remote: `d2972b23287fa6c10b73c39090a516a5e5975c664dc0b7bb0f418a629842d6b0`
- The archive paths and manifest exist in GitHub. This session has not extracted the ZIPs or independently verified their SHA-256 against the actual archive bytes. Treat these as generated development snapshots, not verified releases.

## Package scope
Remote ver022 is documentation/status synchronization only; no firmware or hardware readiness change.
Boat ver009 adds portable payload encoders and host test source; encoder is not connected to boat application, radio ACK or telemetry flow.

## Validation boundary
Full ESP32 firmware compile, native schematic/ERC/DRC, PCB, RF/antenna, hardware bench and operational safety validation remain incomplete. No motor/solenoid operation.
