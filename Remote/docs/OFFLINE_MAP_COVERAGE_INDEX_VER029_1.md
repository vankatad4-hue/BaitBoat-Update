# Offline map coverage index + manager/import UI (ver029.1/029.2)

Exporter emits sorted `/maps/coverage_index.csv` with header `z,x,y,present` and a row for every planned tile. Missing tiles are explicitly marked `present=0`. Firmware loads the index into PSRAM (6000-row cap), validates XYZ ranges and sorted unique keys, and uses binary search to gate tile reads.

Map Manager is opened from the MAP panel's MANAGER button. It supports GPS centre or ±0.001° nudges, a fixed 1km square, zoom 14–19 (14–17 selected initially), tile/storage estimate, active pack planned/present/missing totals, and selected-area coverage percentage.

## Staged import
Copy the exporter output's `maps/` directory contents into the SD card `/map-import/` folder. Expected: `manifest.txt`, `coverage_index.csv`, and selected style folders. The touchscreen's IMPORT STAGED PACK button imports only complete packs with attribution/licence metadata. Tile files are copied in 4KiB chunks and must be exactly 131072 bytes. Coverage index and manifest are copied last; active manifest is removed before import so interrupted installation is not considered valid. Progress displays tiles and bytes. Revalidation and map-cache invalidation occur at completion.

Import requires temporary storage for staged and active pack copies. It is not a network downloader and does not use Google imagery. Source code only, not compiled or tested on target hardware.
