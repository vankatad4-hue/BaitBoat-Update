# Flytec V080 Retrofit — Master Status R5

Updated: 2026-11-20. Development branch; no firmware compile, hardware test, ERC/DRC, or operational release has been validated.

## Selected architecture
- Retain Flytec V080 hull, main switch, battery indicator, two 550 brushed motors.
- Replace boat control electronics.
- Remote: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480/LVGL9.6.0, touchscreen, two joysticks,16 physical buttons,OK/confirm.
- Primary radio: nRF24L01+ PA/LNA on both ends,2.4GHz,250kbps initial.
- Backup: matching EBYTE E22-900T22S UART LoRa modules on remote and boat. Tracker integrated SX1262 unused for first test.
- Remote LEFT antenna=2.4GHz nRF; RIGHT=868MHz LoRa. ESP32 Wi-Fi/Bluetooth uses onboard antenna.
- Mission: manual waypoint save→autopilot to waypoint→bait/hook release→return home. Boat-side failsafe must return only if GNSS/heading/battery/geofence health valid; otherwise stop safely.

## Remote hardware R5 — mechanical/button revision
- R5 shell CAD envelope310×209×69.5mm; two-piece front/rear shell prototype, central 4.3in display, twin sticks, two 4×2 button banks (four columns across, two rows high per side), OK and power keys, dual antenna exits.
- Right bank top four keys are independent H-L/H-R/K-L/K-R release controls. Combined release commands are disabled/rejected.
- MCP23017 physical button scanning/debounce, mapping and release/OK event wiring are in remote loop; GPIO crosswalk and actual hardware remain unverified.
- Remote release commands require authenticated primary link, STOP-confirmed released and boat interlock telemetry. Host-tested 1.2s release-key + 0.8s OK confirm within4s guard exists. `actuatorSafetyReady` is false by default and is not set by current telemetry, so release remains disabled.
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
- Next snapshot markers are Remote `006` and Boat `006`; the packaging workflow creates versioned ZIPs and SHA256 artifacts from the branch snapshot. These are pending successful workflow verification, not yet confirmed generated/downloadable archives.
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
- ZIP snapshot ver006 is pending Actions workflow verification. Existing ver004 archives are preserved.
- Release archives ver004 remain unchanged. ver005 is the next snapshot and not yet verified.
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
- User-confirmed physical button arrangement: **4 columns × 2 rows on each side** (8 keys per bank), replacing the previous 2 columns × 4 rows.
- R6 concept SVG and mechanical/button-map documents are committed. These remain visual design references only; the enclosure CAD has not been dimensionally redesigned or checked for component fit yet.
