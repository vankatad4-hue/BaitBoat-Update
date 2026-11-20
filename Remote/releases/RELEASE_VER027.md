# Remote ver027 — 16 waypoint slots and SD map storage

Date: 2026-11-20

## Changes
- Waypoint capacity expanded from 4 to 16 persistent NVS slots, plus separate Home.
- Manager screen presents slots as a 4×4 grid.
- Save-current requires fresh GNSS (≤2s), persists coordinates/selected slot with CRC, and reports write failure.
- BAIT/HOOK planning flags persist independently.
- CRC-valid ver026 four-slot record migration added; retains prior points/Home and writes the new schema.
- Main map marker array expanded to 16, on the existing local placeholder grid and 150m marker window.
- Integrated TF/microSD is designated offline-map data storage. Existing map pack reader supports manifest and RGB565 XYZ tiles; storage format documented.

## Map pack paths
See Remote/docs/OFFLINE_MAP_SD_STORAGE_VER027.md.
- /maps/manifest.txt, signature FLYTEC_MAP1
- /maps/street/<z>/<x>/<y>.rgb565
- /maps/satellite/<z>/<x>/<y>.rgb565
- Raw 256×256 RGB565 tiles (131072 bytes)

## Not complete
- Geographic tile renderer/cache, zoom/pan, boat-up rotation and route display.
- Offline map pack generator/downloader and manifest metadata parser.
- Target compile, display/SD/NVS tests and hardware validation.
- Mission execution/autopilot/releases/return-home remain disabled.
