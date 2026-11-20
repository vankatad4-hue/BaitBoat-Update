# Flytec V080 Bait Boat Retrofit

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
- Current development snapshots: Remote `ver023`, Boat `ver011`; ZIPs and SHA-256 manifest are committed.
- Boat ver011 ZIP SHA-256 independently matches the manifest; archive structure and test files checked. Remote ver023 binary remains unavailable for independent verification. Remote ver023 binary remains unverified.
- These are development snapshots, not hardware or operational releases. See [ZIP build status](docs/ZIP_BUILD_STATUS_R1.md) and [ZIP release policy](docs/ZIP_RELEASE_POLICY_R1.md).

**Pin audit result:** the Heltec Wireless Tracker revision is not identified; all boat radio GPIO assignments remain disabled/TBD. The integrated SX1262/GNSS/TFT/USB and boot/strap resources are reserved pending physical board identification.
