# Flytec V080 Retrofit — Master Status R10

Updated: 2026-11-20. Development branch; no firmware compile, hardware test, ERC/DRC, or operational release has been validated.

## Selected architecture
- Retain Flytec V080 hull, main switch, battery indicator, two 550 brushed motors.
- Replace boat control electronics.
- Remote: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480/LVGL9.6.0, touchscreen, two joysticks,16 physical positions (15 scanned keys + separately scanned OK).
- Primary radio: nRF24L01+ PA/LNA on both ends,2.4GHz,250kbps initial.
- Backup: matching EBYTE E22-900T22S UART LoRa modules on remote and boat. Tracker integrated SX1262 unused for first test.
- Remote LEFT antenna=2.4GHz nRF; RIGHT=868MHz LoRa. ESP32 Wi-Fi/Bluetooth uses onboard antenna.
- Mission: manual waypoint save→autopilot to waypoint→bait/hook release→return home. Boat-side failsafe must return only if GNSS/heading/battery/geofence health valid; otherwise stop safely.

## Remote hardware R5 — mechanical/button revision
- R5 shell CAD envelope310×209×69.5mm; two-piece front/rear shell prototype, central 4.3in display, twin sticks, two **2×4 button banks (two columns across, four rows high per side)**, OK and power keys, dual antenna exits.
- Right bank top four keys are independent H-L/H-R/K-L/K-R release controls. Combined release commands are disabled/rejected.
- MCP23017 physical button scanning/debounce, mapping and release/OK event wiring are in remote loop; GPIO crosswalk and actual hardware remain unverified.
- Remote release commands require authenticated primary link, STOP-confirmed released and boat interlock telemetry. Host-tested 1.2s release-key + 0.8s OK confirm within4s guard exists. `actuatorSafetyReady` only becomes true with fresh authenticated primary-origin STOP/interlock telemetry; boat-side telemetry sender is not implemented, so the release guard remains fail-closed.
- CAD source: `Remote/New_Handheld_Remote/CAD/`; mechanical and physical key map R5 documents.

## Remote hardware R4
- Enclosure target300×210×70mm; mechanical envelope provisional.
- 1S4P18650 pack, fuse/protection candidates.
- Controller/display 5V: Waveshare CS8501 onboard boost.
- nRF radio3.3V: dedicated TPS63070.
- LoRa E22 VCC5V: dedicated BAT_PROT→TPS61023 boost, not parallel with CS8501.
- E22 UART through SC16IS750 I2C bridge@0x48; UART9600 8N1; 3.3V logic.
- Provisional nRF pins CE=GPIO6(Sensor AD unused),CSN=GPIO15(RS485 TX path; RS485 cable disconnected),SPI11/12/13,channel76. IRQ→MCP23017 0x21 GPA1. Exact board revision, pin crosswalk, loading and continuity must be verified.
- I2C8/9; ADS1115 axes@0x4A and monitor@0x4B; MCP23017 buttons@0x20/radio IO@0x21.
- Carrier PCB target100×75mm,4-layer1.6mm,2oz outer copper candidate; size/stack fit provisional.

## Boat hardware R2
- Reported3S4P pack and20A BMS; confirm.
- Controller candidate Heltec Wireless Tracker ESP32-S3+UC6580GNSS; exact revision/pins not frozen.
- nRF24 primary; matching E22 LoRa backup.
- E22 VCC from5V electronics buck (candidate≥2A peak);UART/mode logic3.3V.
- Motor driver Cytron MDD20A candidate; board88.9×78.74mm exceeds77×79mm carrier envelope, requires separate placement or mechanical revision.
- Bait FW0630 009 and hook FW0630 001 nominal12V. Coil current/voltage tolerance, MOSFET/flyback/fuse and supply unverified.
- Boat control carrier77×79×14mm measured preliminary; lid clearance~30mm. Full CAD fit not done.

## Radio configuration
- nRF: channel76,250kbps,fixed32,CRC16,auto-ack/retries,PA_LOW initial. Pair addresses configured.
- E22: network/address0,UART9600 8N1,air62.5kbps,subpacket32,10dBm,channel18=868.125MHz,transparent. Config write/readback in code.
- 868.125MHz is in1% duty sub-band. E22 channel step1MHz cannot select869.525MHz 10% sub-band. Estimated25ms/frame; 3s interval≈0.83%. Actual airtime and persistent duty counter required before field operation.
- LoRa backup limited to STOP/RETURN, low-rate telemetry and restricted steering if airtime budget allows; no sonar and no mission/release actions.

## Firmware implementation scaffold
Remote:
- nRF24 transport, E22 transparent UART driver, SC16IS750 bridge, bounded RX queues.
- CRC16+HMAC packet codec, command codec/backup restrictions, LinkManager states, primary/backup heartbeat scheduler, UI link status.
- Key store returns false by default; no authenticated link or command TX until key provisioning.
Boat:
- nRF/E22 transport classes, authenticated receive/queue, duplicate-action cache, local failsafe evaluator.
- Boat dual-radio port is not connected to a main application; no motor/solenoid execution, telemetry/app ACK or autopilot integration.
- Session start/sequence scaffold is incomplete for persistent replay protection and secure reboot/session negotiation.
- Neither remote nor boat source compiled or bench-tested in this branch.

## Test sketches
- Remote/Boat nRF24 bench probes.
- Remote SC16/E22 and Boat direct-UART E22 bench probes.
- Fixed CRC-only probe frames; no authenticated commands or actuators. Boat pin macros disabled until board audit.

- E22 bench configuration byte decode is documented in `Remote/docs/E22_CONFIGURATION_DECODE_R1.md`.
- Path audit corrected: test sketches were present; initial fetch failure was retrieval/path-check issue.

## Electrical/PCB status
- Detailed electrical capture sheets, pin tables/CSV, power net drafts, BOM candidate docs and Mermaid block diagrams exist.
- Native KiCad schematic, ERC/DRC, footprints, PCB layout and fabrication files **not created**.
- Remote hardware test-build spec: `Remote/New_Handheld_Remote/PCB/REMOTE_HARDWARE_TEST_BUILD_R4.md`.
- Remote schematic capture: `Remote/New_Handheld_Remote/PCB/SCHEMATIC_CAPTURE_SHEETS_R4.md`; netlist `REMOTE_NETLIST_BLOCKS_R4.md`.
- Remote power nets: `Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R8.md`; capture spec `POWER_SHEET_CAPTURE_SPEC_R6.md`; BOM `REMOTE_POWER_BOM_CANDIDATES_R6.md`.
- Boat test build: `Boat/docs/BOAT_HARDWARE_TEST_BUILD_R2.md`.
- Boat electrical capture: `Boat/docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md`; schematic sheets `BOAT_SCHEMATIC_CAPTURE_SHEETS_R2.md`.

## Versioned ZIP status
- [Build and checksum status](../../docs/ZIP_BUILD_STATUS_R1.md)
- Preserved baseline archives: Remote `Flytec_Remote_ver004.zip`, Boat `Flytec_Boat_ver004.zip`.
- Current source markers are Remote `028` and Boat `014`. Latest source snapshot is Remote ver028 / Boat ver014. Packaging workflow should create the ver028 Remote ZIP after this branch push; compilation and archive verification still need checking.
- Packaging policy: `docs/ZIP_RELEASE_POLICY_R1.md`.

## Boat controller pin audit
- New `Boat/docs/BOAT_CONTROLLER_PIN_AUDIT_R1.md` reserves onboard SX1262, GNSS, TFT, USB, power and boot/strap resources based on manufacturer documentation.
- Exact Tracker revision is unknown. All boat nRF/E22 GPIOs remain unset; GPIO17/18 are only preliminary UART candidates and must be verified against the exact schematic.
- Boat connector table R3 emphasizes safe reset states and remains TBD for MCU pins.

## Freeze/test blockers
1. Exact Waveshare and Heltec board revisions/pin maps and connector continuity.
2. E22/nRF physical modules, antennas and RF bench test.
3. TPS61023/TPS63070 component-level schematics, layout and thermal validation.
4. Battery pack/BMS/fuse/wiring current/thermal measurements.
5. Motor startup/stall current and motor-driver fit.
6. Solenoid resistance/current/voltage tolerance and driver design.
7. Secure key provisioning, replay protection/session negotiation, app ACK and boat main loop/failsafe/actuator integration.
8. EU/Bulgaria RF channel/EIRP/duty-cycle compliance.
9. Native KiCad/ERC/DRC and physical mechanical fit.

## Release status
- Repository branch: `hardware-ready-dual-radio-r1`; draft PR #1 remains open and unmerged.
- Last package markers previously were Remote ver024 / Boat ver014. Remote source has advanced to ver028; package workflow output and checksum must be verified after branch push. Boat ver014 archive independently verified. Remote ver024 binary remains unverified.
- Do not operate boat or connect actuator power. Bench tests only.

- Consolidated remote pin map and connector CSV R4 are the current versions; older R2 docs marked superseded. Consolidated remote power BOM/capture spec R6 are current; R5 docs marked superseded.

- LoRa TX-slot scheduler updated: one E22 frame/endpoint/3s target; remote pending queue priority STOP>RETURN>manual, and heartbeat suppressed while a command is queued. This is provisional until actual airtime is measured.


## Current sync notes
- Root/Boat/Remote README files and docs/IMPORT_STATUS.md now reference the pin audit, ZIP version markers, and packaging policy.
- Repository path discrepancy was corrected in docs/REPOSITORY_PATH_AUDIT_R1.md: the four radio bench sketches and two READMEs were present.
- E22 register/profile decode is recorded in Remote/docs/E22_CONFIGURATION_DECODE_R1.md. The exact purchased module variant and actual register readback must still be confirmed.


## Remote software progress update — 2026-11-20

- Added library-free ADS1115 four-channel joystick input driver; provisional address 0x4A, AIN0–AIN3.
- Wired the joystick processing callback into the main Arduino sketch. ADC errors fail closed and inhibit stick motor commands.
- Added joystick calibration NVS persistence and validation. Invalid or absent calibration remains fail-closed; completing endpoint capture marks calibration valid.
- Added settings controls for Tank/Steering layout, autopilot manual-override behavior, dead zone and opening the calibration UI.
- Added repository SVG front-panel preview and updated R5 physical button concept.

**Not verified:** exact Waveshare GPIO/I2C pins, ADS1115 address straps, analog input voltage ranges, compile against target board/LVGL, ADC hardware tests, radio/boat operation. Do not enable propulsion until calibration, ADC, radio, command safety and boat-side failsafe tests pass.


## Remote concept R6 update (2026-11-20)
- Styling/layout is now being aligned to `Remote/New_Handheld_Remote/CONCEPT/Flytec_Remote_Concept_Reference.jpg` (wide ergonomic grip silhouette, raised display bezel, upper joystick pods, central power key, dual antenna exits).
- User-confirmed physical button arrangement: **2 columns × 4 rows on each side** (8 keys per bank), replacing the previous 2 columns × 4 rows.
- R6 concept SVG and mechanical/button-map documents are committed. These remain visual design references only; the enclosure CAD has not been dimensionally redesigned or checked for component fit yet.


## Remote mechanical CAD R6 progress (2026-11-20)
- Added parametric OpenSCAD source `Remote/New_Handheld_Remote/CAD/Flytec_Remote_Enclosure_R6.scad` with 4-column × 2-row button banks on each side, central touchscreen opening, twin stick apertures, central power key, USB-C opening, rear antenna ports, rear tray, preliminary screw bosses, display support ledges, joystick support rings and assembly-preview geometry.
- CAD workflow expanded to export R6 front/rear/button-cap STL and assembly PNG in addition to R5 artifacts; print package and checksum names revised to R5/R6 combined.
- Added R6 mechanical draft and limitations documentation. No CAD render/export has yet been verified by the workflow; dimensions/components are placeholders, and no fit validation has been performed.


## R6 enclosure CAD validation (2026-11-20)
- Reworked R6 parametric enclosure with target 310×209mm perimeter, 66.5mm rear tray, 3.5mm front panel, raised 2mm preliminary collars around release keys, OK/confirm key in the right-bank bottom-right position, electrically separate on MCP23017 0x21 GPA4, and power switch above display.
- Antenna pass-through positions shifted to Y=82mm. Four release-button apertures are enlarged to 18mm; collar OD21mm/ID17.6mm.
- A reconstructed local R6.6 source mirror exported front, rear and button-cap meshes; front/rear watertight, and button plate has 16 caps. Assembly/internal preview exports timed out. The repository SCAD was not transferred byte-for-byte for independent compile verification.
- Local files are preliminary validation only. The real CAD workflow has been updated to export R6 front/rear/buttons, angled preview, and orthographic front-layout preview. GitHub Actions results and generated repository STLs/ZIP are still pending verification.


## Remote CAD R6.1 internal fit draft (2026-11-20)
- Rear shell CAD now includes sculpted palm pads, 100×75mm placeholder carrier-board mounting posts, and a grip-mounted 1S4P battery cradle envelope.
- Battery assumption: four 18650 envelopes (Ø18.6×65mm), two in each grip, stacked through shell depth. This is an initial arrangement only; no cell restraint, insulation, pack wiring, fuse/BMS, charge or thermal design is complete.
- Added OpenSCAD preview modes for internal component layout and battery cradle, plus GitHub workflow exports for internal/top preview PNGs and battery cradle STL.
- Locally exported meshes: front panel and rear shell watertight one-part meshes; button-cap plate watertight with 17 parts. Rear shell is now a larger integrated mesh and requires print orientation/support review.
- Prototype external depth including grip pads and front/collar stack is approximately 87mm. This replaces the earlier ~70mm nominal estimate for the R6 shell without palm pads.
- Fit remains unverified against actual Waveshare board, sticks, switches, antennas, connectors, battery cells or PCB.


## Remote CAD R6.1 internal fit draft (2026-11-20)
- Rear shell CAD integrates sculpted rear palm pads, 100×75mm carrier mounting posts, and a grip-mounted 1S4P battery cradle placeholder.
- Battery arrangement assumption: four 18650 envelopes (Ø18.6×65mm), two per grip, stacked in shell depth. Cell restraint, insulation, pack wiring, fuse/BMS, charge and thermal design are not complete.
- Lower-center body bridge extended for a provisional USB-C opening; exact connector location still awaits measurement.
- Added internal layout and battery cradle CAD modes; workflow exports internal/top previews and cradle STL.
- Local mesh tests: front panel and rear shell watertight, each one component; button plate watertight with 17 components; cradle feature mesh watertight with two components. CAD generation only, no hardware fit validation.
- Overall shell depth including grip pads and front/collar stack approximately 87mm, revised from the prior ~70mm estimate.


## Remote fit coupons R6.1 (2026-11-20)
- Added small printable tolerance coupons before a full remote enclosure print: button apertures 13.8–15.0mm, joystick apertures 24.5–26.0mm, USB slot matrix 15.0–16.0×8.0–9.0mm, screw holes 2.8–3.4mm, and release guard apertures 17.6/18.0mm with 21mm collars.
- OpenSCAD coupon source and guide committed. Locally generated STL coupon meshes checked watertight. Labels are test aids; no physical fit has yet been measured because target components are not purchased/available.
- GitHub CAD workflow now includes export commands for the four coupon STL files.


## Archive review and R6.4 reconciliation (2026-11-20)
- Reviewed version archives Boat/Remote ver001–005, Remote ver009, and GitHub Import ver004. Remote ver005 was an integrity-verified Actions snapshot; Remote ver009 contains newer R5 electrical/CAD work but remains an unvalidated development bundle.
- Correct R5 baseline retained: MCP23017 0x20 for 15 discrete inputs (8 left + 7 right; GPB7 spare), not a matrix; MCP23017 0x21 for radio IO and separate physical OK at GPA4; ADS1115 0x4A joystick axes and 0x4B monitoring; SC16IS750 0x48 E22 UART bridge; nRF SPI GPIO11/12/13 shared with TF, CE GPIO6 and CSN GPIO15 direct candidates only after onboard circuits are isolated; E22 UART stays on bridge.
- Recent preliminary matrix/ADS1015/alternate GPIO documents are explicitly marked superseded by this reconciliation.
- New records: Remote archive reconciliation and next gate Remote/New_Handheld_Remote/ELECTRICAL/ARCHIVE_RECONCILIATION_AND_NEXT_GATE_R6_4.md; consolidated schematic netlist review draft REMOTE_SCHEMATIC_NETLIST_R6_4.md.
- Next engineering gate: reconcile connector pinout/netlist and power draft, then native schematic capture and ERC. No compiled or bench-tested hardware status changes.


## Remote firmware integration review
- `REMOTE_FIRMWARE_INTEGRATION_REVIEW_R1.md` identifies source integration gaps.
- `REMOTE_TELEMETRY_COMMAND_CONTRACT_R1.md` proposes telemetry and command ACK payloads; not implemented.




## Archive cross-review and payload codec milestone (2026-11-20)
- Re-read archived Remote/Boat ver001–005, Remote ver009, R6.3 internal-layout and R6.4 concept-preview packages.
- Added R6.5 electrical netlist cross-review: key outstanding issue is the proposed 1S4P (~10Ah) pack versus the project note that the Waveshare onboard charger recommends battery capacity below 2000mAh. Charger/current/power-path/thermal compatibility must be verified before battery connection or USB charging.
- Added portable eight-byte HEARTBEAT/ACK/TELEMETRY payload decoder on both endpoint source trees, with strict reserved-bit/range checks and host tests. Corrected contract duplicate sonar subtype in R2.
- Decoder is not integrated into frame dispatch, ACK lifecycle, telemetry freshness/coordinate pairing, or boat-side telemetry generation. No commands or actuators are enabled by this milestone.
- Next: wire authenticated receive dispatch to decoder; implement ACK correlation/pending/timeout; update heartbeat sender fields; apply partial telemetry with per-group freshness; then host-test malformed, stale, duplicate and failover cases.


## R6.6 physical button/OK crosswalk (2026-11-20)
- Approved layout is two columns × four rows per bank. One power key above display; OK is right-bank bottom-right, no duplicate power key.
- Total: 15 scanned key inputs on MCP23017 0x20 plus separate OK on MCP23017 0x21 GPA4. GPB7 spare.
- CAD source and scanner updated. Repository workflow STL/preview exports still require verification; no physical fit/test.
- Added `REMOTE_BUTTON_IO_CROSSWALK_R6_6.md` and `PHYSICAL_BUTTON_MAP_R6_6.md`.

## Firmware utility progress (2026-11-20)
- Added portable strict payload decoder on Remote and Boat for heartbeat, ACK, telemetry status, battery/speed, coordinate, navigation and sonar-summary payloads.
- Added Remote ACK lifecycle tracker with pending/accepted/executed/rejected/timeout states and backup-link restrictions.
- Host tests pass locally for payload decoder, ACK tracker, partial telemetry/freshness, release guard and scanner syntax. Remote receive dispatch, ACK tracking and telemetry state application are now wired. No full ESP32 firmware build; boat-side telemetry/ACK execution remains unimplemented.


## R6.6 preview asset
- Added `CONCEPT/REMOTE_R6_6_FRONT_LAYOUT.svg`, a repository-saved vector preview of the updated two-column/four-row banks, one power key and right-bottom OK input. A PNG layout preview is also rendered locally.


## Remote radio integration R2 (2026-11-20)
- Authenticated receive dispatch validates frame/session/flags/reserved bytes; per-link plus cross-radio sequence guards reject stale/replay frames and avoid double-applying mirrored packets.
- Remote ACK tracker is connected to non-manual command sends and inbound ACKs; exposes status for UI, not yet displayed.
- Remote telemetry is applied per group with local freshness, GNSS sample pairing, primary provenance and release readiness gates.
- Heartbeat payload fields and nonzero 16-bit action IDs corrected.
- Boat-side session-echo, per-radio sequence guards, pending-command deduplication and queue-level ACCEPTED/DUPLICATE/BUSY ACK scaffolds are present, but no telemetry source, command executor, EXECUTED ACK or actuator lifecycle is integrated. Session recovery remains weak without a challenge/response protocol. Full firmware build and RF/hardware test pending.
- New review: `REMOTE_FIRMWARE_INTEGRATION_REVIEW_R2.md`; contract R3.


## Remote UI/input milestone — ver024 (2026-11-20)
- Added touchscreen mode request controls (MANUAL / AUTOPILOT / RETURN HOME) in the operating footer. They use the existing command request API; authoritative acceptance/execution remains boat-side and unimplemented.
- Live joystick input diagnostics show calibration state, ADC read fault, raw values when uncalibrated, normalized values and neutral/active status when calibrated.
- Compact safety strip shows STOP state/confirmation, failsafe or low battery, interlock readiness and pending ACK count.
- Battery percentage now displays unknown when telemetry is absent/stale rather than falsely showing zero.
- **Validation boundary:** source review only; full target compile and hardware tests remain pending. No commands can operate with unprovisioned authentication key; no boat execution path is integrated.


## Package status reconciliation
- Historical ver024 snapshot: its package ZIP/checksum status was recorded at that point. Current remote source marker is ver027; the new archive/checksum is pending.


## Remote mission UI milestone — ver025 (2026-11-20)
- Dedicated waypoint/mission planning screen opened from footer.
- Four RAM-only waypoint slots, selection, clear, save-current-fix and set-home.
- GNSS save/home require valid state and timestamp no older than 2 seconds.
- Selected waypoint can receive BAIT/HOOK planning markers. No actuator command is sent.
- Mission start is blocked; no persistent waypoint store, boat mission executor, geofence validator, secure command implementation or mission ACK/state integration exists.
- Preview file: `Remote/source/UI_PREVIEW_MISSION_VER025.svg`.
- Also repaired missing declarations for ver024 input/safety footer labels discovered during source review.
- ver025 source is not yet compiled against the actual board/LVGL package and is not hardware tested.


## Remote ver027 waypoint storage and map overlay (2026-11-20)
- Waypoint/home and planned bait/hook flags now persist in ESP32 NVS Preferences with record magic + CRC32 check.
- Main placeholder map adds relative waypoint/home markers; markers are hidden when GNSS is invalid or more than 150m from the boat. This overlay is only illustrative, not a navigational map or route.
- The mission UI's save-current and set-home require fresh GNSS (<2s). Selection and marker edits persist.
- Mission start remains blocked. No boat mission execution, geofence/route validation, release command or authenticated operational path.
- Source version marker Remote ver027. ZIP/checksum and target build/hardware test are pending.

- Hardware verification checklist: `Remote/docs/MISSION_NVS_TEST_PLAN_VER027.md` (not yet run).


## Remote ver027 update
- Waypoint capacity increased to 16 (NVS) plus Home. SD/microSD card is designated for offline map packs. Existing map tile reader path documented; no renderer/map pack generator yet. Target compile, SD-card test and NVS test remain pending.


## Remote ver027 waypoint capacity and SD map storage (2026-11-20)
- Mission storage upgraded to 16 waypoint entries plus Home, persistent NVS, CRC, and legacy four-slot migration.
- Mission UI uses 4×4 slot grid; map overlay can show all 16 in the local grid view.
- Integrated microSD/TF is map-data storage. Existing SD map pack reader checks manifest and raw RGB565 tiles; full map renderer and pack creation tool remain pending.
- Release notes and preview: `../releases/RELEASE_VER027.md`, `../source/UI_PREVIEW_MISSION_VER027.svg`. Current source marker 027. ZIP, checksum, compilation and hardware validation not done.


## Remote software progress update — ver028 offline map + SD waypoint persistence
- Added `map_renderer.*`: SD-backed Web-Mercator XYZ RGB565 tile viewport, bounded 3×3 tile slots, zoom buttons (5–19, default 15), boat marker, all 16 saved waypoint markers, Home and direct target route line.
- Map panel viewport expands with map panel. North-up only; no pan/layer selector or route planning. No map tiles bundled.
- Added desktop MBTiles-to-Flytec pack converter (`Remote/tools/mbtiles_to_flytec.py`) with TMS→XYZ row conversion and required attribution argument.
- Mission/Home record now writes SD file `/missions/waypoints.bin` with CRC/read-back validation and temp-file rename; NVS mirror/fallback retained and legacy four-waypoint NVS migration preserved.
- Tests: source edits not compiled. Verify LVGL image/line API compatibility, SD file operations, memory, blocking latency, zoom, tile placement, route overlays and persistence on the actual Waveshare display/card. See `Remote/docs/OFFLINE_MAP_RENDERER_VER028.md` and `Remote/docs/MISSION_SD_STORAGE_TEST_PLAN_VER028.md`.


## Remote ver029 development — online/offline map architecture
- Accepted design: Google Maps online viewing through authorized Map Tiles API 2D tile requests only; offline SD maps must use independent sources whose licences permit storage/conversion. Google tiles are never downloaded for offline use or stored on SD.
- Added map source preference/UI scaffold (AUTO/ONLINE/OFFLINE persisted to NVS). ONLINE currently unavailable and status reports SD fallback. This is not an online renderer.
- Added `map_area_planner.py` for 1km square geometry, XYZ tile plan, tile counts and storage estimates.
- Added `mbtiles_area_export.py` to extract a selected tile subset from locally supplied MBTiles, convert RGB565, and report missing tiles plus source attribution/licence metadata. It does not fetch map data.
- Firmware now rejects offline packs unless they declare complete coverage, attribution, licence URL and zoom levels; attribution is overlaid in the map view.
- Added docs: online/offline architecture, area download spec, Google Map Tiles API integration design and ver029 milestone.
- Example planner arithmetic at 43° latitude for zooms14–17: about50 tiles/6.25MiB raw/6.88MiB with 10% margin. Exact result varies by centre and tile boundaries.
- Source-level tests were added, but not executed. Online provider, area-download UI, per-tile coverage index, compile and hardware validation remain pending. Packaged ZIP and `releases/VERSION` remain ver028.


## Offline map coverage index and manager — 2026-11-20
- Added coverage index CSV generated by the area exporter, including explicit rows for missing requested tiles.
- Firmware requires manifest + coverage index and rejects tile reads not marked present.
- Added Map Manager overlay for current GPS centre, coordinate nudges, fixed 1km square, zooms 14–19, estimate and selected-area coverage check.
- Coverage progress bar is an area-coverage percentage; staged pack import separately reports tile/byte copy progress.
- No compile/hardware validation. Remote version marker remains 028.


## Offline-map coverage and staged import update — 2026-11-20
- Exporter now creates sorted coverage index with present/missing tile rows. Firmware coverage index is loaded in PSRAM (6000-row limit), sorted keys checked, binary-search lookups used by tile renderer.
- Map Manager UI: GPS centre, manual coordinate nudges, 1km square, zoom 14–19, storage estimate, pack/index status and area coverage bar.
- Staged import from SD `/map-import/` into `/maps/` is implemented in source using 4KiB chunk copies, per-tile size validation and manifest-last install; status shows tile/byte progress and renderer cache is invalidated after validation.
- Tests added for coverage-index export, not executed. Firmware compile and hardware tests pending. Remote version remains 028; ver029 development not packaged.


- Consolidated Remote history is maintained at `Remote/releases/CHANGELOG_MASTER_VER015.md` (history document revision VER008; source marker still ver028).

- ver029.3 robustness follow-up: coverage index uses PSRAM/binary search, import validates tile byte lengths/count and cache refresh. Added map-tools test workflow; CI result not confirmed.

- Map tools CI test fixture corrected to test a multi-tile export with both available and missing tiles. CI execution result still pending.

- Import manager includes PSRAM allocator dependency; compile validation remains outstanding.


## Importer audit hardening — 2026-11-20
- Import file-copy operation now handles explicit SD read/write errors and stops rather than looping/retrying indefinitely.
- Staged coverage-index CSV is validated for strict row format, only-present tiles, coordinate bounds, sorted unique keys and expected count before installation.
- Failure remains fail-closed: active manifest is removed before replacement and only restored by the final copy stage. Firmware compile, SD import test and CI status remain unverified.


## Google online renderer — phase 1 contract (2026-11-20)
- Existing Wi-Fi profile/connect UI confirmed; no internet-health service, Google session/API client, credential provisioning, image decoder, or online renderer exists yet.
- New contract document defines source modes, online layers, worker isolation, TLS, API session lifecycle, transient rendering buffers, attribution and fallback policy: `Remote/docs/GOOGLE_MAP_RENDERER_PHASE1_CONTRACT_VER029_7.md`.
- Architecture only; no Google requests or imagery are enabled. Implementation gated on credentials, quota/billing setup, decoder selection and target validation.


## Google online maps — source selector UI update (2026-11-20)
- Settings → MAP now has a real persistent AUTO / ONLINE / OFFLINE selector using the existing map-source NVS preference.
- UI explicitly communicates that AUTO is designed for online-first with SD fallback, ONLINE remains unavailable pending provider implementation, OFFLINE is SD-only, and Wi-Fi association is not proof of internet reachability.
- This is preference/UI only: no internet probe, Google API request, session/key provisioning, JPEG decoder, or online tile render is implemented. Map display remains SD renderer.
- Source file updated: `Remote/source/settings_ui.cpp`. Commit: `57ac81975845f8d958f49b09f86bcf51ee147613`.
- Not compiled or hardware-tested.


## Map settings connectivity display — 2026-11-20
- MAP settings now displays live Wi-Fi association state, refreshing every 2 seconds via LVGL timer. Internet reachability is deliberately labelled UNKNOWN (not inferred from Wi-Fi association).
- Timer is stopped and its label pointer cleared when navigating away from the category, preventing callbacks to a deleted LVGL object.
- This is nonblocking status only; no DNS/HTTPS internet probe or provider integration exists.
- Source: `Remote/source/settings_ui.cpp`; latest implementation commit `81fee88a162260c70fbff6c837e44822e10e071a`.
- Not compiled/tested on target.


## Operating screen UI redesign — 2026-11-20
- Reworked the 800×480 main screen layout: compact status header, 50/50 map and sonar panels, and a two-row footer with live joystick/safety state above larger touch controls.
- Added a consistent dark marine palette, blue panel outlines and button styling, red STOP styling, and Bulgarian labels for the main screen controls/status.
- Rebalanced map viewport and zoom controls for the new panel height. Full-screen map/sonar expand states were adjusted to match the new content height.
- Existing map manager, zoom, map-source cycle, mission planner, calibration, mode requests and STOP callback are retained.
- Sonar is still a placeholder: no CHIRP data feed/echogram is implemented. This is a UI layout update only.
- Source: `Remote/source/boat_ui.cpp`. Commits: `39006c91b759201b`, `5b5c95a76575f51f`, `0ddabd38f9d2d027`, `bbc257156d7f4117`.
- **Not compiled or LVGL Pro-tested.** Layout preview is conceptual; all touch targets, Bulgarian font glyph coverage, clipping, panel expansion, and screen transitions must be checked in the LVGL Pro simulator before hardware.


## Mission planner visual/UI pass — 2026-11-20
- Mission waypoint manager now uses the shared dark marine palette, blue action buttons with rounded touch styling, a blue panel outline and Bulgarian labels throughout its visible controls/status messages.
- The 16 waypoint slots remain arranged as a 4×4 grid. Slot select, save-current-fix, set-home, clear, bait/hook planning flags, back and disabled mission-start callbacks are preserved.
- Selected waypoint label clarified. Mission execution remains deliberately blocked; planning markers are not sent as boat commands.
- Source: `Remote/source/mission_ui.cpp`. Commits: `67e0a87e9cb24219`, `699936ef55033318`.
- Not compiled or LVGL Pro-tested; layout, Bulgarian glyphs and text fitting need simulator validation.


## Joystick calibration UI polish — 2026-11-20
- Calibration screen now shares the dark marine palette, blue accents and rounded outlined touch controls.
- Screen headings, button labels, status messages, axis names, layout mode, autopilot override and dead-zone readout translated to Bulgarian.
- Safety gating, raw/min/centre/max/normalized telemetry, endpoint capture, dead-zone slider and neutral-before-exit behavior retained.
- Source: `Remote/source/joystick_calibration_ui.cpp`; commit `59e0f81fb5676500`.
- Not compiled or simulator tested; text-fit and Cyrillic font support pending LVGL Pro validation.


## Settings UI theme pass — 2026-11-20
- Settings navigation and header updated to shared dark marine palette, rounded blue-outline buttons, and outlined nav/content panels.
- Main settings headings and category names localized to Bulgarian; internal category IDs unchanged.
- Existing Wi-Fi, display, controls, sonar, map source selector and system navigation preserved.
- Source: `Remote/source/settings_ui.cpp`; commit `9f5afc60294790c2`.
- Remaining category-body labels are still mixed English/Bulgarian and require a separate localization review. Not compiled or LVGL Pro-tested.


## Offline Map Manager UI polish — 2026-11-20
- Map Manager screen now uses the shared dark marine palette, rounded panel and blue outline, consistent blue touch buttons and Bulgarian labels for centre nudges, GPS centre, zoom selection, estimates, pack status, coverage, import and back.
- Existing 1 km area, zoom 14–19 selection, storage estimate, coverage index, staged-pack validation, chunked import progress and fail-closed behavior retained.
- Sources: `Remote/source/map_manager_ui.cpp`; commits `0908af0ebc95c905`, `1727fb5cf899e3ec`.
- Not compiled or LVGL Pro-tested. Long progress/error strings and touch layout require simulator review.


## Remote UI milestone — ver029.9 (2026-11-20)
- Main operating UI: Bulgarian status, 50/50 map/sonar panes, controls and safety footer.
- Sonar interface now includes a static demo echogram plot with depth guides and illustrative fish arches; no CHIRP module or real sonar data path is connected.
- Mission UI: 16 waypoint storage slots; selected waypoint coordinate and GPS-derived distance/bearing (fresh fix only).
- Settings localization pass; Wi-Fi/map/sonar options indicate unavailable integration rather than implying it works.
- SVG previews and test checklist committed. LVGL Pro and firmware validation are pending.
- Master changelog: `Remote/releases/CHANGELOG_MASTER_VER021.md` (VER015 superseded).
- Current source remains unreleased ver028 baseline; no ver029 ZIP or SHA generated.


## Packaging safeguard update
- Workflow now distinguishes a development snapshot from a release-version bump. Versioned repository ZIP preservation is gated on a change to Remote/Boat VERSION files; routine source pushes cannot replace the same release filename. This change is source-reviewed only; workflow run after guard update is pending.


## Archive integrity correction
- Prior packaging workflow generated and recommitted ver028/ver014 ZIPs on routine source pushes. This repeated archive loop is now fixed: preserve/commit is gated on a VERSION marker change.
- Latest guard workflow run succeeded; snapshot artifacts validated with unzip/checksum steps. Archive preserve step correctly skipped because version markers were unchanged.
- The existing branch ZIPs were not restored from an earlier immutable release; their release integrity is not asserted.
- Master changelog is VER018.


## UI follow-up (ver029.12)
- Echogram demo plot now scales/redraws for full-screen and split views.
- Remaining sonar setting choices localized.
- Selected waypoint distance/bearing numeric edge case clamped.
- No firmware compile or LVGL Pro test yet.
- Consolidated changelog VER019.


## Source cleanup — ver029.13
- Main UI source line-break cleanup and waypoint haversine clamp confirmed in repository source.
- Changelog VER020.
- Target build not run; use LVGL Pro before release.


## Continuation checkpoint synchronization — 2026-11-20
- New canonical restart document: `../../docs/CONTINUATION_CHECKPOINT_VER001.md`. It summarizes decisions, current status, safety restrictions and prioritized next work.
- UI source is unreleased ver029 development (latest polish ver029.13); static echogram remains demo-only. UI checklist `UI_BUILD_VER029_9.md` and SVG previews are available.
- Latest consolidated changelog VER021. Source markers remain Remote 028 / Boat 014; current branch archives are unverified snapshots.
- Workflow #880 previously succeeded with archive-preservation guard. Latest documentation sync triggered packaging run #888 (pending at last check); verify its outcome when resuming. No target compile or LVGL test has occurred.


## Source audit / compile preparation — VER022 (2026-11-20)
- Findings recorded in `docs/SOURCE_CONSISTENCY_AUDIT_R1.md`; consolidated development changelog VER022.
- Remote authenticated session markers now retry independently per link until confirmation (nRF 500ms; E22 3s duty-limited). Boat resends same-session replies for idempotent retry recovery.
- Fixes are source-only, not compiled or integration-tested. Auth key provisioning remains disabled/fail-closed.
- No reproducible target build manifest or dependency lock. Exact Waveshare/display-library compatibility and board SKU still require confirmation.
- Remote host-test workflow is missing; no complete host test run is claimed.
- Current markers remain Remote 028 / Boat 014; no ZIP/release bump. Branch archives remain unverified.


## Target compiler follow-up — 2026-11-20
- Target build run #11 reached C++ source compilation and failed; installed candidate versions are visible in its log.
- Fixed LVGL9 event-target accessor, precise route-point array type, and restored the Map Manager copy-chunk helper declaration.
- Rebuild pending. Host CI run #11 passed on the prior source; target compile and physical hardware verification remain outstanding.


## Compile correction tracking
Target Compile #14 failed on missing declaration for `g_map_attribution` in the offline map module. Added the private 256-byte attribution buffer (commit `9aa20b0d987f94ea5b49e46509ae0ba10cc9d36b`). Target Compile #15 pending; complete target compilation remains unvalidated.


## CI build status — 2026-11-20 (latest)
- Remote target compile #19 / run ID 44781879101 passed on commit 3f189cb1ac65a764aec678893f1df857d113c815; full sketch compile/link and firmware artifact upload succeeded.
- Build uses ESP32-S3 16MB Flash, OPI PSRAM, huge_app partition. Size 1,430,957 bytes / 45%; globals 178,136 bytes / 54% (149,544 bytes remaining). No C++ errors or warnings reported.
- This validates CI compilation only. Actual display/touch, Wi-Fi, SD, radio, input and safety behavior require physical testing. Keep key provisioning and control TX disabled/fail-closed.
- Next: audit settings/Wi-Fi timer teardown/re-entry, Map Manager parser/import safety, renderer cache/PSRAM and LVGL lifecycle; fix issues and rebuild.


## Eight-lake development checkpoint — 2026-11-20
Remote package marker remains 028; Boat 014. Source now has eight SD lake profiles, sixteen mission slots and separate Home per profile, checked active selection, recoverable name and mission records, and A/B offline map installation. Lake selection UI uses standard LVGL keyboard and confirmed STOP/manual/connected-radio/inactive-motor gates. Mission execution remains blocked; boat-side synchronization is not implemented.
Latest observed before this update: host #41 passed (6a3d88c), packaging #977 passed, target #42 running. Host #41 did not exercise the new SD storage. This update adds a mock SD host test compiling actual lake_storage.cpp: eight profiles, safe switching, name persistence, isolated paths, map marker rename failure, short writes and unavailable card. Results pending. The mock does not test actual FAT/SD durability, mission serialization or LVGL. Verify new CI and fix real failures before bumping the release marker. Hardware remains untested.


## Verified test handoff — 2026-11-20
Source SHA 8030ecab6cbccb64204e638b4c398b588681515e. Host #43 failed on missing sdMissionAvailable reference after extraction; corrective commit 8030eca replaced it with lakeStorage_ready. Host #44 passed (run 44793207359), including actual mission serialization for 128 waypoints, per-lake Home and CRC/rename/short-write simulation. Packaging #980 passed (44793207717). Target compile #45 (44793207383) is running; previous #43/#44 were cancelled by newer pushes, not passed. Do not change firmware merely to retrigger CI while the current target run is active.
Next: inspect #45 compiler result and fix actual diagnostic if any; extend coverage/import parser and fault-injection tests, audit remaining source modules in manageable batches (repository inspection is not an exhaustive line-by-line audit), validate UI lifecycle and SD runtime, then bump Remote package marker 028 to 029 only after software gates pass. Boat remains 014 and hardware/actuation unvalidated. Repository project marker docs/PROJECT.md is not a ChatGPT Project. Existing root legacy map and NVS-only legacy data require follow-up; standard keyboard is not a dedicated Bulgarian keyboard.


## Eleven-stage execution handoff — 2026-11-20
**Current firmware source:** 3d363f07c84c24cb12c5ecd7be2c52b5f15e8897; test-only follow-up 7ec7a7a5f313f8d330233d1941ca7ff4ff0ca6e5.
- Remote host #50: PASSED, all ten groups (44795330911). Includes actual map manifest/coverage parsers, projection/pan, 128-waypoint mission storage, matching portable Remote/Boat mission codecs and lifecycle, physical input/OK/release arbitration and lake zoom persistence. Boat portable #15 PASSED (44795188700). ZIP snapshot #986 PASSED (44795188825). Target #50 (44795188918) was still installing pinned ESP32 core; no full compile result yet. Do not push unnecessary firmware changes that cancel the target run.
- 1: #45 escaped duplicate Wi-Fi state declarations fixed (2db6318); complete latest target compilation and remainder of branch audit still open.
- 2: shared strict map manifest and coverage parser/count checks tested (bec5550); mock recovery passed, real SD and runtime/cache/import failure testing outstanding.
- 3: physical waypoint, settings, map, sonar, back and OK actions connected; scanner STOP/fault/release arbitration tested (70c7f9e, 7ec7a7a). Hardware and UI integration validation outstanding.
- 4: mirrored eight-byte candidate mission protocol/assembler (81dd926), host tests passed. Authenticated live dispatch, ACKs, transaction and release-mask UI integration still open.
- 5: non-actuating mission feedback state machine tested. No live autopilot, pause/resume/return execution, accuracy gating or override integration. Non-manual command requests explicitly blocked pending synchronization.
- 6: diagnostic screen and Boat unsynchronized-clock bug corrected (3d363f0). Secure key provisioning, authenticated session/replay redesign and radio bench testing open; key store deliberately fails closed. Boat command execution must enforce local queue age.
- 7: north-up touch pan/recenter, per-lake SD zoom, panned route origin, enlarged 4x3 tile footprint, descriptor fixes and retry cooldown implemented. Boat-up rotation, async tile loads, full preferences and hardware display tests open.
- 8: diagnostics, settings back/timer cleanup, sonar demo point lifetime corrected; Cyrillic font asset/keyboard, full localization and display layout validation still open.
- 9: Remote release remains 028; 029 NOT released. Boat remains 014. Snapshot workflow success is not release certification. Bump only after software gates.
- 10: physical unit not available; boat integration/actuator/range tests not performed.
- 11: sonar module/interface not selected or verified; demo only; no real driver or data.
Next action: read target #50 result/log and correct actual errors, then continue stages in the approved order. Recheck CI on exact new source and update this handoff; no claim that all eleven stages are complete. 


## Target Compile #50 — verified successful (2026-11-20)
Supersedes the pending #50 statements above. GitHub Actions run 44795188918, job 136492949945, completed compile/link and firmware/log uploads successfully. 1,435,681/3,145,728 bytes flash (45% reported); globals 178,808/327,680 bytes (54%), 148,872 bytes remaining. ESP32 core 3.0.2, LVGL 9.6.0, Display Panel 1.0.0, IO Expander 1.0.1, esp-lib-utils 0.1.2, RF24 1.4.11. No C++ error/warning diagnostics found; esptool SyntaxWarning and Node/Actions deprecations are nonblocking. Host #50 (44795330911) verified all ten groups successful; this is a previously completed run, not a fresh rerun. Packaging #986 (44795188825) emitted Remote 028 and Boat 014 snapshot artifacts for 3d363f07c84c24cb12c5ecd7be2c52b5f15e8897, plus checksum artifact; archive contents not independently verified. Repository has subsequent mechanical work; do not assume these runs cover every subsequent commit. No release marker increment or actuation enable. Next: compare current branch to tested source, finish code audit and extend map import interruption/cache/runtime tests; re-run CI for any source fixes. Keep mission dispatch and actuator outputs fail-closed. Physical validation and real sonar remain outstanding.


## Stage 2 follow-up — bounded import index
Commit 2ec3a0a5b54fd0114157ed5306ce876207b38903 bounds staged coverage index size before synchronous parsing (including blank lines), rechecks index/manifest sizes before copying and cleans up on rejection. Fresh target, host and snapshot CI required; results not yet verified. Next: observe new CI, add oversized/blank-line importer regression and continue interruption/cache/runtime audit. Remote 028 / Boat 014 unchanged.


## Stage 2 installed-index and initial-publication regression — 2026-11-20
- abd61e48c48dc0eef00c3ab643a101a7212d4de9: installed coverage reader now rejects indexes larger than (6000+1)*48 bytes before allocation/parsing.
- 0425ee14974a3edb456c8d2b1d5b108b43a389d5: host test oversized blank-line padding, invalidation of prior coverage, recovery on valid reload.
- 3d4048c0349594bb9e52c75cf4d221676a21598e: host test initial A/B map publication short write and failed rename followed by reboot; prior implicit slot must survive.
- New test and build outcomes not observed. Commit workflow lookup returned no runs (connector only lists PR-triggered runs); do not infer push CI outcome. Next: verify actual push CI in Actions, add integrated importer copy/validation fault tests and renderer lifecycle tests, then proceed with remaining stages. Target #50 remains historical last verified target. Remote 028 / Boat 014 unchanged; no ZIP regeneration or hardware claim.


## Stage 2 renderer lifecycle and copy bounds follow-up
- a59c8d421619807918972abb3c17115f66ecc563: guard duplicate viewport creation and updates after deletion; detach LVGL image sources/drop cache entries before freeing tile memory; release tiles when map unavailable or viewport slots out of bounds. Target compilation and LVGL deletion-order/cache behavior require verification.
- 9b212be2cf5c1cdefa87728b2dad2164731b38f4: bound source size at copy open, reject chunk exceeding expected file or total pack bytes, require exact total before staged map publication. Integrated copy fault tests remain outstanding; exact byte count does not prove content integrity.
- Fresh CI results unavailable via PR-only lookup; local clone blocked by DNS/network. No new test pass or archive synchronization claimed. Next: verify push CI; extract/test copy state machine, exercise viewport create/delete/recreate under LVGL, then cooperative tile reading with bus ownership and cancellation. Remote package 028 / Boat 014 unchanged.


## Cooperative SD reads and map-copy regression continuation
- 546b550 / 836ab34 / a648892 implement a single cooperative tile reader, 4096-byte reads per renderer update, cancellation on obsolete viewport/zoom/style/profile or catalogue reset, no publication of partial buffers. Actual SD latency and LVGL lifecycle remain unverified; synchronous compatibility API retained.
- 4e33cd0 / 50c5df1 / 4c1b4f2 extract actual map-copy logic into map_copy.h/.cpp and connect Map Manager; 6468090 adds success, short-write, bounds, truncate and cancellation tests; 7580110 registers eleventh host group. Results pending; push CI lookup returned no runs through PR-only connector. #50 remains last verified target, not evidence for new source.
- Reviewed physical scanner and mission interfaces: independent actuator requests remain guarded, nonmanual mode explicitly refused, portable mission assembler/lifecycle not yet wired to authenticated dispatch. Next: verify push target/11 host groups, address diagnostics, then complete physical input integration and authenticated non-actuating mission preparation/feedback before enabling real execution. No package marker bump or ZIP sync.


## Physical input safety and mission sender continuation
- c53b95d / 18bf1bd: manual motor control requires fresh neutral rearming after invalid inputs, safety/link inhibition or failed send, confirmed manual mode and actuator readiness. Temporary autopilot override stays disabled until acknowledged Boat-side transitions exist. 1027027 / 62d9972 inhibit manual releases in nonmanual/failsafe states at both guard and command boundary.
- a165c5f / 1c63879: held-key startup requires STOP and all inputs released, with mocked startup regression; 6f8ce63 adds release inhibition tests. Joystick transport/runtime tests and actual hardware remain outstanding.
- 0811811 / 2292f18 add portable mission upload sender with exact transaction/opcode ACK matching, bounded retry/timeout, safety inhibition and fake receiver integration tests. Not attached to live radio or mission UI; no autonomous movement enabled. Next: verify latest target and all 11 CI groups, then define matching authenticated wire message/ACK dispatch on both peers, enforce session/replay and stopped-state gates, connect mission UI. No packaging bump or ZIP sync; last verified target #50. Push CI lookup remains limited to PR runs.


## Mission preparation transport and navigation continuation — 2026-11-20
- Remote/Boat mirrored `mission_wire.h` and `radio_protocol.h` define dedicated mission PREPARE (8) and ACK (9) messages keeping 32-byte authenticated frames. Boat `boat_dual_radio_port.*` dispatches only primary-link session/replay-checked preparation to a non-actuating assembler with application-owned stationary/STOP gate defaulting to null/deny; Remote `boat_radio.*` drives the upload and accepts matching authenticated ACKs. Both key stores still intentionally deny all TX. Existing 8-bit session epoch is not adequate final replay protection; no secure pairing or hardware certification.
- `mission_ui.cpp` now has preparation-only PLAN action with independent per-upload four-channel selection (all off by default; legacy marks are not mapped), preparation feedback, confirmation, local cancel and edit guards. Prepared plans expire independently; STOP invalidates local/Boat preparation. No mission START or actuator execution enabled. Relevant commits 763221c, 9cb3628, 68cb549, feb84db, f4724be, 6a1ded4.
- Verified locally using strict g++ flags and exact Git blob SHA matches: Remote mission protocol/upload/lifecycle/ACK regression PASS (test 4cdcc547669b7172bf52f7682fd3ede571bcff9a); Release guard PASS (test a2a6662931d0330bc84d1db0402a389d60b994f6); Physical scanner test initially failed at restart OK count because earlier ordinary OK was counted, fixed test baseline f62926985e903a64d8074e370918431804992969 and reran successfully (test blob 712e24efa6615ca190f0fc9472e2ff278f051b77). These are portable tests, not full CI or ESP32 tests.
- Boat `mission_navigation.h` adds pure distance/bearing/heading error and conservative arrival criterion distance + estimated horizontal accuracy <= requested radius (3 m default), requires fresh fix/healthy heading/battery/geofence. It is not a steering controller. Host test registered in Boat CI (6b8b4a3); local C++11 strict run PASSED on exact blobs a260353a7afbdff84e2a9f94ea10024399a45b73 (navigation) and f2004291e2d4119fe244ea24f92b1b1c63b97215 (test), mirrored codec blobs d0077f6727f9ec975d5c2fe95927bab4879999a4 and 651ea8faf239fd41e19f3f3b09668b94be2aae4f.
- Next: retrieve latest push-triggered target compile and full Remote/Boat host CI results, fix diagnostics before more target integration. Local `git ls-remote` fails DNS and Arduino CLI unavailable. No fresh full target evidence (last verified #50); remaining map-copy/loader and LVGL lifecycle tests, authenticated session redesign, Boat application supervisor, mission execution feedback and actual navigation/actuation remain open. Remote 028 / Boat 014 unchanged. No archive regeneration verified. No user design decision required for this software work; build evidence is the immediate blocker.


## Workflow audit and development continuation — 2026-11-20
- Repository-wide Actions run listing now accessible through authenticated GET. Target #74 (44801485451) failed at missing lv_image_cache_drop declaration; commit 0f4c47b added upstream 9.6 header but #75 (44802976345) failed due to Arduino include root already being lvgl/src. Commit 692f959 fixes to <misc/cache/instance/lv_image_cache.h>; #76 (44803266207) running at time of audit. Never treat previous #50 as current.
- Remote host #90 (44803265844, job 136520557058) passed all 12 groups including production SD tile reader/cancellation/short-read/missing-card tests and mission, release, map copy/recovery. Boat host #26 (44803347880, job 136520835967) passed all 8 including new simulation-only execution supervisor. Boat #29 (44803586651, job 136521642320) passed all 9 including actual OpenSSL HMAC domain-separated session transcript and bounded replay counter tests. No physical actuator integration. Session V2 candidate lives in Boat/source/radio_session_v2.h, not integrated; key store remains fail-closed.
- Corrected packaging workflow 74700d2: event SHA checkout, per-side exact +1 checks, immutable existing ZIPs, no rebase onto newer branch; #1044 (44803056623) passed normal snapshot build, and #1048 (44803265820) succeeded with SHA-specific artifacts. Release-publication path not exercised; no bump 028/014. Archive contents/inner checksums not independently downloaded.
- Added separate Bulgarian UI resource generation workflow and script using DejaVu Sans and pinned lv_font_conv 1.5.3. Generated resource not yet linked to LVGL or keyboard and workflow result unverified. Existing default Montserrat font does not cover Cyrillic. Do not claim screen validation.
- Next: confirm #76, inspect new font workflow, integrate tested session handshake and font into both target applications; complete real Boat supervisor/feedback, joystick/UI runtime, SD performance, release verification. Hardware, pinout, secure physical key provisioning and real CHIRP transducer remain external gates. Do not enable actual motor/solenoid execution or bump versions until acceptance criteria are met.


### Target #76 acceptance evidence (supersedes running status above)
- Run 44803266207, job 136520558870, source 692f959fcd76430d21e79c75ddeb183d8be8a8b4: PASSED. Program 1,441,429/3,145,728 bytes; globals 178,968/327,680 bytes; 148,712 bytes remaining. No C++ errors in retrieved logs; toolchain esptool SyntaxWarning and Node deprecation notices remain. Firmware artifact 14662810189 and compiler log artifact 14662560853 uploaded successfully. Remote host #90 and Boat #29 passed 12 and 9 groups respectively; Boat-only additions after source revision do not constitute a new Remote target build. This is compilation evidence only, not hardware certification. Version markers remain 028/014. Next integration gate: session V2 live transport/provisioning, actual Bulgarian font/keyboard integration, execution supervisor wiring and physical acceptance; archive publication remains gated.


### Additional verified workflow results
- Bulgarian font resource #1 (44803930186, job 136522841000): generation and source artifact upload PASSED; not linked or rendered on target. Offline Map Tool Tests #3 (44803930041) PASSED. ZIP snapshot #1053 (44803930029, source bc1ea6fff22e36b60a3e40870864762c0f65982f) PASSED archive creation, integrity tests and upload; artifacts Remote 14662426210, Boat 14662431183, checksum 14662371258. Artifact digests describe outer Actions ZIPs, not inner archive hashes. No release marker bump; immutable publication gate remains untested. Next: wire secure handshake into both endpoints and integrate generated Bulgarian font; verify actual execution supervisor and hardware gates before releasing 029/015.


## Session V2, Bulgarian UI and execution-control continuation — 2026-11-20

- 9ca1af5 integrates generated DejaVu Cyrillic font into the LVGL default theme in target CI, with Montserrat icon fallback, font artifact/license and EN/BG lower/BG upper keyboard layout. Local Arduino build requires generating/copying the font resource per Remote/docs/BULGARIAN_FONT_BUILD_R1.md. Resource workflow #2 (44805083427) and offline map tools #4 (44805083408) passed. Actual rendering/keyboard usability not hardware-tested.
- f249e76 implements mirrored mutual HELLO/CHALLENGE/PROOF/CONFIRM handshake with real OpenSSL HMAC host tests, replay/refused authentication and duplicate-proof confirmation checks. 67e2643 connects both actual adapters to derived session keys, rejects legacy handshake, uses bounded nonwrapping replay counters, resets mission/queue/ACK/telemetry at session loss or rekey, and stops queuing obsolete LoRa joystick commands. 8880fd2 authenticates the physical link flag, rejects copied frames on the wrong link, and adds flag regressions. Primary nRF24 is required to establish new sessions; backup shares an established session only. Secure provisioning reader is guarded by explicit opt-in, encrypted NVS, flash encryption and secure boot; normal target builds fail closed, the opt-in secure SDK path is untested, and no actual key or hardware entropy validation has occurred. See docs/SECURE_PAIR_PROVISIONING_R1.md.
- 0c31980 adds actual non-actuating Boat diagnostic sketch polling the radio and a separate Boat ESP32-S3 target build. Unassigned pins, no mission safety provider or actuator execution; queue acknowledgements are not execution. First Boat target #1 (44805621575) failed missing ManagedLink declaration; f9c6bde corrects it. Remote target #79 (44805520905) failed on LVGL keyboard control enum conversions; 8880fd2 corrects explicit casts. Superseded intermediate builds cancelled. Current Remote target #81 (44806126603) and Boat target #4 (44806126280) are running on 8880fd25b8df9fd84cd3549bc8f8f38d516df1b1; no target pass claimed for new changes yet. Last verified Remote target remains #76.
- c2b565d adds mirrored 8-byte mission-control and feedback codec plus portable BoatMissionController (stage, start, pause, resume, cancel, temporary override neutral confirmation, permanent manual request, duplicate request handling, expiry). Tests are simulation only; no authenticated radio dispatch, physical mode confirmation, manual override motor lease or release safety executor yet. Future live integration must distinguish prepared plan from execution, invalidate on STOP, require current trusted Boat inputs and hardware watchdog, and never infer acknowledged movement from a queued operation.
- Verified latest host CI on 8880fd2: Remote #95 (44806126364) passed 12 groups; Boat #35 (44806126309, job 136530479362) passed 11 groups including actual HMAC, handshake and mission control/override simulation. Packaging #1062 (44806126387) passed snapshot build/upload; archive publication not yet tested, inner hashes not independently checked. Remote 028 / Boat 014 retained; no release bump.
- Continuation: verify both target builds #81/#4 and fix any failures; after compile acceptance wire mission-control request/feedback to authenticated primary radio and UI with a default-deny Boat application provider, add trusted input/STOP and freshness checks and temporary override command lease across actual joystick/Boat executor, then package frozen 029/015 only after acceptance. Hardware and CHIRP remain pending. No physical provisioning or actuator enablement approved or performed.


## QWERTY Bulgarian UI, mission execution radio integration and override lease — 2026-11-20
- 5b638ac and 7cadc20 implement touch-adapted phonetic QWERTY Bulgarian keyboard shared by Settings and lake name editing; lower/upper rows cover all 30 letters, English and symbol/number modes retained. Offline tool tests #5 (44807388840, job 136534858451) passed all 8 including explicit keyboard row and glyph coverage assertions. Actual physical LVGL keyboard usability remains unverified; this is not a BDS compliance claim.
- 215a239 and 8c2377b implement authenticated primary-link mission control, periodic phase feedback and dedicated stick input messages, Remote request/ACK client (bounded retries retaining logical ID; missing ACK unknown), a receiver-local 200 ms override lease (nominal 50 ms Remote samples), neutral rearm before override and resume request after displacement then neutral; physical stationary/STOP and release interlocks still required on Boat. No actual motor output is wired. 7cadc20 adds mission overlay for START/PAUSE/RESUME/CANCEL/temporary override/manual/STOP/clear STOP and guards lake/waypoint changes. d229cb1 requires fresh matching primary telemetry before confirmed manual transition and retains local STOP until fresh confirmation; adds completion token checking and queue clearing at mission start. b1f4926 fixes successful upload handoff and duplicate request side effects/ACK after STOP. 1f0d585 stages Remote client header into canonical shared-wire test directory; cross-peer integration simulates request, lost ACK, override neutral, 200ms expiry and abort. No actual RF or actuator tests; no trusted mission input provider in diagnostic Boat sketch. The live session-reset return-home behavior remains an explicit unresolved requirement: session timeout/rekey currently aborts the supervisor, so prolonged link loss does not yet guarantee autonomous return. Do not enable execution before watchdog, sensor, propulsion, release, session-loss-return and mode telemetry integration are independently verified.
- Previous Remote target #81 (44806126603) cancelled by source update, not a pass; Boat #4 (44806126280) passed 320945/1310720 program bytes, 19968/327680 globals. Boat #5 (44806845960) passed; Boat #6 (44807388836, job 136534858206, d229cb1) passed 333569/1310720 program bytes, 20072/327680 globals. Latest Remote target #87 (44807620544) and Boat #7 (44807620473), both source b1f49260ab29f266d8e0e925209e253465cad162, remain in progress at checkpoint. #76 remains last verified Remote target. Superseded Remote builds cancelled by subsequent firmware changes. No newly observed compile errors in this batch.
- Remote host #101 (44807620465, job 136535625521) passed 13 test groups; Boat host #39 (44807676162, job 136535821192) passed 13 (including the revised cross-peer simulation and stick lease). #39 differs from firmware revision by test/workflow-only commit 1f0d585. ZIP snapshot #1069 (44807620683, source b1f4926) passed creation and upload: Remote artifact 14664446356, Boat 14664526045, checksum 14664511165. These are development snapshots; inner archive contents/checksums not independently downloaded. Remote 028/Boat 014 unchanged; no immutable 029/015 publication until acceptance. 
- Next: verify #87/#7 target completion and diagnostics, reconcile later builds, test secure pairing and real application provider/actuator watchdog with disconnected power, complete autonomous return on prolonged link loss and confirmed manual/mission status paths, verify LVGL/SD real runtime. Release acceptance not complete; no production key provisioning or hardware actuation approved or performed.

### Boat target #7 verification (supersedes running status above)
Boat diagnostic target #7 (44807620473, job 136535626232, firmware source b1f49260ab29f266d8e0e925209e253465cad162) PASSED; program 333597/1310720 bytes, globals 20072/327680 bytes, leaving 307608. Remote #87 still in progress at last check. Release version markers remain unchanged. No actuator hardware or live RF validation claimed.
