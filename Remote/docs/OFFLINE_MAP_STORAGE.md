# Offline map storage — first implementation stage

## Hardware interface

The Waveshare ESP32-S3-Touch-LCD-4.3 standard board routes TF-card SPI to GPIO11 (MOSI), GPIO12 (SCK), GPIO13 (MISO), and routes physical SD chip-select to CH422G EXIO4 (active low). The firmware reuses the CH422G instance created by `Board::init()` / `Board::begin()`; it does not create a second expander or a second I2C driver.

The Arduino ESP32 SD library still expects a GPIO chip-select argument even though the real card CS is on the expander. The current adapter therefore uses GPIO6 as an unused dummy CS argument, disables hardware CS, and keeps the physical card selected through EXIO4. GPIO6 is not assigned to the board's listed LCD, touch, TF SPI, RS485/CAN or USB signals. This arrangement follows the Waveshare SD example pattern and the documented ESP32 SD-library workaround. Confirm on the actual board before treating the mount as hardware-verified.

The mount is optional: display and touch continue if no card is inserted or mounting fails. Serial output reports the storage result.

## Map pack format currently supported

The map loader is deliberately simple and deterministic in this stage. It expects:

```text
/maps/manifest.txt
/maps/street/<zoom>/<tile-x>/<tile-y>.rgb565
/maps/satellite/<zoom>/<tile-x>/<tile-y>.rgb565
```

`manifest.txt` must begin with the exact 11-byte signature `FLYTEC_MAP1`. Each tile is an uncompressed 256 × 256 RGB565 image, exactly 131,072 bytes, with no header. Tiles use the standard Web-Mercator XYZ coordinate scheme. Missing or malformed tiles are rejected; the renderer must show a neutral missing-tile background rather than inventing land or water.

`offline_map.cpp` can validate the map pack and load an individual tile into PSRAM. The caller owns each returned buffer and must call `offlineMap_freeTile()` after LVGL no longer references it. The map renderer, tile cache, geospatial placement/rotation, waypoint overlay and map-pack creation tool are not implemented yet.

## Current UI status

The map panel reports GPS fix and SD/map-pack status separately. The visible grid remains a diagnostic placeholder, not a geographic map. No map imagery is bundled in this firmware ZIP.

## Test sequence

1. Build and run the firmware with the display board and a formatted microSD card inserted.
2. Check Serial Monitor for `SD: TF card ready`.
3. If no map pack exists, the UI should show `MAP: NO TILES ON SD`.
4. Later, copy a generated map pack with the directory structure above and verify `MAP: TILES READY`.
5. Test missing, truncated and valid tile reads before enabling continuous map rendering.

Do not use the public OpenStreetMap tile server as a bulk download source. Use a lawful offline data source and retain required attribution/licensing in the map pack.
