# Repository Import Status

## Connection

GitHub App installation and repository push access were verified on 2026-11-17 for `vankatad4-hue/BaitBoat-Update`, default branch `main`.

## Source archive inventory

The project Library contains:
- `Flytec_Boat_ver004.zip` (18,080 bytes)
- `Flytec_Remote_ver004.zip` (79,125 bytes)

Both archives were retrieved and their contents inspected. Boat ver004 contains 9 files. Remote ver004 contains 30 entries (including directory entries).

## Import limitation

The available GitHub connector supports UTF-8 text file operations and Git object operations, but does not provide a direct binary archive upload or a filesystem bridge from the retrieved ZIPs. Therefore the original source files and ZIP release archives have **not** yet been committed by this import step. This document does not claim otherwise.

The repository structure and shared project documentation are being established first. To complete a byte-for-byte import of all existing firmware files and the two ZIP archives, use GitHub's web upload interface (or a local Git client) to upload the extracted folders and archives. Preserve the existing ver004 filenames and contents exactly.

## Planned paths

- `Boat/Flytec_Boat/` — contents of the boat ver004 archive.
- `Remote/Flytec_Remote/` — contents of the remote ver004 archive.
- `releases/Flytec_Boat_ver004.zip`
- `releases/Flytec_Remote_ver004.zip`

After import, compare file counts and archive checksums before treating the migration as complete.
