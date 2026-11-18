# Repository Status

## Canonical project location
This repository is the authoritative project location:
https://github.com/vankatad4-hue/BaitBoat-Update

The Boat and Remote project folders, source files, documentation, and ver004 release archives are present in the repository.

## Repository organization
- `Boat/source/` — boat-side firmware and configuration.
- `Boat/docs/` — boat-side hardware, status, radio and safety documentation.
- `Boat/releases/Flytec_Boat_ver004.zip` — archived boat release.
- `Remote/source/` — remote-side firmware, UI and supporting files.
- `Remote/docs/` — remote-side hardware, UI, map and project status documentation.
- `Remote/releases/Flytec_Remote_ver004.zip` — archived remote release.
- `docs/` — shared project documentation.

## Working rule
Before changes, inspect the relevant files in this repository. Commit approved source and documentation updates here. Keep the Boat and Remote projects separate while updating shared decisions in both when necessary. Do not overwrite released ZIPs; increment the version for each deliverable modification and preserve previous releases.

## Local synchronization
Use GitHub Desktop (or another Git client) to Fetch and Pull the latest `main` before editing locally. After local changes are committed and pushed, verify the updated repository state.
