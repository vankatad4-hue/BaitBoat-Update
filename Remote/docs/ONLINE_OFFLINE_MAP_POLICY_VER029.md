# Online Google Maps + licensed offline SD maps — design decision

**Status:** Accepted architecture; implementation pending.  
**Decision date:** 2026-11-20  
**Baseline:** Remote ver028 / Boat ver014.

## User-approved behavior
- When the remote has Internet access, offer Google Maps online viewing through an authorized Google Maps Platform service.
- When Internet is unavailable, automatically show the locally stored offline map on SD.
- Autopilot, GPS, waypoint storage, Home/Return Home and boat-control safety must never depend on Internet or map-provider availability.
- Never download, bulk-prefetch, or persist Google map tiles to SD. Google online map imagery is transiently displayed only, subject to the applicable API terms, quotas, attribution and billing.
- Offline SD imagery/data must come from a provider/source whose licence explicitly permits downloading, conversion, storage and offline display. Preserve attribution/licence metadata in the map pack and show attribution on-screen.

## Proposed UI
Map source indicator: `ONLINE / OFFLINE` and selected layer.
- Auto: use authorized online Google map when connectivity and API service are available; otherwise fall back to SD offline map.
- Online: request online map; show a clear unavailable state if offline.
- Offline: use SD map only.
- Offline map manager: select current GPS position or centre point; choose area (default 1 km × 1 km square), layer/source and zoom range; preview estimated tile count/storage; download on Wi-Fi; cancel/resume; verify files and manifest; display progress and coverage.
- A downloaded area can be extended or overlapped; deduplicate tiles and report storage use. Do not claim imagery is available for a zoom/layer until coverage is verified.
- Map panel must continue to display own boat, Home, 16 waypoints, BAIT/HOOK labels and selected target overlays irrespective of map source.

## Online implementation guardrails
The ESP32-S3 may use Wi-Fi for authorized online map requests, but provider-specific API access must be implemented only after selecting a service/API compatible with the embedded client, its usage terms, quotas, attribution requirements and billing. Google Maps Platform is not the Android Maps SDK on this device. Do not use undocumented endpoints, scrape map websites, proxy prohibited tile downloads, or store Google tiles for offline use. No API key or credentials should be committed to the repository; use a provisioning mechanism and restrict the key to the intended API.

## Offline map pack
Continue the existing Flytec XYZ RGB565 renderer format:
- `/maps/manifest.txt` with provider, licence, attribution, layer, coverage bounds, zooms, tile format and pack date.
- `/maps/<layer>/<z>/<x>/<y>.rgb565`, 256×256 little-endian RGB565.
- MBTiles converter remains an import path only when the source's licence and terms allow offline use and conversion. Keep source attribution and licence in pack metadata.
- Tile download/import tool must validate manifest, coverage, file size, checksum (where available), and storage estimate. Do not bundle third-party map data in firmware releases without permission.

## Fail-safe behavior
- Maps are presentation only. No Internet dependency for mission controls.
- Online failure or expired quota: switch to offline map if coverage exists; otherwise display blank/grid background plus overlays and a status warning.
- SD missing/corrupt: retain online view if Internet is available; mission records retain their existing NVS fallback. Never report a map pack as ready without a coverage check.
- Do not block the UI or control loop during network/SD work; use bounded queues and progress/error states in implementation.

## Implementation sequence
1. UI source selector/status and attribution/licence display requirements.
2. Offline pack metadata/coverage index and 1×1 km area geometry (geographic bounds; tile coverage expands to tile boundaries).
3. Downloader/import workflow with provider/source selection, storage estimate, cancellation, integrity validation and resumable jobs.
4. Authorized online provider adapter and connectivity detection; API key provisioning, attribution, rate limiting and no persistent Google tile cache.
5. Bench tests: network loss, SD removal, invalid packs, overlapping downloads, zoom coverage, UI responsiveness, attribution visibility and map-only failure while mission/safety features continue.

## Not yet implemented or verified
This document records the accepted architecture only. Online Google map display, offline area downloader, UI selector, coverage index, provider integration and hardware testing are not implemented in ver028.
