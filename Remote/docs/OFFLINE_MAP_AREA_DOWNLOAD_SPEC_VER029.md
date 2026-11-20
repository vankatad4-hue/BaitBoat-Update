# Offline map area download specification — ver029

This is the implementation contract for the approved 1 km² SD offline-map feature. It is not yet implemented in firmware.

## Area selection
- Default shape: square, 1000 m × 1000 m, centred on current GPS location (or user-selected centre).
- User may select preset sizes later (0.5, 1, 2, 5 km); v1 UI exposes 1 km only.
- Compute WGS84 bounds using local geodesic approximation: latitude offset = 500 / 111320 degrees; longitude offset = 500 / (111320 × cos(latitude)). Clamp longitude handling across ±180°, reject near-pole invalid extents.
- Preview area and bounds; clearly state imagery coverage is tile-aligned and may extend beyond the square.

## Tile coverage
For each selected zoom z, convert north/west and south/east bounds to XYZ tile indices. Include all tiles intersecting the bounding rectangle. Deduplicate tile keys across selected zooms and any existing packs.
- Use standard Web-Mercator XYZ math, clamp y to [0, 2^z−1], wrap x modulo 2^z.
- Keep selected zoom list explicit in manifest. Suggested first default: z14–17, with z18–19 selectable after storage estimate. Do not default to all 5–19 because the tile count and storage can become excessive.
- Calculate exact tile count before starting and show estimate. Raw RGB565 is 128 KiB per tile; add filesystem/metadata overhead and recommend free space ≥ estimated total × 1.10.
- Example rough scaling: each additional zoom level near the same location can quadruple the tile count; the UI must warn before a high-zoom selection.

## Download/import flow
1. Source must be explicitly licensed for offline download, storage and display. Persist provider, attribution text, licence URL, source type, pack date and bounds in manifest.
2. Online provider selection must not offer Google Maps tiles as an offline-download source.
3. For remote online imagery, implement a provider adapter only after its API terms, quotas, attribution, billing and embedded-client compatibility are reviewed. Credentials must not be stored in public source.
4. Start, pause/resume, cancel and retry failed tiles. Queue is bounded; download must not block LVGL, GPS telemetry, radio or safety task.
5. Write to temporary filenames; verify expected length (RGB565 = 131072 bytes per 256×256 tile) and checksum when supplied; rename only after validation.
6. Pack completion is atomic at manifest level: incomplete downloads remain marked incomplete and are not advertised as complete coverage.
7. Existing matching tile files are reused; duplicate data is not downloaded again. Report count reused, downloaded, failed and total disk use.
8. UI shows progress, current zoom, transfer rate where available, remaining estimate, and stop action. Resume scans the job manifest after restart.
9. When online connectivity drops, downloader pauses and reports it; map viewer may switch source independently. No mission state or boat-control action may be affected.

## Map display source policy
- AUTO: authorized online map if available, else verified SD pack.
- ONLINE: authorized online map only; if unavailable, display warning and map overlays on neutral background.
- OFFLINE: verified SD map only.
- Source indicator and attribution remain visible at all times when map imagery is rendered.
- Offline coverage lookup must be fast and use pack index; don't scan all tile files on every frame.
- Map imagery failures never disable waypoint editing, mission screen, GNSS, Return Home or boat-control communications.

## Acceptance tests
- At representative latitude, centre square is 1000 m in each direction within calculation tolerance.
- Tile count/estimated storage matches generated manifest and does not overflow integer counters.
- Dateline and tile-index limits are handled without invalid paths.
- Duplicate overlapping packs reuse tiles and do not double-count storage.
- Missing, corrupt, partial and licence-metadata-free packs are not marked verified.
- Download cancel/pause/reboot resume tests; SD full, write failure, network loss and corrupt tile tests.
- UI stays responsive and telemetry/radio task latency remains within its safety budget.
- Online provider attribution, quota/HTTP errors, API credential provisioning and no persistent online Google tile storage are verified.
