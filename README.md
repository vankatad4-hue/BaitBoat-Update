# Flytec V080 Bait Boat Retrofit

**GitHub is the canonical project source.** Current hardware/software development branch: `hardware-ready-dual-radio-r1`.

## Baseline
- Retain V080 hull, 12V 550 brushed motors, main switch and battery indicator; replace control electronics.
- Boat controller candidate: Heltec Wireless Tracker ESP32-S3 + UC6580 GNSS.
- Remote: Waveshare ESP32-S3-Touch-LCD-4.3 Standard,800×480,LVGL9.6.0.
- Dual radio: nRF24L01+ PA/LNA primary2.4GHz; matching E22-900T22S UART LoRa backup on both endpoints. 500m remains unvalidated.
- CHIRP echogram sonar selection/integration pending.

## Current development snapshot
- Remote nRF/E22 transports, SC16 bridge, CRC/HMAC codec, command decoder, failover manager and heartbeat scheduler scaffolds exist.
- Boat dual-radio receive/queue, dedupe and failsafe helpers exist, not integrated with actuators.
- Secure key provisioning returns false; authenticated communication/commands disabled.
- Remote mechanical envelope300×210×70mm; carrier100×75mm proposal. Boat carrier77×79×14mm; MDD20A motor driver candidate doesn't fit this carrier footprint.
- Electrical capture specs and test sketches exist; native KiCad/ERC/DRC, fabrication PCB, compile and hardware testing are not complete.

## Links
- Master status: [Remote/docs/PROJECT_MASTER_STATUS.md](Remote/docs/PROJECT_MASTER_STATUS.md)
- Remote test build: [REMOTE_HARDWARE_TEST_BUILD_R4.md](Remote/New_Handheld_Remote/PCB/REMOTE_HARDWARE_TEST_BUILD_R4.md)
- Remote capture sheets: [SCHEMATIC_CAPTURE_SHEETS_R4.md](Remote/New_Handheld_Remote/PCB/SCHEMATIC_CAPTURE_SHEETS_R4.md)
- Remote power net draft: [POWER_SCHEMATIC_NET_DRAFT_R8.md](Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R8.md)
- Dual-radio architecture: [REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md](Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md)
- Protocol spec: [DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R2.md](Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R2.md)
- Boat test build: [BOAT_HARDWARE_TEST_BUILD_R2.md](Boat/docs/BOAT_HARDWARE_TEST_BUILD_R2.md)
- Boat electrical design: [BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md](Boat/docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md)
- Radio test sketches: [Remote/hardware_test](Remote/hardware_test/README.md), [Boat/hardware_test](Boat/hardware_test/README.md)

## Safety
Outputs default OFF. Do not connect motor/solenoid power or operate the boat. Radio tests are fixed CRC-only probe frames. Hardware pin maps, power/current, RF legal settings, failsafe and actuator interlocks must be validated.
