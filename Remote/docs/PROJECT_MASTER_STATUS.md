# Flytec V080 Retrofit — Master Status R5

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
- Current markers are Remote `022` and Boat `009`; matching ZIPs and SHA256 manifest are committed. Independent extraction/checksum verification is still pending.
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
- Current package markers are Remote ver023 / Boat ver010. ZIP pair and checksum manifest are generated and committed; independent byte-level integrity verification is pending. Existing ver004 archives are preserved.
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
