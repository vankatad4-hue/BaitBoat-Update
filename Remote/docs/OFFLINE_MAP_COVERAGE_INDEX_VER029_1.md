# Offline coverage index + map manager (ver029.1)

Exporter emits `/maps/coverage_index.csv` with header `z,x,y,present` and one row per requested XYZ tile. Rows with present=0 record missing data. Firmware requires this index in addition to a valid manifest, loads at most 6000 rows, and rejects malformed/out-of-range entries. Tile reads are gated by a matching present row.

The Map Manager opens from the map panel and provides:
- GPS current location as centre, or ±0.001° coordinate nudges;
- fixed 1km square;
- zoom toggles 14–19, initially 14–17;
- approximate tile/storage estimate, RGB565 raw and 10% margin;
- pack index planned/present/missing summary;
- selected-area coverage check with progress bar.

The bar is coverage verification progress, **not download/import transfer progress**. A staged pack installer, downloader and resumable transfer are not implemented yet. This milestone is source-only and not compiled/hardware tested.
