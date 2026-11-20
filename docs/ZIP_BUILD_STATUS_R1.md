# ZIP Build Status — 2026-11-20

## Historical ver005 artifacts — verified
PR workflow run #32 produced Remote/Boat ver005 archives and checksums. ZIP integrity and SHA-256 were previously verified. Older archives are preserved.

## Current version markers — Remote ver023 / Boat ver010
- Markers: Remote `023`, Boat `010`.
- ZIP paths are present: `Remote/releases/Flytec_Remote_ver023.zip`, `Boat/releases/Flytec_Boat_ver010.zip`.
- Manifest: `releases/SHA256SUMS_ver023_010.txt`.
- Manifest currently claims:
  - Boat: `de2332e6daa150ad237a4337cacdfd742afe6292eeaf61eb0b4e1e7fa25db89c`
  - Remote: `789cc1e270e5fb6bb7f652bde09943e7cf941352d2f75f24e29ef3a93677955c`

## Integrity findings
- Boat ZIP latest retrieved size: 74,747 bytes. Its SHA-256 was calculated independently as `ef657219758d1ee00c6b9198c94cc7d6ef04dfe472f760818371a864fa7afa1f`, which **does not match the committed manifest**. Archive central-directory/local-header structure was parsed; it contains 70 entries, no nested ZIPs, and includes encoder, test, session/ACK documentation, release note and current version marker. It is not currently integrity-verified.
- Remote ZIP is present, but the connector did not return its binary content; its checksum remains unverified.
- A reproducible-build workflow change was committed: sorted file lists, normalized timestamps, ZIP integrity test and copy/checksum verification. A fresh workflow package run is still needed to regenerate both archives and manifest consistently.

## Package scope
Remote ver023 is status-document synchronization only.
Boat ver010 includes payload encoders/host tests and session echo, per-radio sequence guards, pending-command deduplication and queue-level ACK scaffold. No Boat application/executor or telemetry source.

## Validation boundary
No full ESP32 build, native schematic/ERC/DRC, PCB, RF/antenna, power/thermal or hardware validation. Do not operate motors or solenoids.
