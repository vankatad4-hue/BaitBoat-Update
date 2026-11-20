# Flytec V080 bait boat and remote — master project record R3

**Consolidated:** 2026-11-20 (schematic design update). Repository source is canonical; latest schematic capture specification is documented below.

## Archive versioning
ZIPs use Flytec_Remote_verXXX.zip and Flytec_Boat_verXXX.zip. Never overwrite a prior release. ver005 archives exist but are not fully synchronized to current repository work.

## Project
Retain Flytec V080 hull, motors, battery indicator and main switch; replace boat control electronics. Remote is a new handheld design using existing Waveshare 4.3 touchscreen board.

## Boat baseline
- 12V 550 brushed motors; motor startup/stall current unmeasured.
- Reported pack 3S4P, BMS 20A; verify actual cell/pack data.
- Solenoids FW0630 009 DC12V bait and FW0630 001 DC12V hook; coil current/resistance/duty not measured.
- Carrier PCB envelope 77×79×14mm, compartment lid clearance ~30mm.
- Heltec Wireless Tracker V1.2 ESP32-S3/UC6580 GNSS candidate.
- GY-91 MPU-9250 + BMP280 motion/heading candidate; exact breakout/pinout and calibration pending.
- nRF24L01+ PA/LNA 250kbps initial; 500m range goal unvalidated.
- Cytron MDD20A motor-driver candidate; confirm motor current before use.
- Sonar: full CHIRP echogram/fish arches desired, module/interface still unselected.
- Autopilot mission: save waypoint, send, release bait/hook, return; failsafe return-home only with valid navigation/system health.

## Remote schematic design progress (2026-11-20)
- GPIO/peripheral assignments checked against official Waveshare online docs; PORT1 physical pin-number crosswalk remains provisional due schematic/document discrepancy. Confirm exact board revision/orientation before wiring; see `WAVESHARE_HEADER_MAPPING_AND_PIN_ASSIGNMENTS_R2.md`.
- Capture-sheet design and net-level drafts created: `SCHEMATIC_CAPTURE_SHEETS_R1.md`, `SCHEMATIC_NETLIST_BLOCKS_R1.md`, `SCHEMATIC_CAPTURE_CHECKLIST_R1.md`.
- Battery protection/power branches specified; controller power via onboard CS8501 boost, external TPS61023 fallback only; radio TPS63070 dedicated rail.
- Controls: ADS1115 @0x48 for four joystick axes, MCP23017 @0x20 for buttons, second ADS1115 @0x49 for battery/NTC/optional radio rail monitoring, MCP23017 @0x21 for radio CE/CSN/IRQ candidate.
- Shared SPI GPIO11/12/13 with onboard TF card; radio priority and bus serialization required.
- Expander-based radio control remains an explicit validation gate; no native KiCad capture/ERC/bench test has yet been performed.

## Remote baseline
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480; onboard ESP32 antenna.
- 2 joysticks, 16 buttons (2×4 banks on each side), touch UI; ergonomic remote concept remains.
- Pack 1S4P 18650; exact cells and BMS not selected.
- nRF24 PA/LNA module with external antenna. Wi-Fi/Bluetooth coexistence requires testing.

## Revised remote power tree
- Pack/BMS output → fuse candidate → reverse-protection P-MOS candidate → BAT_PROT.
- Controller/display 5V: use Waveshare onboard CS8501 battery boost via single-cell PH2.0 connector. Manufacturer documentation says boost from single cell to 5V and board consumption 5V/450mA. Verify current/transient/thermal performance. External TPS61023 remains fallback only; do not parallel converters.
- Radio 3.3V: dedicated TPS63070 buck-boost. 1.5µH candidate; feedback 470k/150k, nominal 3.3067V; effective capacitors and configuration pending tests. EN candidate controlled after boot, default OFF.
- Protection shortlist: Littelfuse 0458004.DR 4A fast-acting fuse; onsemi FDS6575 P-MOS (Drain pack side, Source protected output); battery ADC divider and NTC. Parts/coordination not validated.
- Current estimate at 3V loaded: controller 5V/450mA at assumed 90% ≈0.83A battery; radio 3.3V/300mA at assumed 90% ≈0.37A; total ≈1.20A. Estimate only.

## Remote controls/peripherals
- Board I2C GPIO8/9 shared with touch/CH422G/RTC.
- ADS1115 candidate (I2C 0x48) for four joystick axes, 16-bit, up to 860SPS aggregate; verify latency/noise.
- MCP23017 candidate (I2C 0x20) for 16 button inputs; active-low contacts and software debounce.
- Battery sensing GPIO6/Sensor-AD candidate.
- nRF SPI candidate GPIO11/12/13 shared with TF card; separate CS. Candidate CSN GPIO15 and CE GPIO43 are provisional; GPIO attachments and header exposure must be verified. IRQ omitted/polled initially.
- I2C addresses/pullups, SPI sharing and GPIO map not frozen.

## Engineering status
Power and interface documentation updated. Native KiCad schematics, ERC, PCB layout, fabrication BOM, exact pin/connector assignment, bench validation and hardware bring-up are still not completed. Do not treat docs/net drafts as fabrication-ready.


## New radio redundancy decision — 2026-11-20
- **Added LoRa as backup communications**, alongside nRF24L01+ PA/LNA primary. Both remote and boat must carry matching LoRa modules for the backup link.
- Candidate module: EBYTE E22-900T22S-V2 UART/SX1262 EU868-capable family; exact SKU/configuration is not finalized or purchased. UART pins remain unassigned until GPIO/header audit.
- Remote antenna ports reassigned: RIGHT = EU868 LoRa; LEFT = nRF24 2.4GHz. ESP32 Wi-Fi/Bluetooth uses the Waveshare board's onboard antenna; no external Wi-Fi antenna connector planned.
- LoRa is for heartbeat, low-rate telemetry, reduced-rate/manual safety control, STOP and RETURN. It will not carry sonar streaming. Mission start and bait/hook release are disabled over backup by default.
- Firmware requirements: dual-link LinkManager, primary/backup/no-link UI, handover hysteresis, duplicate command suppression, packet freshness/sequence checks, authenticated/replay-protected commands, independent boat failsafe. Failover must be bench/field tested.
- LoRa module power is provisionally from Waveshare CS8501 5V rail with local bulk/decoupling; simultaneous display/backlight+nRF+LoRa load/thermal test required. If margin inadequate, design a separate boost branch (never parallel boosts).
- EU868/Bulgaria channel, radiated power, duty cycle, airtime and antenna gain/cable loss compliance must be verified; module's 22dBm capability is not an approved setting.
- Design record: Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md. Capture/net/power/BOM documents have LoRa addenda. No firmware code has yet been merged or bench-tested, and no hardware purchased.


## Dual-radio documentation and firmware scaffold files
- Shared architecture/antenna/power/regulatory design: `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.
- Protocol, packet safety, failover state machine, code architecture and pseudocode: `Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R1.md`.
- Boat-side implementation requirements: `Boat/docs/DUAL_RADIO_BACKUP_LINK_R1.md`.
- Remote radio API scaffold: `Remote/source/boat_radio.h/.cpp` (still returns disconnected; no actual drivers).
- Command scaffold adds stable `commandId` and `createdAtMs` fields: `Remote/source/boat_commands.h/.cpp`; no packet-level duplicate handling or authentication yet.
- No firmware was compiled in this update. No release ZIP ver006 was created; ver005 archives remain unchanged.


# Hardware/software integration update R3 — 2026-11-20

## Dual radio correction
- Remote backup: EBYTE E22-900T22S UART LoRa candidate. Boat backup can use the **integrated SX1262** on the Heltec Wireless Tracker controller candidate. A second LoRa module on the boat is not required unless the exact board revision/band or interoperability tests fail.
- Remote E22 UART is routed via an SC16IS750 I2C-UART bridge candidate @0x48 to avoid allocating GPIO15/16 wired to RS485. Bridge clock/crystal and footprints need schematic/datasheet review.
- nRF24 SPI uses Waveshare GPIO11/12/13; CSN GPIO6 candidate direct; CE via MCP23017 only a bench prototype (timing must be measured). Physical header mapping remains unverified.
- ADC addresses moved to ADS1115 0x4A/0x4B; MCP23017 button/radio I/O at 0x20/0x21.
- LoRa antenna RIGHT 868MHz; nRF antenna LEFT 2.4GHz. Boat uses dedicated matching antennas; Heltec LoRa IPEX connector is separate from GNSS RF port.

## Remote electrical capture R2 documents
- `Remote/New_Handheld_Remote/PCB/SCHEMATIC_CAPTURE_SHEETS_R2.md`
- `Remote/New_Handheld_Remote/PCB/REMOTE_HARDWARE_PIN_ASSIGNMENTS_R2.md`
- `Remote/New_Handheld_Remote/PCB/REMOTE_ELECTRICAL_PIN_CONNECTIONS_R2.csv`
- `Remote/New_Handheld_Remote/PCB/REMOTE_HARDWARE_TEST_BUILD_R2.md`
- `Remote/New_Handheld_Remote/PCB/REMOTE_BOM_R2.md`
- `Remote/New_Handheld_Remote/PCB/LORA_UART_BRIDGE_R1.md`
- `Remote/New_Handheld_Remote/MECHANICAL/REMOTE_ENVELOPE_R2.md`

## Software implementation status
- Added fixed 32-byte packet frame, CRC-16/CCITT and truncated HMAC-SHA256 codec in `Remote/source/radio_protocol.*`.
- Added command encode/decode, ID/age/backup command-policy checks in `radio_command_codec.*`.
- Added DualRadioManager health state, degraded/backup/recovery, failback hysteresis, backup manual limit 25% and 1Hz control cap.
- Remote radio API and UI state now expose primary/backup/degraded/recovery states. Hardware driver is not registered; all links remain disconnected and command sends fail closed.
- Added boat-side 32-entry action-ID dedupe helper and local failsafe decision evaluator. Not connected to actuator firmware.
- No nRF RF24 transport, E22/SC16 bridge driver, Heltec SX1262 transport, key provisioning, application ACK/telemetry, session negotiation or release-action execution code has been completed. Source not compiled/tested in this update.

## Boat electrical design
- Logical harness and schematic-interface documents: `Boat/docs/BOAT_HARDWARE_PIN_CONNECTIONS_R1.csv`, `BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R1.md`, `BOAT_RADIO_PIN_ASSIGNMENTS_R1.md`.
- MDD20A candidate size (~88.9×78.74mm) exceeds the measured 77×79mm carrier footprint. See `BOAT_MOTOR_DRIVER_FIT_REVIEW_R1.md`; physical placement/layout needs revision.
- Solenoid coil resistance/current and motor startup/stall current remain unmeasured. Fuse, MOSFET, diode, cable and connector values cannot be finalized until measurements.

## Radio regulatory airtime
- Candidate EU SRD sub-band 869.4–869.65MHz is listed at up to 500mW ERP with 10% duty-cycle or polite spectrum access in CEPT/EU conditions; confirm Bulgaria national implementation, device conformity, antenna gain/feed loss and channel access. Initial backup command rate set to 1Hz pending airtime test. Avoid assuming 868.0–868.6MHz 1% subband is suitable for 2Hz control without compliant LBT/AFA.

## Finalization blockers
Native KiCad capture and ERC/DRC, exact physical PORT1 crosswalk, final footprints, board revisions, nRF/LoRa drivers, secure key provisioning, boat receiver/action cache integration, motor/solenoid current measurements, MDD20A fit, converter thermal and RF legality/interoperability tests remain outstanding. No ver006 ZIP release created.


## R3 pairing decision (supersedes R2 LoRa pairing)
- Initial hardware prototype will use **identical EBYTE E22-900T22S UART modules on both remote and boat**. This avoids assuming transparent E22 ↔ raw native SX1262 interoperability. Heltec Tracker's integrated SX1262 remains unused for this link unless later compatibility testing supports it.
- Boat UART must be allocated on a verified spare pair from exact Tracker board schematic; GNSS, integrated LoRa, USB, strapping and other board functions remain reserved. If no direct UART is safe, bridge or controller change.
- Reference: `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md`, `Boat/docs/BOAT_LORA_HARDWARE_DECISION_R3.md`.


## R4 development branch implementation details
- Remote nRF24 source class now uses shared SPI and fixed 32-byte frames; pin defaults CE=GPIO6, CSN=GPIO15, SPI GPIO11/12/13, channel76. CSN GPIO15 is the RS485 TX path: RS485 bus must be disconnected/disabled. CE GPIO6 repurposes Sensor AD. Exact connector crosswalk/continuity and bus setup remain gates.
- Remote E22 uses SC16IS750 I2C UART bridge @0x48, 1.8432MHz crystal, 9600 8N1; M0/M1/AUX/reset through bridge GPIO. The E22 configuration write/readback is in source and bench sketch.
- E22 config: channel18=868.125MHz, 62.5kbps air, 32-byte subpacket, 10dBm output, transparent mode. 868.125MHz is 1% duty band. 25ms estimate; manager interval3s (0.83%) and hourly airtime guard36s. Duty counter is RAM-only, so no field use until persistent accounting/actual airtime measured.
- Added dedicated remote SC16/E22 test sketch and boat direct-UART E22 test sketch; neither sends actuator commands.
- Boat source now contains nRF driver, E22 port, dual-radio receive queue and failsafe/dedupe helpers; all boat GPIO defaults unset. No operational boat application or actuator integration.
- Remote and boat HMAC key store returns false by default; all authenticated link/command paths fail closed. Secure provisioning/rotation and session negotiation are still needed.
- Native KiCad schematic, ERC/DRC and PCB still not generated. Files are detailed electrical capture/test specs, not native fabrication data.
