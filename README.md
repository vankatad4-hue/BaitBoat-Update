# Flytec V080 Bait Boat Retrofit

> **ACTIVE HARDWARE RULE — R15 MODULE-ONLY (2026-11-21):** Custom electronic PCB/board fabrication has been canceled for BOTH Boat and Remote. Use ready-made manufactured modules, keyed/removable harnesses, premade fuses/terminal adapters; keep custom 3D printed non-electronic enclosures only. Previous KiCad and Remote PCB drafts are **archived engineering reference**, not active manufacturing plans. Exact module choices and safety parts remain subject to physical qualification. Read [R15 module-only policy](docs/HARDWARE_MODULE_ONLY_POLICY_R15.md), [Boat implementation](Boat/docs/BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md), and [Remote implementation](Remote/docs/REMOTE_MODULE_ONLY_IMPLEMENTATION_R15.md). **No power to motors/solenoids.**

**GitHub repository is the canonical project source.** Development branch: `hardware-ready-dual-radio-r1`.

## Baseline
- Retain V080 hull,12V550 brushed motors,main switch,battery indicator; replace control electronics.
- Boat controller candidate Heltec Wireless Tracker ESP32-S3+UC6580 GNSS.
- Remote Waveshare ESP32-S3-Touch-LCD-4.3 Standard,800×480,LVGL9.6.0; R5 handheld CAD and four independent bait/hook release controls (H-L/H-R/K-L/K-R).
- Primary nRF24L01+ PA/LNA2.4GHz; backup matching E22-900T22S UART LoRa modules on both ends.500m remains unvalidated.
- CHIRP sonar selection/integration pending.

## Current development status
- Remote firmware: joystick acquisition/calibration, LVGL UI scaffolds, dual-radio transport, authenticated receive dispatch, ACK tracking and partial telemetry freshness/safety application. Not fully compiled or hardware-tested.
- Boat firmware: transport/codec/queue/failsafe scaffolds; portable payload encoders with host tests; session epoch echo, per-radio sequence guards, pending-command deduplication and queue-level ACCEPTED/DUPLICATE/BUSY ACK scaffolds. No boat application/executor, telemetry source, EXECUTED ACK or actuator integration.
- Key provisioning remains fail-closed. Radio GPIOs remain disabled/TBD pending exact board revision and pin audit.
- Remote R6.1 enclosure/grip/battery cradle and fit coupons are draft CAD; no actual component fit or live battery validation.
- Native KiCad schematics/ERC/DRC, PCB fabrication, full firmware compile, RF range/compliance, power/current/thermal and hardware tests remain incomplete. Boat must not be operated.

## Main documents
- [Comprehensive progress review and next engineering gates](docs/PROJECT_PROGRESS_REVIEW_R1.md)
- [Current ZIP integrity/build status](docs/ZIP_BUILD_STATUS_R1.md)
- [Master project status](Remote/docs/PROJECT_MASTER_STATUS.md)
- [Remote test build R4](Remote/New_Handheld_Remote/PCB/REMOTE_HARDWARE_TEST_BUILD_R4.md)
- [Remote schematic capture R4](Remote/New_Handheld_Remote/PCB/SCHEMATIC_CAPTURE_SHEETS_R4.md)
- [Remote power net draft R8](Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R8.md)
- [Remote pin map R4](Remote/New_Handheld_Remote/PCB/REMOTE_HARDWARE_PIN_ASSIGNMENTS_R4.md)
- [Remote connector CSV R4](Remote/New_Handheld_Remote/PCB/REMOTE_ELECTRICAL_PIN_CONNECTIONS_R4.csv)
- [Remote mechanical envelope](Remote/New_Handheld_Remote/MECHANICAL/REMOTE_MECHANICAL_ENVELOPE_R4.md)
- [Dual-radio architecture R3](Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md)
- [Firmware protocol spec R2](Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R3.md)
- [Boat test build R2](Boat/docs/BOAT_HARDWARE_TEST_BUILD_R2.md)
- [Boat electrical design R2](Boat/docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md)
- [Boat controller pin audit R1](Boat/docs/BOAT_CONTROLLER_PIN_AUDIT_R1.md)
- [Boat radio pin assignment R3](Boat/docs/BOAT_RADIO_PIN_ASSIGNMENTS_R3.md)
- [E22 configuration decode R1](Remote/docs/E22_CONFIGURATION_DECODE_R1.md)
- [Repository path audit R1](docs/REPOSITORY_PATH_AUDIT_R1.md)
- [ZIP packaging policy R1](docs/ZIP_RELEASE_POLICY_R1.md)

## Safety
Do not connect motor/solenoid power or operate boat. Hardware pins, power, RF compliance, failsafe and actuator interlocks require verification. Bench radio sketches transmit fixed CRC-only test frames.


## Versioned project ZIPs
- Current package markers: Remote `028`, Boat `014`; these are development snapshot version markers, not verified operational releases.
- Existing branch ZIPs are unverified snapshots due to prior workflow regeneration under unchanged filenames. A fresh release-marker bump and clean archive verification are required.
- These are development snapshots, not hardware or operational releases. See [ZIP build status](docs/ZIP_BUILD_STATUS_R1.md) and [ZIP release policy](docs/ZIP_RELEASE_POLICY_R1.md).

**Pin audit result:** the Heltec Wireless Tracker revision is not identified; all boat radio GPIO assignments remain disabled/TBD. The integrated SX1262/GNSS/TFT/USB and boot/strap resources are reserved pending physical board identification.


## Resume checkpoint (2026-11-20)
Read **[Continuation Checkpoint VER001](docs/CONTINUATION_CHECKPOINT_VER001.md)** first when continuing work in another chat. It consolidates selected architecture, current source/UI work, blockers, verification status and ordered next tasks.

Latest source/UI work is unreleased ver029 development; Remote marker 028 and Boat marker 014 remain unchanged. The branch ZIPs are unverified snapshots. Workflow #880 previously succeeded and version-archive preservation is guarded by marker changes; documentation sync runs #888 and #889 subsequently succeeded; latest #889. No complete ESP32 firmware compile, LVGL Pro render, PCB ERC/DRC or hardware test is confirmed.


## Current development verification (2026-11-20)
- Remote target CI compile #19 passed: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44781879101. Full compile/link and artifact upload; huge_app partition selected.
- Build usage: 1,430,957 bytes firmware (45% app); 178,136 bytes globals (54%, 149,544 bytes remaining). No C++ errors or warnings reported.
- Host tests and ZIP workflow passed on previous firmware-source commit; current change is build workflow/documentation only. Not hardware tested.
- Continue from docs/CONTINUATION_CHECKPOINT_VER001.md; next task is runtime timer and offline-map import/renderer safety audit. Boat is still a non-operational scaffold.
