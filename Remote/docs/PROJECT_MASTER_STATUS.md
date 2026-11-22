# Flytec V080 Retrofit — Master Status R10

> **R79 ACTIVE STATUS (2026-11-22):** Remote endpoint remains Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6, dual nRF24/E22 radios, 1S4P battery and SD-backed maps/waypoints. Boat endpoint and battery ADC are specified in [Boat Hardware Selection R20](../../Boat/docs/BOAT_HARDWARE_SELECTION_R20.md). R79 adds the Boat's green/yellow/red battery voltage/estimated-charge indicator, recharge/RTH warning and stale-data fail-safe mission-preparation block. Older Heltec/PCB descriptions below are historical. Hardware and display/radio acceptance remain pending.


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


## Autonomous return-home and local hardware safety bridge — 2026-11-20

- Remote target #87 (44807620544, job 136535790034, Remote firmware source b1f49260ab29f266d8e0e925209e253465cad162) PASSED, superseding #76. Program 1,457,365/3,145,728 bytes, globals 179,464/327,680 bytes, remaining 148,216. Later commits in this batch change Boat source only. No new Remote compiler errors observed. Remote host #101 passed 13 groups; map/keyboard tool #5 passed 8.
- 2452159 implements one-way local diversion to Home of an already-started mission, clears releases and keeps Home navigation authorization across session expiry/rekey without retaining the old keys, transport queues or stick leases. Old mission transaction stays local during detached return; new-session mission/control/stick traffic and ordinary actuator requests are rejected, authenticated/physical STOP takes precedence. Never starts a prepared-only mission. Invalid navigation/battery/geofence, physical STOP or an unconfirmed release pulse prevent continued movement. Full dual-link loss diverts, rekey during active mission also conservatively diverts to Home; no automatic resume of outbound. The return controller test is a software simulation of session expiry policy, not a live transport/driver test. Actual obstacle avoidance and path/geofence sensing remain hardware/algorithm acceptance items; a valid GPS fix alone cannot guarantee a safe water route.
- b239438 implements portable locally timestamped trusted sample validation, normalized differential navigation mixer candidate, 200 ms manual propulsion lease, one-at-a-time tokenized release with explicitly commissioned pulse durations. 21e0ee7 integrates the application bridge and a driver contract that requires independent electrical cutoff/watchdog; no physical driver registered in the default diagnostic sketch. Physical 100 ms output lease is a contract, not measured hardware behavior. Sample limits 100 ms safety, 2 s GNSS and 300 ms heading, and limits in tests are provisional engineering/fixture values, not commissioned coil parameters.
- 9627f72 adds actual-output feedback validity gates, requires a new local sample after deenergization before release completion, confirms manual transition only with stationary/idle feedback and emits authenticated measured status (including detached return failsafe) through the application bridge. Ordinary manual-release execution, full GNSS/heading/position telemetry generation and final board-specific input/output drivers are not yet implemented. No production key provisioning, eFuse changes or actual actuation performed.
- Boat host #40 (44808224304) passed 14 groups with new link-loss tests; #42 (44808489252, source 21e0ee7) passed 15 including watchdog/release tests. Boat target #10 (44808489093, job 136538752483, source 21e0ee7) PASSED: program 336,465/1,310,720 bytes; globals 20,216/327,680, remaining 307,464. Latest source 9627f726a9327b6e2a5ab0e13ecafc3617126766 triggers Boat target #11 (44808772228), host #43 (44808772480), ZIP #1076 (44808772344); pending at time of checkpoint. Last verified snapshot #1075 (44808489113) generated both development ZIPs. No version marker bump (028/014); immutable 029/015 publication not authorized by current acceptance gates.
- Acceptance remains incomplete: actual board design pinout/driver mapping, independent watchdog/STOP electrical verification, calibrated GNSS/compass/stationary detection, motor direction/current and all four coil pulse/idle measurements; actual secure pairing with keys and entropy on selected devices; Waveshare keyboard/touch/live SD timing, complete live telemetry and water trials. See Boat/docs/ACTUATOR_DRIVER_ACCEPTANCE_R1.md and docs/SECURE_PAIR_PROVISIONING_R1.md. No physical evidence available. Next: verify latest Boat target/host/package results and fix errors, complete manual release and live telemetry application paths, test bridge with a mock driver, then obtain exact physical hardware and perform disconnected-power/bench validation; keep releases gated until acceptance.

### Final host and archive evidence
Boat host #43 (44808772480, job 136539600288, firmware source 9627f726a9327b6e2a5ab0e13ecafc3617126766) PASSED all 16 groups including measured status confirmation and strictly newer post-pulse sample. ZIP #1076 (44808772344, job 136539600263, same source) PASSED snapshot creation and integrity/upload checks. Boat target #11 still running; last verified Boat build #10. Archive markers remain 028/014, and inner archive contents not independently downloaded. No hardware validation.


## Manual release, sensor telemetry and application integration — 2026-11-20

- Boat target #11 (44808772228, job 136539599468, source 9627f726a9327b6e2a5ab0e13ecafc3617126766) PASSED; program 338,069/1,310,720 bytes; globals 20,224/327,680 bytes; 307,456 remaining. No new compiler errors. Remote target #87 remains last verified Remote target (Remote source unchanged in this batch).
- 38fb447 integrates all four independent manual releases into the existing shared one-at-a-time pulse supervisor, checks stationary/idle/interlocks and primary authenticated link, prevents motor output during a pulse, sends terminal completion ACK only after post-pulse local idle confirmation, separates transport generation from cached actions, and rejects non-new manual release IDs after queue acceptance. A repeat completed ID may get a duplicate ACK without another pulse. Physical mechanical release is not proven by coil idle. No actual physical driver registered. dd66dda adds neutral rearming after release and STOP on a new session unless a detached return remains active, strengthens GNSS status validity gates.
- 38fb447 adds locally timestamped coherent battery/heading/speed and matching lat/lon pair telemetry from trusted driver samples, periodic status at 200 ms and sensor batch at 500 ms. Invalid/stale fields use unknown sentinels or omit coordinate pairs; no synthetic GPS coordinates are transmitted by diagnostic firmware. Actual GNSS/compass/battery board-specific acquisition remains unimplemented/unverified. Waypoint index currently unknown (255) in sensor batch. Real RF performance and remote status rendering require physical tests.
- Boat host #44 (44810506477) passed 17 groups; #45 (44810626275, job 136545934705, source dd66dda3ddc887c07da8863bb73ef38288c90c94) PASSED 18 groups, including new telemetry codec round trips and actual production application bridge compiled against a mocked radio and physical driver. Test covers all four releases, concurrent request denial, backup denial, old idle sample rejection, post-release neutral rearming, manual lease expiry, loss of primary during pulse, session rotation, telemetry and failed reads. It does not test live RF dispatch, electrical cutoff, actual mechanisms or water.
- Boat target #12 (44810506870) cancelled by later source update, not a failure/pass. Target #13 (44810626250) still running at checkpoint. ZIP #1080 (44810626184, job 136545935490) PASSED exact-source snapshot creation, unzip integrity and artifact upload; immutable publication skipped, no version bump. Inner archives not independently downloaded.
- Added docs/PHYSICAL_COMMISSIONING_R1.md, staged 18 physical acceptance checks and evidence form. All physical results NOT RUN. Existing release fixture timings and steering gains are NOT real coil/driver settings. Separate electrical watchdog and exact board driver implementation remain mandatory; pairing and touchscreen/SD hardware test, real geofence/water trial and CHIRP integration remain open.
- Next: verify Boat #13, fix errors if any; audit remaining actual hardware adapters and real release ACK timeout against measured coil pulse lengths; complete board-specific integration and physical acceptance with locally measured evidence. Do not claim operating readiness or publish official VER029/VER015 before acceptance. Current marker 028/014 retained.

### Final target verification
Boat target #13 (44810626250, job 136545998180, source dd66dda3ddc887c07da8863bb73ef38288c90c94) PASSED; program 339,993/1,310,720 bytes, global RAM 20,280/327,680 bytes, remaining 307,400. Firmware and compile log uploaded successfully. Remote firmware source unchanged since successful target #87. Both target builds and 18 Boat / 13 Remote host groups are verified for the current software revisions; physical commissioning remains NOT RUN, official releases remain 028/014. No physical readiness claim.


## Board-specific integration R4 — 2026-11-20
- Manufacturer public revision documentation reviewed: V1.2-specific authoritative schematic not located; V1.1 and V2 are not safe substitutes. Added conditional GPIO crosswalk and provisional companion I/O candidate with RP2040 GP assignments, sensor/actuator logical connector schedule and independent electrical watchdog design calculations in Boat/docs/BOAT_BOARD_INTEGRATION_R4.md and BOAT_HARDWARE_PIN_CONNECTIONS_R4.csv. No final pin freeze or actual wiring. New ADS1115 and companion controller are proposals; exact module and power cutoff require selection/engineering. TPS3431 150pF candidate timing 66.61ms nominal, approximate 59.8–73.5ms bounds before total cutoff; requires measured <=100ms including gate/feedback. Four separate coil connections documented, pulse/current/fuse ratings pending. No KiCad schematic or ERC completed in this batch.
- e640800 adds bounded local UC6580 checksummed GGA/GSA/RMC/GST parser requiring coherent UTC epoch and actual accuracy estimate, calibrated tilt-compensated heading math and ADC pack conversion. No HDOP fabricated as metres; GST output must be verified/enabled and estimated uncertainty commissioned. 1ebd347 adds actual Arduino UART/I2C register acquisition for UC6580, MPU9250/AK8963 and proposed ADS1115, and normalized reversible motor PWM/one-hot output planner. ca79245 strengthens coil channel-change/fault guards and adds optional read-only locally audited diagnostic sketch path. Default remains no GPIO/sensor or actuator activation. Heading/battery calibration initially disabled, no state-of-charge or safety flag fabrication. No companion firmware or high-current output GPIO adapter yet; no physical testing. Boat host #46 (44811977065) and #47 (44812118555) passed 19 and 20 groups respectively; latest #48 (44812326454), target #16 (44812326290), ZIP #1085 (44812329736) pending at this checkpoint. Last verified Boat #13, Remote #87 unchanged. Version markers Remote 028 / Boat 014 unchanged. Next verify #48/#16/#1085; resolve actual Tracker revision and I/O topology before native schematic, final hardware driver mapping, watchdog gate and current/feedback design, then physical acceptance.

### R4 final CI evidence
Boat host #48 (44812326454, job 136551764834, source ca79245cb6800637192514d3728826449b049593) PASSED all 20 groups. Boat target #16 (44812326290, job 136551916706, same source) PASSED: program 357,917/1,310,720 bytes, globals 20,600/327,680, leaving 307,080. Default diagnostic configuration compiled; locally commissioned optional sketch configuration and actual sensor I/O untested. ZIP #1085 (44812329736, job 136551778130) PASSED snapshot build, integrity checks and upload; archive publication skipped and inner archive contents not independently inspected. Remote #87 and Remote host #101 remain valid for unchanged Remote firmware. Physical tests NOT RUN; versions 028/014 retained. Next resolve V1.2 exact hardware documentation and candidate companion-I/O design approval, then capture native schematic/verify power budget and implement companion peripheral protocol/driver and compile-time signed pin profile.


## Owner design decision — Heltec-only MDD20A restoration (R5)
The owner explicitly rejected the RP2040 companion and restored the original Heltec Tracker + Cytron MDD20A direct PWM/DIR design. This supersedes the R4 companion recommendation and all companion development commits. Removed companion Pico SDK source, wired protocol/master/safety core, companion-only tests and Actions workflow from the active branch; Git history retains them. Kept Tracker GNSS/IMU/ADC acquisition, output planner, existing radio/session/mission/release/telemetry and independent electrical watchdog/STOP requirements. Added a fail-closed direct GPIO profile validator and regression; diagnostic sketch remains unassigned. New active R5 design and pin schedule supersede R4. No MCU pins, expander part, high-current cutoff, or PCB frozen: authoritative V1.2 pin map and exact GPIO budget unresolved. Four independent solenoids retained. Remote source unchanged; release version markers Remote028/Boat014 retained. Re-run Boat target, 21 host groups and ZIP packaging; record actual results. All physical acceptance gates remain open.

### R5 rollback verification
Commit a22886a7c0c59694a5a8c2d7bff3a519d1e4a047 updates 28 paths atomically. Recursive tree audit confirmed no companion-specific source, master link, safety core or workflow files remain. Prior abandoned Boat target #19 (44813593533) failed in carrier_link/carrier_master_port; those obsolete files have been removed rather than repaired for a rejected architecture. Boat host #52 (44814101535, job 136557839658) PASSED all 21 active groups, including direct GPIO profile regression. ZIP snapshot #1091 (44814101036) PASSED; official markers unchanged, inner archives not independently inspected. Boat target #20 (44814101124) still running at verification; last successful Boat baseline #16. No new compilation pass claimed.
Active R5 BOM added; old schematic/BOM references explicitly marked historical for corrected four-channel topology. Remaining: confirm exact Tracker revision, audit native GPIO count and optional non-programmable I/O expansion, capture direct MDD20A driver and independent watchdog/power-gate native schematic, implement physical driver only after pin freeze, then commission with disconnected actuator power. Remote untouched. No physical or ERC acceptance.


## R6 continuation — direct MDD20A HAL, pin budget, sonar deferred (2026-11-21)
- Boat target #20 (run 44814101124, job 136557839493, prior approved R5 source) PASSED. Program 358,009/1,310,720 bytes, global RAM 20,600/327,680 bytes (307,080 remaining); tooling emitted only external esptool/Node warnings.
- 3fa3799 adds `mdd20a_arduino_stage.*` (Arduino-ESP32 3.x LEDC 20 kHz/10 bit, two channels, direct PWM/DIR, initial PWM/permit/WDI LOW), and `mdd20a_direct_safety.h` with local measured feedback freshness and latched failure; **driver is NOT instantiated or attached to active Boat firmware, no GPIO selected and no power output authorized**. 3d1a755 makes motor→coil interlock enforce the existing shared output planner; d70028f corrects a test expectation at exactly the 30 ms post-release deadtime boundary. Full target and host verification of these new changes must be checked separately before asserting pass.
- Added R6 direct GPIO budget and electrical capture checklist: independent TPS3431/safety cutoff, Heltec native MDD PWM/DIR/WDI/permit, optional non-programmable I/O expansion only for coil/feedback, and strict signed pin map gate. Exact V1.2 PCB/schematic not supplied, final GPIO allocation blocked; current work is an interface skeleton, **not native KiCad schematic or ERC**. Power, motor and coil values remain commissioning candidates only.
- **Sonar is intentionally only planned** in Boat/docs/BOAT_SONAR_INTEGRATION_PLAN_UNDECIDED_R1.md. Owner has not selected a model. No sonar code, new display, GPIO, transducer wiring or chosen transport. One existing 4.3-inch 800×480 Waveshare screen, real CHIRP-like fish arch echogram, manual/autopilot, 50/50 map/sonar view and optional full-screen mode remain requirements; actual data protocol, electrical budget and high-throughput link depend on selection.
- Prior Remote #87 and unchanged Remote software remain verified; official markers Remote VER028 / Boat VER014 unchanged, no physical acceptance or release authorization. Next: check latest target and host CI, correct errors, freeze the exact Heltec V1.2 pin map and safety hardware/feedback, and capture a native MDD20A/power/coil KiCad design after verified choices. No physical actuation before bench proof.


## R6 verification and frozen sonar boundary — 2026-11-21

- **Boat ESP32-S3 target #22 PASSED**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865488754 (job 136728159017, source 3d1a75530f7e27dc0132de36a93de999f7ed0a8c). Program 358,121/1,310,720 bytes; globals 20,600/327,680 (307,080 remaining). Arduino core 3.0.2 source with direct MDD20A LEDC output HAL compiles; target #21 was superseded. No physical GPIO configured.
- **Boat portable host tests #55 PASSED all 22 groups**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865543808 (job 136728262617, source d70028f22e73d52b967bb2cb35057d443c2e2cc6). First intermediate test runs #53/#54 failed due incorrect motor/coil state and deadtime test expectations. The source and test were fixed in 3d1a755/d70028f; the final source passes regression. These were software test failures, not evidence of an actuated device.
- ZIP packaging #1096 PASSED on documentation commit 465caea4696c928100734f99919af1f2f0075f31: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865619611. Boat snapshot artifact 14691667268, Remote snapshot 14691388678, digest manifest artifact 14691647257. Inner ZIP payload and hashes have not been independently inspected outside CI. No official release.
- Confirmed Heltec-only architecture. Direct MDD20A stage remains **uninstantiated** in the Boat diagnostic sketch and gate/STOP/WDI feedback remains uncommissioned. It does not energize the actuator permit or auto-pet the independent watchdog. **Do not claim motor-driver physical integration or circuit safety acceptance.**
- R6 sonar planning is recorded **without selecting any sonar model or implementing sonar**. The one-Waveshare-screen full echogram and fish arch requirements remain, with sonar power/data connector, transport, parser and transducer unassigned. Dedicated model selection gate retained. No code, GUI, firmware, GPIO pin or hardware circuit was added for sonar.
- Unresolved engineering decisions before native KiCad/Electrical Rule Check (ERC): exact Heltec V1.2 authoritative schematic/pin audit; six truly free direct output-capable motor/permit/watchdog pins while preserving GNSS+nRF+E22+I²C; whether a non-programmable I/O expander is necessary for four solenoids plus physical feedback; actual independent watchdog cutoff, coil currents and fuse ratings. **No board purchase, physical acceptance or official release.** Remote #87 and versions Remote 028 / Boat 014 remain unchanged; planned 029/015 gated on hardware acceptance.


## Next-step engineering R7 — direct MDD20A feasibility and KiCad draft (2026-11-21)

- Current Heltec-only MDD20A design preserved. Added V1.2 conditional GPIO feasibility program and matrix. Native minimum for nRF primary + external E22 UART + I²C + MDD20A PWM/DIR + WDI + permit is 12 pins; optimistic unverified V1-family candidate pool 11. A GPIO expander for coils alone cannot resolve the minimum deficit. No actual Heltec GPIO pins allocated. Non-programmable I²C-UART bridge or expander for DIR are contingency studies, not approved selections. Source documentation: Boat/docs/BOAT_MDD20A_ELECTRICAL_GATE_R7.md.
- Captured editable **preliminary native KiCad** MDD20A control/watchdog sheet with custom embedded connector and TPS3431 symbols; first KiCad 9 netlist export succeeded. **First ERC reported 83 violations**, not an electrically approved/fabricable design. High-current switch/gate, 4 solenoid MOSFETs and final rails are not captured. Dedicated KiCad validation workflow and report artifact created; ERC must be reviewed and corrected before board release.
- Added measured-only pack current budget and hardware fault-latch requirements with software verification. No invented motor stall, coil current or fuse selection. Physical watchdog signal must latch actuator power OFF independent of MCU; 150 pF CWD is a candidate (66.61ms nominal, approximate 59.8–73.6ms bounds excluding switch/current decay). Every physical input/driver still default-denies.
- Sonar plan remains **unchanged and model undecided**. No sounder, firmware, UI, connector pin/power or code has been implemented this turn. Existing 4.3-inch screen and future full CHIRP-style fish arches remain requirements.
- No release increment or physical acceptance. Remote #87 and Boat #22 target compiler baselines remain last verified for unchanged application code. Host regressions and ZIP snapshots run in GitHub Actions; check the latest jobs for exact SHA before claiming results. Next: authorize a verified GPIO resource solution and complete KiCad/ERC, physical watchdog, power and four-coil circuits.


### R7 completion checkpoint — 2026-11-21

**Active architecture unchanged:** single Heltec Tracker + external Cytron MDD20A, nRF24 primary/E22 backup, four independent solenoid channels, no RP2040. Sonar model explicitly **undecided**, sonar code/wiring **not implemented**.

**Verified software and project checks:** Boat host #59 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714160) PASSED all existing portable regressions plus GPIO feasibility, pack-current safety and gate truth-table tests (24 groups by workflow stages). ZIP packaging #1103 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714149) PASSED for source SHA 358ed10687fbd09606aafbaf18a730cc0ef80e49. Boat target #22 remains last verified ESP32-S3 diagnostic source build; subsequent commits modify tests, docs, and preliminary CAD only. Remote target #87 remains last verified Remote build. Official Remote VER028 and Boat VER014 unchanged; 029/015 withheld.

**Native preliminary KiCad progress:** KiCad workflow #6 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866942940, source e2ec4f6c8cfc3852c78d1f475a7f57af1c7a6ed4) PASSED parser/netlist export and geometry/project-symbol checks after adding project-local `Flytec.kicad_sym`, `sym-lib-table` and `.kicad_pro`. Unresolved ERC violations reduced from 83 to 26 after grid alignment, then to **15** after project library setup (13 unimplemented release/feedback net connections and 2 unpowered TPS3431 power pins). **ERC production acceptance FAILS/NOT COMPLETE**. Remaining physical power gate, watchdog fault latch, high-current motor/coil circuits, fuse/protection, final footprints, true V1.2 pin assignment and PCB are not captured. A successful *preliminary* KiCad check must never be represented as zero-violation ERC.

**Verified GPIO shortfall:** provisional best-case pin pool 11, minimum native demands 12 with both external radios retained; even this assumes contested oscillator/LED/UART/flash pins are available. New active `Boat/tools/gpio_feasibility_r7.py` and matrix explicitly block unsafe assignments. I²C-UART bridge or direction expander remain optional proposals, neither selected or implemented. All physically driven GPIOs remain unassigned/disabled. No hardware or physical acceptance evidence; no sonar model or implementation. Next: decide a safe extra I/O topology after exact Heltec V1.2 pin documentation, fill unresolved nets/rails in native KiCad, verify a separately latched electrical cutoff, run zero-violation ERC and physical commissioning before releases.

## R15 — owner-mandated premade-module-only electronics (2026-11-21)
**SUPERSEDES all previous custom PCB/carrier fabrication plans on Boat and Remote.** New canonical policy: `docs/HARDWARE_MODULE_ONLY_POLICY_R15.md`. ACTIVE module plan: `Boat/docs/BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md` and `Remote/docs/REMOTE_MODULE_ONLY_IMPLEMENTATION_R15.md`, plus manufacturer-grounded master BOM `docs/MODULE_ONLY_BOM_R15.csv` and physically uncommissioned harness contracts in Boat/Remote docs. All R7–R13 KiCad drawings and Remote historical PCB design notes are archived only; no Gerber/PCB manufacture, not even a small safety board. 3D-printed plastic enclosure and mounts remain permitted. EKM014 four-channel driver preferred candidate (not purchased/accepted), MDD20A and RECOM safety regulator remain, ready-made Pololu rails and Adafruit MCP23017 + two ADS1115 proposed; **0x48** reserved for SC16IS750; proposal uses 0x49/0x4A joystick/diagnostics ADC boards. All critical independently rated premade STOP/power-cutoff, POR, pulse rearm, current-zero modules are **unselected/unsafe**, so physical actuator enable remains disabled. Boat onboard SX1262 vs Remote E22 compatibility remains unverified. Sonar unselected and not implemented. Remote VER028 / Boat VER014 held.

### R15 further work — premade safety candidates and bench acceptance
- New [safety candidate review](docs/MODULE_ONLY_SAFETY_SHORTLIST_R15.md): Duelco NST-3.2 **8–36VDC** emergency stop monitoring candidate **42042237**; Cebek I-33 **12V** non-retriggerable 0.1s–4min relay timing candidate but **not approved** because 0.1s min may exceed actual safe coil pulse and relay only **2A inductive**, not 6A coil. Electrokit EKM025 INA260 **±15A** / 0–36V/4 I2C addresses current telemetry candidate, **not independent cutoff**. High-current DC-rated contactor, physical onehot and rearm, true independent current-zero and brownout reset **UNSELECTED**.
- New [hardware acceptance checklist](docs/MODULE_ONLY_ACCEPTANCE_R15.md) prohibits using simulator or GPIO output as safety evidence, and [module pin/I2C map](docs/MODULE_ONLY_PIN_AND_BUS_MAP_R15.csv) preserves Remote SC16 0x48 vs ADS1115 0x49/0x4A and 16-key GPIO 0x20.
- Six legacy KiCad workflows now manual-only and marked ARCHIVED; R7–R13 are **historical references only**, not PCB fabrication targets. Module-only portable adapter is **host-only** (not compiled into active Boat target GPIO). No hardware, power rails or actuator load physically commissioned. Boat VER014 / Remote VER028 unchanged. Sonar planning only.

## R16 owner-confirmed four-channel module (2026-11-21)

**HARDWARE SELECTION FROZEN:** Electrokit **EKM014**, manufacturer/article **41024394**, is the final *model choice* for Boat LH/RH bait and LK/RK hook solenoid **low-side** switching. Exactly **ONE** four-channel preassembled module. It **replaces R9 custom-FET PCB permanently** in the active module-only project. This choice does **NOT** mean purchased, electrically verified, qualified, or safe to energize. No substitution to NCD or other coil driver without owner approval.

Read the dedicated [EKM014 R16 hardware and wiring contract](Boat/docs/BOAT_EKM014_SELECTED_R16.md), [R16 channel/wiring CSV](Boat/docs/BOAT_EKM014_PIN_WIRING_R16.csv) and [R16 commissioning test plan](Boat/docs/BOAT_EKM014_ACCEPTANCE_R16.md) (relative paths from repository root). R15 module-only policy remains mandatory for both sides. A dedicated *commercial* hardware one-hot and no-repeat maximum pulse path, independent DC power cutoff/STOP, rail/brownout supervision and independent current-zero verification are still required before connecting coils. Sonar still planned only; no RP2040/rudder/custom PCB. Remote VER028 / Boat VER014 remain held.

## R18 — distributed modular placement and Bulgarian-first verified electronics sourcing (2026-11-21)

**Owner’s new mechanical rule:** Previous 77×79mm carrier and 30mm clearance are **NOT active constraints**. Separately mounted modules may be distributed along the Boat hull in suitable protected positions; actual full hull balance/cooling/waterproofing still must be surveyed. **NO custom electronic PCBs**, factory modules and removable harness only. Active placement: `Boat/docs/BOAT_DISTRIBUTED_MODULE_INSTALLATION_R18.md`.

**Purchasing policy:** Official Bulgarian electronics suppliers/manufacturers (**Farnell Bulgaria, Comet Electronics, Olimex, Ardboard**) first. Verified specialized EU electronics shops/manufacturers may supply exact models where BG not confirmed (**Electrokit** for owner-chosen EKM014, **Lemona EU/Cytron** for MDD20A). **AliExpress is conditional only with verifiable authentic seller/product**; no listing currently approved. **OLX/eMAG/general marketplaces and anonymous copycat stores excluded.** Active audited evidence/register: `docs/APPROVED_ELECTRONICS_SUPPLIERS_R18.md`, `docs/VERIFIED_ELECTRONICS_SOURCING_R18.csv`.

**Verified exact product listings:** Farnell BG **RECOM R-78E3.3-0.5 #2218602**, no live Farnell stock proof; Comet RS #10923552 exact same RECOM part **0 in local warehouse, distributor order only**, not guaranteed stock; **Olimex DCDC6-16-TO5 6–16V→5V/2A in stock at Bulgarian manufacturer** (USB-A 5V output/barrel input, installation/actual Heltec-VL pin audit required), Ardboard **Heltec Tracker v1.2** exact product listing (stock unconfirmed). Exact EKM014 and MDD20A remain selected but no approved **Bulgarian** seller confirmed; known exact listings at reputable EU specialists, delivery to BG checkout must still be verified. Independent STOP/POR/no-repeat/current-zero modules remain unselected and no powered actuator commissioning is authorized. Sonar unselected. Remote VER028 / Boat VER014 unchanged.


## R62 — selected Waveshare Remote code review (2026-11-22)
Remote dual-radio manager now rejects invalid ManagedLink identifiers, checks authenticated link freshness before sending, and applies backup motor-command pacing only on successful physical sends; new regression `Remote/tests/dual_radio_manager_r62_test.cpp` covers fail-retry, stale backup, rate limit and primary recovery. Missing C-string include fixed after CI. Remote portable host tests **PASSED** (run 44956981643), and full Waveshare target compile **PASSED** (run 44956981581). No authenticated key provisioning, RF physical tests or on-device SD/LVGL validation. Backup LoRa short-rate steering cannot be treated as continuous motor control. Official Remote VER028 unchanged. See [R62 audit](../../docs/R62_ENDPOINT_CODE_AUDIT_AND_NEXT_GATES.md).

## R63 — LoRa backup control correction and Home request UI path (2026-11-22)
The 3 s LoRa airtime pacing cannot maintain the Boat's ~200 ms manual motor command lease. `Remote/source/dual_radio_manager.cpp` and `boat_radio.cpp` therefore limit the backup link to **STOP and Home request for an already-started mission**; they reject all steering, releases and autonomous mission-start commands. `boat_commands_setMode(MODE_RETURN_HOME)` can now submit the request for a locally active mission; final acceptance, navigation, STOP/interlocks and measured Home are checked on the Boat. The Remote host suite passed run 44991125196. Physical RF, 868 MHz legal airtime, acknowledgement roundtrip, full latest target compile and hardware tests are not established by these tests. See [R63 report](../../docs/R63_BACKUP_CONTROL_AND_RTH_INTEGRATION.md). Remote VER028 unchanged.


## R65 cross-endpoint hardware note — 2026-11-22

- Boat controller **ESP32-S3-DevKitC-1-N32R16V** preliminary pin budget now documented in `Boat/docs/BOAT_R65_SAFETY_AND_PIN_BUDGET.md` and its 44-header-pin CSV. Four Boat EKM014 requests need a **premade expansion interface into independent qualified hardware onehot/no-repeat gating** because Boat radio/motor/safety interfaces consume the candidate safe GPIOs; the Boat and Remote have independent I2C buses (both may use 0x20 on separate buses).
- No change to chosen Remote Waveshare 4.3 Standard, Remote-integrated SD lake/waypoint storage, primary nRF or E22 SMD-carrier block. Remote actuator commands remain guarded; backup LoRa STOP/active-mission Home only. No new purchased hardware or Remote firmware change. Release marker 028 unchanged; sonar deferred.


## R66 cross-endpoint status (2026-11-22)

- Boat independent preassembled safety controller capacity study and host-only reference tests: `Boat/docs/BOAT_R66_SAFETY_ARCHITECTURE_REVIEW.md`. A candidate Pilz B0 24V PLC cannot alone replace rated motor/coil power cutoffs and 24V outputs must never be directly wired to EKM014 3.3V logic. All safety hardware remains uncommissioned.
- Remote Waveshare model, primary nRF, matched E22 UART selection, SD lake/waypoint design, and fail-closed release command policy unchanged. Do not bypass Boat physical release guards for UI demonstrations. Remote VER028 unchanged; no sonar firmware.


## R68A cross-endpoint power note (2026-11-22)

The owner supplied Ardboard **article 1127** nRF24 AMS1117-3.3 socket for **Boat** radio-power investigation. This is **not** confirmation of a Remote power architecture: Remote uses 1S4P and needs its own verified battery-to-5V adapter supply if this socket model is ever chosen there. No design/purchase change to Remote radio rails, LoRa or SD storage. Boat nRF adapter proposal is downstream of verified 5V, while E22 remains on an independent proposed 3.3V Pololu rail; all hardware and actuator safety gates remain uncommissioned. Remote VER028 unchanged.


## R69 — matched E22-900T22S backup radio confirmed (2026-11-22)
The owner reconfirmed **1× EBYTE E22-900T22S UART SMD on Remote and 1× same exact model on Boat**. Do not use the proposed E22-900T30S. Remote I²C-to-UART SC16IS750 path and Boat direct UART remain tentative pending real breakout compatibility, hardware 3.3V supplies and RF tests. EU868 regulatory configuration and LoRa STOP/Home-only limitations unchanged; Remote VER028 not advanced.


## R78 — Remote target compilation completed (2026-11-22)

- [Remote Waveshare ESP32-S3 target compile](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45004179977) completed successfully on commit `7d32cf7a8862b57348e835fa7e569d93716d3afd`.
- The target job staged and compiled the complete Remote diagnostic/target sketch and uploaded firmware binary and compiler log artifacts.
- Remote portable host tests also passed at run 45004179963.
- R78 changes the Boat-side radio-loss policy and Boat-local Home NVS only; Remote source and Remote release version are unchanged.
- Boat/Remote ZIP snapshots were packaged successfully with existing VER014/VER028 markers in [run 45006416675](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45006416675).
- Physical display, touch, SD, nRF24 and E22 tests remain pending on purchased hardware.


## R79 — Boat battery indicator and mission preparation gate (2026-11-22)

- Remote target remains Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6.
- The main Boat telemetry header now displays Boat voltage plus estimated percentage, independent of the Remote's own battery.
- Green above 50%; yellow 21–50% with `СРЕДНА БАТ. — ЗАРЕДИ`; red at 20% or lower or Boat low-battery flag with `НИСКА БАТ. — RTH`. Missing/stale Boat data displays `ЛОДКА: --` in muted grey.
- Mission preparation is blocked when battery telemetry is stale or estimated charge is red. The Boat independently applies its own low-battery mission hold/RTH and critical STOP; Remote is not the safety authority.
- Remote host tests passed on the R79 code commit; Waveshare target compile is tracked in GitHub Actions. Physical LCD/touch/SD/radio and actual Boat telemetry acceptance remain pending.


### R79 critical-battery indication refinement (2026-11-22)

- Boat status bit 5 remains low-battery/RTH; bit 7 now independently marks critical-battery STOP.
- Remote battery indicator remains red for either state, but the warning text is distinct: `НИСКА БАТ. — RTH` versus `КРИТ. БАТ. — STOP`.
- Remote mission preparation rejects stale/red/critical Boat battery telemetry. Boat firmware remains the final safety authority.

### Red battery operator message refinement

The main display uses `НИСКА БАТ. — ЗАРЕДИ/СМЕНИ` when red but the Boat is not reporting Home return; `НИСКА БАТ. — RTH; ЗАРЕДИ/СМЕНИ` only when the Boat reports return-to-home; and `КРИТИЧНА БАТЕРИЯ — STOP!` when the Boat reports critical cutoff. This avoids promising that Home diversion has begun merely because the voltage is low.

## R27 Boat battery sensor update (2026-11-22)

Boat's selected battery-voltage module is now **Ardboard INA231 product 1491**, 3.3V I²C BUS-only; former ADS1115 divider is historical and INA226 remained a candidate. Boat runtime INA231 I²C not yet wired or commissioned, so **Remote must continue to show stale/unavailable Boat battery telemetry** and block mission preparation when no valid Boat reading is received. No Remote code, display thresholds, radio firmware, screen design, purchase quantity or version changed. Remote VER028 / Boat VER014 remain; physical test and independent Boat actuator STOP still pending.

## R28 Boat sensor software boundary (2026-11-22)

Boat's selected INA231 sensor now has a software-only triggered measurement state machine, I²C transport adapter, and integration into the Boat `TrackerSensorPort`, but `INA231_PHYSICAL_COMMISSIONED=false` and all local signoffs remain unset. The Boat must supply invalid/unavailable battery telemetry until physical signoff, so Remote's existing stale/missing Boat-battery warning and mission-preparation lock remain correct. No changes to Remote LVGL/SD/radio firmware, Remote physical casing or release version; **Remote VER028 / Boat VER014**.

## R30A cross-endpoint Boat handoff (2026-11-22)

The Boat has a [consolidated R30A design/assembly document](../../Boat/docs/BOAT_R30A_PROTOTYPE_ASSEMBLY_HANDOFF.md), [73-row power/signal netlist](../../Boat/docs/BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv) and [17-item Boat source integration inventory](../../Boat/docs/BOAT_R30A_SOFTWARE_INTEGRATION_MATRIX.csv). This changes **no** Remote MCU, joystick, button, SD, radio or UI source. Real Boat INA231 battery data remain uncommissioned/invalid and the Remote must keep its stale-battery warning and mission block. Both units still lack physical RF acceptance and pairing. Official Remote **VER028**, Boat **VER014** unchanged; sonar deferred.

## R30F — Boat safety fault diagnostic receiver and Bulgarian UI (2026-11-22)

Boat ↔ Remote now support identical, optional authenticated `RADIO_MSG_TELEMETRY` subtype 8 version 1 (`radio_safety_diagnostic_v1.h` byte-for-byte same). Its 14 fault reasons, `UNVERIFIED|COMMISSIONING_LOCK` bits, voltage sentinel and sequence are decoded separately from normal Boat STATUS, battery and mission authority. `Remote/source/boat_radio.cpp` admits this only after established HMAC/session and sequence checks on the **primary nRF** path. Crucially, a diagnostic-only frame **does not refresh `g_linkManager.reportValidRx`** or clear STOP. `Remote/source/boat_telemetry.cpp` stores separate read-only diagnostic fields (6000ms validity), invalidates them on link loss and session reset, and never changes actuatorSafetyReady or battery freshness from these fields. `Remote/source/boat_ui.cpp` displays `ДИАГН.: НЕПОТВ. / ...` or `ДИАГН.: НЯМА ДАННИ (НЕПОТВ.)` on a separate footer line while retaining STOP and battery alerts. See [R30F format and risk review](../../Boat/docs/BOAT_R30F_DIAGNOSTIC_WIRE_PROTOCOL.md). Remote host/target verification results to be observed before release. Remote VER028 frozen; physical radio/Boat inputs uncommissioned.

## R30F confirmed verification — 2026-11-22

Actual GitHub Actions results were inspected after each job completed:

- **[Boat host #45048750474](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048750474): SUCCESS.** R30F strict subtype-8 mirrored wire-vector tests, static Boat/Remote security contract and all other Boat portable host tests passed.
- **[Remote host #45048847021](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048847021): SUCCESS.** R30F paired codec golden vectors, invalid/forged-commissioned packet rejection and telemetry-isolation test passed, including a test that diagnosis cannot clear an already confirmed STOP. All other Remote portable host tests passed.
- **[Boat ESP32-S3 target #45048442529](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048442529): SUCCESS.** Diagnostic-only Boat target includes R30F optional authenticated send route, plus locked INA231 and historical locked ADS1115 benchmarks. This is a compilation test, **not** real RF transmission or actuator work.
- **[Remote Waveshare ESP32-S3 4.3-inch target #45048618202](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048618202): SUCCESS.** Complete Remote LVGL/SD/UI/radio target compiled, **1,460,005 / 3,145,728 program bytes (46%)**, **179,512 / 327,680 global dynamic RAM bytes (54%)**, 148,168 bytes reported available for local variables. This is compile output, not a measured runtime memory budget or physical board acceptance.

Intermediate early R30F CI runs failed because their respective commit snapshots invoked `verify_r30f_diagnostic_protocol.py` before that file and its linked R30F document were committed; both strict C++ wire codecs already passed in those runs. The later complete host runs above are **SUCCESS**. No command type or radio-frame authentication contract was changed.

**Status at R30F:** Boat and Remote software paths, strict codecs, host tests and ESP32-S3 builds are verified. Physically uninstalled and unverified STOP/watchdog, shared DC cutoff, independent one-hot coil limiter/current-zero sensors and actual RF range remain **outstanding**. Both endpoints retain all motor/coil STOP locks; R30A 24 physical gates OPEN and prior 81 physical tests NOT_RUN. Single shared 3S pack, no separately switched motor/coil domains, no new module selection. Boat VER014 / Remote VER028 unchanged.

**Next engineering step:** R30G — review rate limiting and bounded-queue/backpressure behavior of diagnostic packets during manual control and deliberate link loss, then regression-test no control-freshness or priority inversion. Physical radio acceptance and motor power remain blocked.

## R30G Boat-only optional diagnostic RF scheduling (2026-11-22)

The Boat R30F optional primary-nRF diagnostic now uses a [single-slot low-priority 500ms cache and heartbeat-first send policy](../../Boat/docs/BOAT_R30G_RADIO_DIAGNOSTIC_PRIORITY.md). **No Remote executable source, authenticated frame format, safety state, session, status decoder, joystick, screen, or telemetry thresholds changed in R30G.** Optional Boat diagnostic loss is expected; Remote already expires its display-only `ДИАГН.: НЕПОТВ.` information at 6000ms and never treats it as authenticated motor/STOP safety permission. The earlier Remote host #45048847021 and Waveshare target #45048618202 passed. Physical radio range, real MDD20A/EKM014 safety and full Boat HAL remain uncommissioned; Remote VER028 / Boat VER014 frozen.


## R30H-1 — actual radios: SC16IS750 FIFO repair and unsigned RF commissioning

- [R30H-1 inspection and bridge-fix report](../../Boat/docs/BOAT_R30H_1_RADIO_BRINGUP_AND_BRIDGE_FIX.md) and [22 physical RF signoffs (NOT_RUN)](../../Boat/docs/BOAT_R30H_1_RF_MODULE_SIGNOFF.csv). [R30H STOP/10s RTH trials](../../Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv) remain 22 NOT_RUN.
- **Fixed a real source defect:** Remote `SC16IS750Bridge::writeFifo()` previously computed `64 - TXLVL` despite NXP TXLVL reporting *free slots*, making an empty FIFO appear full. Corrected Remote TX writes and `flush()`, added pure 64-byte FIFO logic, host C++ regression and static negative tests.
- **Fail-closed startup:** Remote primary nRF GPIO6/15 and SC16IS750/E22 now require signed PCB, carrier, RF-interop and airtime flags; Boat E22 adds a signed airtime gate. All new signoff flags remain `0`. E22 `EST_AIRTIME_MS=25` is explicitly *unmeasured*; do not treat 1% software counter as legal compliance. Existing R30A physical-actuator locks remain false.
- [Boat host R30H-1 #45051432923](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45051432923) and [Remote host R30H-1 #45051429416](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45051429416) **SUCCESS** for source/CI regressions. Rebuilds after final header comments must be inspected separately; no physical pairing, scope measurements, STOP output time or real RTH was verified.
- **Next:** Inspect purchased exact board and modem suffix revisions, scope UART TXLVL/9600-8N1 and TTL voltages, verify regulatory 868MHz airtime and profile readback; only then attempt RF-only authenticated pairing with all MDD20A VM/EKM014 VH disconnected. Boat VER014 and Remote VER028 unchanged; sonar deferred.

## R30H-2 — Boat-local diagnostic trace and simulated dual-radio timing (2026-11-22)

Added [R30H-2 Boat-local trace & simulation](../../Boat/docs/BOAT_R30H_2_LOCAL_TRACE_AND_SIMULATION.md): bounded 32-record Boat trace for authenticated STOP reception, logical STOP intent, dual-link stale/recovered transitions and RTH entry, with source-local millis() and last accepted authenticated RX timestamp. Optional USB output is **disabled by default** and compiled only in a separately staged, **non-actuating, radio-locked** test variant. No physical motor/solenoid driver or live RF link is enabled.

New C++11 ring/timer regression and Python serial-log analyzer reject missing STOP intent, buffer overflows, invalid/early (under 10s) RTH and misleading physical-current labels. `Boat/tools/verify_r30h_radio_preflight.py` now checks the trace remains locked and no critical-path USB I/O was added. Existing 250ms primary / 6000ms backup freshness and last-authenticated-RX +10000ms conditional RTH are **unchanged**; these show a potential up-to-6s stale-E22 stop delay in software, not an RF measurement.

[Boat host run #45052442054](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45052442054) completed SUCCESS with the new R30H-2 tests; [Boat host #45052521183](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45052521183) completed SUCCESS after serial delimiter correction. A separate Boat target build with R30H2 USB trace enabled was in progress at last inspection; its result must be confirmed before declaring that optional target compile accepted.

**Physical state:** R30H-1 board/rail/antenna gates 22 NOT_RUN, R30H RF/STOP/RTH test matrix 22 NOT_RUN, R30A physical safety gates still OPEN. No real RF/STOP timings, no current-zero observation, no module purchase or physical driver commission. Boat VER014 / Remote VER028 held; sonar deferred. **Next R30H-3**: signed radio SKU/level/airtime and shared-clock RF-only bench capture, then revise unsafe backup pacing/freshness if measured evidence warrants.

## R30H-3 — software-only E22 STOP worst-case audit and Remote queue trace (2026-11-22)

**Implemented in current branch:** [R30H-3 engineering review](../../Boat/docs/BOAT_R30H_3_RF_STOP_EXPOSURE_REVIEW.md), [16 unsigned physical acceptance gates](../../Boat/docs/BOAT_R30H_3_STOP_RF_ACCEPTANCE.csv), [source-derived 1ms phase simulator](../../Boat/tools/simulate_r30h3_rf_timing.py) and exhaustive host tests. The Remote now has a compile-time-disabled 16-event local STOP trace (`r30h3_stop_trace.h`, `boat_radio.cpp`) distinguishing request, nRF driver-buffer admission, E22 3-second pacing deferral and E22 UART-buffer admission. This does **not** change the RF send pacing, command authorization, HMAC, or real STOP execution.

**Software-derived exposure, not measured:** E22 STOP can be locally deferred up to 3000ms by 3s pacing, with unpredictable UART/RF/Boat latency afterward; after simultaneous radio cut, the Boat's 6000ms backup freshness may keep its software watchdog from total-loss STOP for ~3.002–6.001s in the modeled heartbeat phases, plus unknown runtime delay. The 10s conditional RTH clock remains anchored to last Boat authenticated RX, not RF cut. At an entirely **hypothetical** 25ms E22 frame airtime, 1200 heartbeats/hour cost 30s of a provisional 36s per-hour software budget; 30ms hypothetical airtime leaves zero command headroom. **Actual airtime, legal EU868 duty and EIRP need physical verification.**

[Boat host #45054319954](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45054319954) passed the corrected phase-sweep regression; [Remote host #45054438088](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45054438088) passed the new Remote event-ring regression with corrected timing parser. New preflight CI additionally forbids any R30H-3 fabricated physical signoff. Latest Remote ESP32 target compilation with trace disabled is checked separately. **All R30H-3 16 physical gates NOT_RUN**, existing R30H-1 22 module gates and R30H 22 RF trials NOT_RUN. R30A common-3S independent hardware STOP/one-hot/current-zero requirements remain OPEN. Boat VER014 / Remote VER028 unchanged; sonar still deferred.

**Next R30H-4:** obtain hardware-specific module/level/antenna/airtime measurements with a common-clock logic analyzer, demonstrate actual emergency STOP delivery and total-link-loss latency with MDD20A VM/EKM014 VH disconnected, determine an independently approved maximum STOP limit, then redesign any unacceptable 6s/3s exposures before physical-actuator commissioning.

## R30H-4 — common-clock RF-only capture protocol; physical data still NOT_RUN (2026-11-22)

- [External clock bench procedure](../../Boat/docs/BOAT_R30H_4_EXTERNAL_CLOCK_RF_BENCH.md) defines the safe RF-only capture order, including physical disconnection of MDD20A VM / EKM014 VH (the active **single common 3S battery bus** remains unchanged). [Empty capture CSV](../../Boat/docs/BOAT_R30H_4_LOGIC_CAPTURE_TEMPLATE.csv) and [UNSPECIFIED manifest](../../Boat/docs/BOAT_R30H_4_CAPTURE_MANIFEST_TEMPLATE.json) are new **templates, not measured evidence**. [10-channel testpoint worksheet](../../Boat/docs/BOAT_R30H_4_RF_ONLY_PROBE_CROSSWALK.csv) has **10 NOT_READY** mappings; in particular authenticated Boat STOP/RTH electrical marker GPIOs **do not exist yet**.
- [External analyzer read-only validator](../../Boat/tools/analyze_r30h4_common_clock_capture.py) enforces source identity, capture SHA256, one physical clock, microsecond timestamp ordering and label/edge polarity, and refuses to equate nRF CSN/IRQ or E22 UART with authenticated STOP, physical zero-current or true end-to-end radio latency. A synthetic or even apparently complete hardware trace gets **REVIEW_REQUIRED exit 4**, never physical PASS. Empty evidence gets NOT_RUN exit 3; malformed captures exit 2.
- [R30H-4 hostile host regressions](../../Boat/tests/test_r30h4_common_clock_capture.py) and original [R30H safety preflight](../../Boat/tools/verify_r30h_radio_preflight.py) reject fabricated external captures, signed probe/manifest templates, inconsistent clocks, 9,999,999us RTH, and missing disconnection evidence. [Boat host #45055157836](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45055157836): **SUCCESS** for the initial source/preflight integration; the subsequent additional negative-regression run must be verified separately.
- **No physical milestones changed:** R30H original 22 trial scenarios NOT_RUN; R30H-1 22 module gates NOT_RUN; R30H-3 16 RF/STOP acceptance gates NOT_RUN; R30A 24 physical readiness gates OPEN plus earlier 81 NOT_RUN. Boat/Remote RF pin and duty flags remain 0, STOP 3-second E22 queue and 6-second backup freshness remain unresolved hazards, and independent STOP/current-zero hardware remains uncommissioned. Boat VER014 / Remote VER028 frozen, sonar deferred.
- **Next R30H-5:** prepare a *non-actuating* verified debug-marker allocation plan for authenticated STOP/RTH before any real E2E timing claims; actual measurements demand purchased modules, safe antennas/RF compliance, reviewed voltage levels, and one common external logic-analyzer clock.

## R30H-5/6 — GPS UTC on both ESP32-S3 wall clocks and marker readiness (2026-11-22)

[GPS UTC and R30H-6 report](../../Boat/docs/BOAT_R30H_6_GPS_UTC_AND_MARKER_READINESS.md): the Boat reads verified GNSS UTC from M100 PRO (RMC/GGA/GSA or UBX NAV-PVT with valid fix/time). After two consistent samples the Boat disciplines its CLOCK_REALTIME. Boat sends versioned, HMAC-authenticated subtype-9 UTC via priority-limited nRF24; Remote accepts only valid session/sequence/primary frames, requires two coherent samples and disciplines its CLOCK_REALTIME via settimeofday. No PPS is commissioned; timing includes unmeasured UART/RF delays.

The two monotonic millis clocks remain independent and untouched: STOP, link freshness and last-authenticated-RX +10000ms RTH do not use civil UTC. GPS UTC is COARSE display/log time, never shared physical microsecond STOP evidence. R30H-6 adds shared fail-closed clock quality/marker policy, last GPS UTC sample age and Remote boatRadio_utcQuality() which reports INVALID without fresh primary. No marker GPIO selected or driven; seven actual board/IO/common-clock safety conditions are still unsigned.

[Boat host #45056406215](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056406215) SUCCESS; [Remote host #45056411520](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056411520) SUCCESS for source-level clock quality and marker isolation. Real GNSS/radio/oscillator measurements remain NOT_RUN. R30H original 22 RF checks and R30H-1 22 board signoffs NOT_RUN, R30H-3 16 safety acceptance NOT_RUN, R30H-4 ten probe mappings NOT_READY, R30A independent hardware STOP/current-zero OPEN. Boat VER014 and Remote VER028 unchanged. Next R30H-7: physical UART/PPS review and RF-only external-clock timing after purchased module inspection.

## R30H-5 — GNSS UTC RTC discipline on Boat and Remote (2026-11-22)

**Source implemented; physical UTC/PPS/RF accuracy NOT_RUN.** [R30H-5 GPS/UTC engineering report](../../Boat/docs/BOAT_R30H_5_GPS_UTC_CLOCK_DISCIPLINE.md) defines the new UTC source authority: Boat HGLRC M100 PRO UBX NAV-PVT valid date/time/tAcc/nano, NMEA RMC date fallback, Gregorian validation and fix freshness, and `settimeofday()` of Boat ESP32 system CLOCK_REALTIME (only under a commissioned tracker sensor config). Remote receives a dedicated **authenticated, sequence-checked, nRF-only telemetry subtype 9** with UNIX UTC seconds/milliseconds and applies RTC update after two consistent observations, at most once/minute. Shared wire code: [Boat](../../Boat/source/radio_utc_time_v1.h) and [Remote](../source/radio_utc_time_v1.h), and read-only `boatRadio_utcNow()` for Remote UI/logs.

**Safety isolation:** UTC is informational; time packets never refresh radio/control freshness, reset STOP, change link lease, count toward 10s RTH, authorize manual/hopper/hook or cause E22 airtime. The existing 250ms/6000ms heartbeat freshness and authenticated-RX +10000ms conditional RTH remain **monotonic `millis()`**. No physical radio or actuator HAL enabled. Without actual inspected GNSS modules, UTC is unavailable and no board was synchronized on the bench. UART serial readout + one-way RF transfer **do not provide PPS/microsecond clock alignment**, so the R30H-4 external common-clock analyzer remains mandatory for physical STOP timing.

[Boat GNSS UTC C++ tests](../../Boat/tests/r30h5_gnss_utc_clock_test.cpp), [Remote clock tests](../tests/r30h5_utc_clock_test.cpp), [fail-closed UTC source preflight](../../Boat/tools/verify_r30h5_gps_time_isolation.py) and hostile tests cover date validity, leaps, invalid NAV-PVT, stale time, time regressions, wraparound and radio HMAC/sequence isolation. [Boat host CI #45056333757](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056333757) and [Remote host CI #45056333751](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056333751) both **SUCCESS**. [Boat ESP32 target #45055961006](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45055961006) **SUCCESS**; latest Remote target compile must be checked individually and is not claimed PASS here.

**Next R30H-6:** non-actuating, read-only [UTC quality/marker readiness contract](../../Boat/source/r30h6_utc_marker_policy.h) — no debug GPIO, PPS wire, physical current-zero or external-marker signoff has been installed. R30H-1 22 module signoffs, R30H 22 RF trials, R30H-3 16 physical STOP gates are NOT_RUN; R30H-4 ten mappings NOT_READY; R30A 24 safety gates OPEN. Boat VER014 / Remote VER028 frozen; sonar undecided.

## R30H-7 — GPS UTC consistency and physical PPS signoff gates (2026-11-22)

[Engineering review](../../Boat/docs/BOAT_R30H_7_GNSS_CLOCK_RECOVERY_REVIEW.md) and [12 unsigned real GNSS/PPS clock-offset tests](../../Boat/docs/BOAT_R30H_7_GNSS_PPS_RF_CLOCK_SIGNOFF.csv) are committed. Boat NMEA UTC now requires **RMC date/UTC second equal to the valid GGA fix second**; unequal epochs refuse the wall-clock discipline even when a previous position remains fresh. New host regression verifies mismatch is denied and matching GGA restores qualification. Remote `boatRadio_utcNow()` now also refuses a stale authenticated nRF primary link, independent of any previously set RTC. Both ESP32 still synchronize **only CLOCK_REALTIME from GNSS**, not millis()/STOP/RTH timebase, and neither is PPS calibrated. There is no deployed GNSS or real RF timing measurement.

Shared preflight and hostile tests verify epoch gating, primary freshness, identical UTC wire codec and no fabricated PPS data. [Remote host #45056804347](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056804347) **SUCCESS** after static guard repair. [Boat host #45056804415](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056804415) was in progress at last inspection; do not claim final compile/test success until checked.

**Physical states unchanged:** R30H-7 12 GNSS/PPS gates NOT_RUN; R30H-4 10 probe mappings NOT_READY; R30H-3 16 STOP gates NOT_RUN; R30H-1 22 module signoffs NOT_RUN, original R30H 22 RF scenarios NOT_RUN; R30A independent current-zero safety gates OPEN. Official Boat VER014 / Remote VER028 frozen; no actuator authorization. **Next R30H-8:** inspect real GNSS UART mode/PPS pin/logic level, commission RF-only pairing and measure UTC phase / STOP latency against a single independent external clock before considering precise timestamp calibration.
