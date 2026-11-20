# Repository Status

## Canonical project location
This repository is the authoritative project location:
https://github.com/vankatad4-hue/BaitBoat-Update

The Boat and Remote project folders, source files, documentation, and preserved ver004 release archives are present in the repository. Current source updates are on development branch `hardware-ready-dual-radio-r1`.

## Repository organization
- `Boat/source/` — boat-side firmware and configuration.
- `Boat/docs/` — boat-side hardware, status, radio and safety documentation.
- `Boat/releases/Flytec_Boat_ver004.zip` — archived boat release.
- `Remote/source/` — remote-side firmware, UI and supporting files.
- `Remote/docs/` — remote-side hardware, UI, map and project status documentation.
- `Remote/releases/Flytec_Remote_ver004.zip` — archived remote release.
- `docs/` — shared project documentation.

- `Remote/releases/VERSION` and `Boat/releases/VERSION` are set to `005` for the next automated ZIP snapshots.
- [ZIP packaging policy](ZIP_RELEASE_POLICY_R1.md) and `.github/workflows/package-flytec-zips.yml` define packaging and SHA256 artifacts.
- The Heltec controller pin audit is pending physical revision identification; boat GPIOs remain unset.

## Working rule
Before changes, inspect the relevant files in this repository. Commit approved source and documentation updates here. Keep the Boat and Remote projects separate while updating shared decisions in both when necessary. Do not overwrite released ZIPs; increment the package version file for each deliverable modification and preserve previous releases. Verify the automated ZIP artifacts and checksum before declaring the new archive issued.

## Local synchronization
Use GitHub Desktop (or another Git client) to Fetch and Pull the latest `main` before editing locally. After local changes are committed and pushed, verify the updated repository state.
