# Offline-map renderer milestone — ver028

## Implemented in source
- Geographic Web-Mercator XYZ tile placement on the Waveshare touchscreen.
- SD-backed RGB565 image tiles, bounded nine-image viewport, tile-key reuse and missing-tile caching.
- Zoom buttons (integer zoom 5–19; initial 15).
- Boat marker centered on fresh GNSS position; Home and all 16 waypoint overlays with BAIT/HOOK tags.
- Direct route line from boat to selected waypoint, or to Home while Return-Home mode is selected.
- Expanded map panel resizes its viewport.
- Waypoint/Home record writes to /missions/waypoints.bin on SD; record CRC32 and read-back verification. NVS mirror/fallback retained, and legacy four-slot NVS record migrates to the 16-slot record.
- Desktop utility converts a lawful MBTiles source into Flytec tile folders: tools/mbtiles_to_flytec.py.

## Map tile format
/maps/manifest.txt starts with FLYTEC_MAP1. Tiles:
/maps/<street|satellite>/<z>/<x>/<y>.rgb565
Each is exactly 256×256 little-endian RGB565 pixels (131072 bytes). Tile numbering is XYZ/Web-Mercator. Converter accepts MBTiles (stored row scheme TMS) and converts rows to XYZ.

## Important limitations / next tests
- This source milestone has not been compiled against the exact LVGL 9.6.0/Waveshare SDK nor tested on hardware.
- Map is north-up; compass heading is shown in the status strip but map rotation/boat-up is not yet implemented.
- No drag-pan, layer selector, map coverage index, tile download, or source attribution display on-screen yet.
- Route is a straight target line, not a route planner or boat-side mission executor.
- No map pack or map imagery is bundled. Generate a card pack from a lawful MBTiles source and copy its maps folder to the SD card.
- Renderer reads SD tiles synchronously; on-device latency, memory pressure, tile-edge behavior, zoom changes, missing-tile fallback and long-duration GPS updates must be bench tested.
- Waypoint storage is SD-primary when a card is mounted; NVS fallback is used only when SD is absent. If SD is present but writes fail, UI reports failure rather than falsely confirming persistence.

## SD mission file
/missions/waypoints.bin: fixed-size record for 16 slots, independent Home, selected slot and flags, with CRC. The writer uses a temporary file, validates read-back, then renames to the destination; NVS remains a backup/migration source. A FAT power-loss test is still required.
