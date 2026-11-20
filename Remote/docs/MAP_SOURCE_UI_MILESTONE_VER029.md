# Map-source UI and area-planner milestone — ver029 development

**Current package:** ver028 (unchanged)  
**Development source:** ver029 in progress  
**Hardware verification:** not performed

## Implemented
- `map_source.h/.cpp`: AUTO / ONLINE / OFFLINE preference enum, NVS persistence and cycling helper.
- Map panel includes source selector button. Selection persists after reboot.
- ONLINE state is deliberately labelled unavailable; the source scaffold returns false until an authorized Google provider is integrated. The current renderer is SD-only, so it falls back to SD/no-map state rather than pretending online tiles exist.
- `tools/map_area_planner.py`: computes a square geographic area, XYZ coverage for specified zooms, unique tile list, per-zoom counts and RGB565 storage estimate. JSON output is a plan only; it fetches no imagery.
- Unit tests added for area bounds, deduplicated tile counts, dateline wrapping and storage math.

## Example storage planning
For a 1 km square centred near latitude 43° using zoom levels 14–17, the planner logic estimates approximately 50 tile files, about 6.25 MiB raw RGB565, or roughly 6.88 MiB recommended free storage with a 10% margin. Actual tile count depends on the centre location and tile-boundary alignment.

- `tools/mbtiles_area_export.py`: extracts only the planned area/zoom XYZ tile subset from a local MBTiles archive and writes RGB565 pack files, attribution/licence metadata, and a missing-tile completeness report. No network downloads occur.

## Not implemented
- Google online renderer/API requests, online connection validation or API-key provisioning.
- Actual map-data download.
- Provider catalogue/licence validation.
- Area selection GUI, storage preview in the remote UI, progress/pause/resume.
- Tile coverage index and exact per-tile coverage validation.
- Compile against Waveshare board libraries; physical display/SD test.

## Verification
The GitHub files and commits were confirmed through repository actions. The test source is present, but the test suite could not be executed in the working environment because network access to raw GitHub was unavailable and the files were not present in the local runtime. Do not mark the tests as passing. Review tile calculations manually; run `python3 -m unittest discover -s Remote/tools/tests -v` once the repository is cloned locally.

## Next
1. Add a per-tile coverage index and report actual covered zoom/tile bounds, beyond the pack-complete flag.
2. Add the offline-map manager UI (area centre, 1 km default, zoom choices, storage estimate, SD import/download progress).
3. Implement a licensed-source download/import flow; avoid public tile servers that prohibit bulk use.
4. Implement Google Map Tiles API 2D online renderer after decoder, credentials, TLS, attribution and billing/quota controls are designed.
