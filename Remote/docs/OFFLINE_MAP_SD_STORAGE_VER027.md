# Remote offline-map SD storage specification — ver027

**Decision:** The integrated TF/microSD slot on the Waveshare remote is the storage medium for offline map data. Do not store map tiles in ESP32 NVS or firmware flash. NVS remains for compact waypoint/home/settings records only.

## Card layout
Use a FAT-formatted microSD card. The existing offline-map loader expects:
```
/maps/manifest.txt
/maps/street/<zoom>/<x>/<y>.rgb565
/maps/satellite/<zoom>/<x>/<y>.rgb565
```
Manifest first 11 bytes must be exactly `FLYTEC_MAP1`. The tile format is raw 256×256 RGB565, 131,072 bytes per tile, with no header/compression. Web-Mercator XYZ tile indices. Tile reader rejects malformed sizes and out-of-range indices. Map pack should include coverage bounds, available zoom range, source attribution/licence and generation date in manifest metadata after the renderer/manifest parser is extended.

## Current code support (updated ver028)
- The board display adapter mounts the integrated TF slot; app passes its mount state to `offlineMap_begin()`.
- `offline_map.cpp` validates the manifest and reads individual street/satellite RGB565 tiles from SD into PSRAM.
- SD unavailable, missing manifest, missing tiles or malformed tiles must display the neutral fallback and never fabricate map imagery.
- ver028 adds a geographic tile renderer with a nine-image viewport cache, integer zoom buttons (5–19), boat marker, all 16 WP overlays, Home and a direct route line to selected WP/Return-Home target. Missing tiles remain neutral. The renderer is north-up; pan and layer UI remain pending.
- Mission data is now SD-primary in `/missions/waypoints.bin`, CRC-protected and read-back verified. NVS mirror/fallback and legacy migration are retained. See `MISSION_SD_STORAGE_TEST_PLAN_VER028.md`.
- Do not claim maps are installed until a card with a compatible map pack is tested.

## Map-data management plan
1. Use a PC-based offline pack generator/downloader (not live downloading from the controller) to create coverage around chosen fishing lakes/river reaches.
2. Export bounded zoom levels to the directory format above; avoid excessive zoom/storage.
3. Preserve provider attribution and comply with the source's licence/terms. Do not bulk-download from providers that prohibit offline caching.
4. Verify pack manifest, tile count, tile sizes, and card free space on PC.
5. Insert card and verify the SD status on remote; later firmware work will provide coverage/zoom selection and tile-cache diagnostics.

## Waypoint capacity
Remote ver027 expands the NVS mission record to 16 waypoint slots, plus one separate Home. Each slot stores coordinates and independent BAIT/HOOK planning flags. CRC and coordinate validation remain. This is compact data and stays in NVS; map imagery goes to SD.
