# Flytec V080 Bait Boat Retrofit

**GitHub repository is the canonical project source.** Development branch: `hardware-ready-dual-radio-r1`.

## Baseline
- Retain V080 hull,12V550 brushed motors,main switch,battery indicator; replace control electronics.
- Boat controller candidate Heltec Wireless Tracker ESP32-S3+UC6580 GNSS.
- Remote Waveshare ESP32-S3-Touch-LCD-4.3 Standard,800×480,LVGL9.6.0.
- Primary nRF24L01+ PA/LNA2.4GHz; backup matching E22-900T22S UART LoRa modules on both ends.500m remains unvalidated.
- CHIRP sonar selection/integration pending.

## Current development status
- Remote/boat nRF and E22 transport scaffolds, SC16 bridge, CRC/HMAC protocol, command codec, dual-radio manager, heartbeat, dedupe and failsafe helpers exist.
- Secure key provisioning returns false; no authenticated control, actuator execution or boat application.
- Remote envelope300×210×70mm,carrier100×75mm proposed. Boat carrier77×79×14mm; MDD20A candidate does not fit that footprint.
- Electrical capture specs, pin tables, BOM candidates, block diagrams and radio bench sketches exist. Native KiCad/ERC/DRC, fabrication PCB, compile and hardware test remain incomplete.

## Main documents
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

## Safety
Do not connect motor/solenoid power or operate boat. Hardware pins, power, RF compliance, failsafe and actuator interlocks require verification. Bench radio sketches transmit fixed CRC-only test frames.
