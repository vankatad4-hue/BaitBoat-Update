# Flytec Remote — Consolidated Version / Change History
**Document revision:** VER003  
**Remote source/release version:** 028 (kept unchanged)  
**Boat source/release version:** 014 (kept unchanged)  
**Branch:** `hardware-ready-dual-radio-r1`  
**Last reconciled:** 2026-11-20

## Purpose and maintenance rule
This is the single consolidated visible change-history file for the Remote project. Individual release notes and milestone documents remain as supporting records, but the timeline and current status are maintained here.

- Do not change the Remote source/release marker just to revise this history file.
- Each time this consolidated file itself is updated, rename it to the next document revision (VER002, VER003, …), update the document revision at the top, and update links/references to its new filename.
- For an actual software/source release, keep the source version marker unchanged until that release is packaged and verified. Development work after ver028 is recorded as **unreleased development**, not as a released version.
- Historical detail before ver017 is incomplete because corresponding release-note files are not present in this branch. Preserve the archives; do not invent missing changes.

## Current project state
- Remote source marker: **ver028**. ver029 is development work and is not packaged.
- Boat source marker: **ver014**.
- Remote ver028 ZIP exists in the repository, but its contents/build/checksum are not independently verified in this reconciliation.
- No full target firmware build, native electrical ERC/DRC, or operational hardware validation has been confirmed.
- The boat mission executor, autonomous navigation, sonar integration, and actuator execution remain unverified/incomplete and must remain fail-closed.
- Branch PR #1 is draft/open and unmerged, per current status record.

## Historical version timeline

### ver001–ver016 — early development/archive period (history incomplete)
- Historical Remote ZIP archives are retained in the repository for versions that exist (ver004–ver009 and ver011–ver016).
- Individual release-note records for this range are not present in the current release-note set. The changes cannot be reliably itemized from the filenames alone.
- Later project documents record the evolution toward the R5/R6 handheld design, dual radio links, ESP32-S3 touchscreen/joystick architecture, safety and telemetry scaffolding, CAD and electrical design. These are not attributed to a specific early version without evidence.

### ver017 — button/OK mapping and CAD source corrections
- Corrected R6 front-panel OK key to right-bank bottom-right.
- 15 scanned keys on MCP23017 0x20 plus separate OK input on MCP23017 0x21 GPA4.
- Normalized escaped newline formatting in OpenSCAD R6 source.
- Source/CAD/documentation only; exports, mesh fit, firmware and hardware checks pending.

### ver018 — layout preview and package-status reconciliation
- Added R6.6 vector front-layout preview.
- Corrected OK placement and expander crosswalk; scanner remains 15 scanned keys plus separate OK.
- Updated package-status documents.
- CAD validation relied on a reconstructed local source mirror; repository render and physical fit not verified.

### ver019 — Remote authenticated receive, ACK, telemetry
- Added authenticated receive dispatcher with session/envelope validation.
- Per-radio and cross-radio sequence guards; duplicate session markers no longer reset ACK/telemetry state.
- Remote ACK tracker wired to sends and incoming ACKs.
- Partial telemetry group updates and freshness/safety provenance.
- Corrected nonzero 16-bit action IDs and heartbeat fields.
- Host tests reported passing for decoder, ACK tracker, telemetry freshness/safety, release guard and scanner syntax.
- Boat telemetry/ACK generation, execution/deduplication, full target build and hardware validation remained incomplete.

### ver020 — status synchronization
- Master status synchronized to R6.6 and Remote receive/telemetry/ACK integration.
- No readiness change; build/RF/hardware/actuator validation still pending.

### ver021 — package-pair status synchronization
- Master status synchronized with generated ver020 Remote / ver007 Boat package pair.
- No functional firmware, electrical or mechanical readiness change.

### ver022 — release-note gap
- No dedicated release note was found in the current branch. Do not infer a change from the missing note.

### ver023 — Boat-side protocol scaffolding reflected in Remote status
- Updated Remote status for Boat partial session echo, per-link sequence guards, pending-command deduplication and queue ACK scaffold.
- ACCEPTED means queue acceptance only, not command execution.
- No Remote functional change recorded; no compile or operational test.

### ver024 — touchscreen operating controls and input/safety feedback
- Added MANUAL, AUTOPILOT and RETURN HOME request controls to operating footer.
- Added joystick diagnostics: calibration/read health, raw/normalized values, neutral/active state.
- Added STOP confirmation, failsafe/low-battery/interlock readiness and pending ACK count strip.
- Stale/missing battery telemetry shown as unknown rather than 0%.
- UI requests/status only; boat executor is not implemented. Authentication key provisioning disabled. Not target-compiled or hardware tested.

### ver025 — mission/waypoint planning UI scaffold
- Added dedicated mission planner.
- Four RAM-only waypoint slots: select, save fresh GNSS fix, clear.
- HOME can be set from fresh GNSS.
- Selected waypoint can be tagged for planned BAIT/HOOK actions; tags are planning only.
- Mission start deliberately blocked.
- Fresh-fix requirement: GNSS valid and no older than 2 seconds.
- Added screen preview and fixed missing ver024 UI label declarations.
- No persistent storage, mission executor, geofence validation, secure command implementation or release sequencing.

### ver026 — NVS persistence and relative map markers
- Waypoint, Home and BAIT/HOOK planning flags persisted in ESP32 NVS Preferences.
- Record magic/version and CRC32 integrity validation; malformed data rejected.
- Fresh GNSS gate retained for save/Home.
- Main placeholder map displays relative waypoint/Home markers within a ±150m window.
- Map is only a local grid placeholder, not a geographic navigation map.
- Mission execution remains disabled; target compile/hardware tests and archive/checksum not confirmed.

### ver027 — 16 waypoint capacity and SD map-storage support
- Increased waypoint capacity from 4 to **16**, plus separate Home.
- 4×4 slot grid; save-current fix requires ≤2s age.
- Added migration from CRC-valid four-slot ver026 record.
- Selection and BAIT/HOOK planning flags persist.
- Marker overlay supports 16 entries.
- TF/microSD designated for offline maps. Map pack structure and existing manifest/RGB565 tile reader documented.
- No full renderer, map downloader/generator, compile, SD/NVS test or hardware validation.
- Mission execution/autopilot/releases/return-home remain disabled.

### ver028 — offline geographic map renderer and SD mission persistence
- Added Web-Mercator geographic renderer for SD XYZ RGB565 tiles.
- Bounded 3×3 tile slots, tile reuse and missing-tile fallback.
- Zoom 5–19 (default 15).
- Shows boat, 16 waypoints with BAIT/HOOK tags, Home, and straight route to selected waypoint or Home in Return-Home mode.
- Mission/Home data stored on SD at `/missions/waypoints.bin`, CRC checked/read-back validated and temp-file rename; NVS backup/fallback retained.
- Added MBTiles-to-Flytec pack converter with TMS-to-XYZ row conversion.
- Map data is not bundled; renderer remains north-up with no pan/layer selection/route planning.
- No target compile or display/SD hardware validation. Boat mission executor/autopilot/sonar/actuator functionality remains unavailable.

### ver029 — UNRELEASED development: online/offline map architecture
- Approved policy: online Google Maps viewing via authorized Map Tiles API 2D tile requests only; offline packs use independent data sources whose licence permits storage/conversion. Google tile imagery is not stored on SD or used offline.
- Added AUTO/ONLINE/OFFLINE map-source preference scaffold persisted in NVS. ONLINE path is not implemented; currently reports unavailable and falls back to SD.
- Added 1km-area/XYZ tile planner, per-zoom tile counts and storage estimate tool.
- Added local-MBTiles subset exporter with RGB565 conversion, missing-tile report, attribution/licence metadata; no map download function.
- Offline map manifest validation now requires complete-pack flag, attribution, licence URL and zoom-level declaration. Attribution is rendered on map.
- Added architecture, area planning/export, Google integration and milestone documentation; source-level tests written but not executed.
- Added ver029.1: CSV per-tile coverage index generated by MBTiles exporter; firmware requires/validates index and gates tile reads against present entries. Added Map Manager overlay for GPS/manual centre, 1km area, zoom selection 14–19, approximate storage estimate, pack stats, selected-area coverage and progress bar.
- Pending: staged pack installer/downloader, lawful source/download workflow, online provider/session/TLS/key handling/temporary decode, compile and hardware tests.
- **Not released:** source marker and package version remain 028; no ver029 ZIP or checksum.

### ver029.1 — coverage index and map manager UI (development update)
- Added `/maps/coverage_index.csv` with one row per planned XYZ tile and present/missing status; generated by the MBTiles area exporter.
- Firmware loads and validates the index at SD startup and refuses map tile reads unless coverage index marks the tile present. Coverage summary is exposed for UI.
- Map Manager overlay supports GPS centre or coordinate nudging, fixed 1km area, zoom 14–19 (14–17 selected by default), storage estimate, SD pack index statistics and selected-area coverage verification with progress bar.
- Progress is coverage verification only; no staged pack install or network transfer yet.
- Source-only, not compiled or hardware tested.

## Current implementation architecture (snapshot)
- Touch UI: Waveshare ESP32-S3 4.3-inch 800×480/LVGL9 target; target board revision/pin map still needs verification.
- Controls: two joysticks, two 2×4 button banks; 15 scanned keys plus separate OK; power key above display. R6 CAD remains preliminary.
- Primary link: nRF24L01+ PA/LNA, 2.4GHz.
- Backup link: EBYTE E22-900T22S UART LoRa; operational RF profile and legal duty-cycle validation pending.
- Map storage: microSD, RGB565 tile packs.
- Offline maps: renderer and export tooling exist in source, but compile and device testing are outstanding.
- Google Maps: design only; not yet rendered online.

## Outstanding verification and safety gates
1. Compile the Remote firmware against the exact Waveshare board and LVGL package.
2. Verify exact pin map, I2C addresses, ADC ranges, joystick calibration and key input hardware.
3. Run host tests in a reproducible environment and record results; ver029 tests have not yet been run.
4. Test SD map pack validation, tile decode/render, zoom, attribution, missing tiles and mission-file recovery on hardware.
5. Implement and test online map provider integration without persistent Google tile caching; confirm licence, billing, key protection and quota.
6. Implement coverage index and map manager/import workflow.
7. Complete secure session establishment, replay/reboot protection, ACK lifecycle and Boat telemetry/executor integration.
8. Native schematics, ERC/DRC, PCB layout, exact module pin verification and mechanical fit.
9. Verify motor/solenoid current, drivers, flyback, fuse, battery/BMS, thermal and radio compliance.
10. Do not connect actuator power or operate the boat until the relevant safety and hardware gates pass.

## Supporting records
- Release notes: `Remote/releases/RELEASE_VER017.md` through `RELEASE_VER028.md` where present.
- Current master status: `Remote/docs/PROJECT_MASTER_STATUS.md`.
- ver028 map renderer: `Remote/docs/OFFLINE_MAP_RENDERER_VER028.md`.
- ver029 map source milestone: `Remote/docs/MAP_SOURCE_UI_MILESTONE_VER029.md`.
- ver029 area export/planning: `Remote/docs/OFFLINE_MAP_AREA_DOWNLOAD_SPEC_VER029.md`.
- Google online design: `Remote/docs/GOOGLE_MAP_TILES_ONLINE_INTEGRATION_VER029.md`.
- Offline map policy: `Remote/docs/ONLINE_OFFLINE_MAP_POLICY_VER029.md`.


### ver029.2 — staged pack installation progress (development)
- Added SD staged-pack import from `/map-import/` to active `/maps/` storage. Stage the contents of the exporter-created `maps/` directory (manifest, coverage index and style tile folders) beneath `/map-import/`.
- Import accepts only complete packs with attribution/licence metadata and a coverage index. It copies tile files in 4KiB chunks, validates each tile's expected 131072-byte size, then copies the index and manifest last; the manifest is removed before installation begins so a partial replacement is not presented as valid.
- Progress reports tile count and copied bytes; completion revalidates pack and refreshes map tile cache. Failed import leaves no active manifest and is fail-closed.
- Firmware-side import has not been compiled or tested; staging/import is source-only.
