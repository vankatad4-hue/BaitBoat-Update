# Remote Hardware and Firmware

## Baseline
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard,800×480,LVGL9.6.0.
- Handheld target300×210×70mm; screen, joysticks,16 buttons,OK/confirm; 1S4P18650 battery.
- Primary nRF24L01+ PA/LNA 2.4GHz LEFT antenna.
- Backup matching E22-900T22S UART LoRa EU868 RIGHT antenna.
- Wi-Fi/Bluetooth uses onboard ESP32 antenna.

## Power
- Controller/display5V: onboard CS8501 boost.
- nRF radio3.3V: TPS63070.
- E22 VCC5V: dedicated BAT_PROT→TPS61023 boost; CS8501 rail reserved for controller/display. Never parallel converter outputs.
- E22 UART/M0/M1/AUX are 3.3V logic.
- Battery protection/fuse and converter layout/test pending.

## Pin proposals
- I2C GPIO8/9.
- SPI GPIO11/12/13 shared TF.
- nRF CE GPIO6 (Sensor AD repurposed), CSN GPIO15 (RS485 TX repurposed; disconnect bus), IRQ MCP23017 0x21 GPA1.
- SC16IS750 bridge @0x48 drives E22 UART and mode/reset/AUX.
- Physical board revision, connector crosswalk and continuity must be verified.

## Firmware
Remote source includes nRF/E22 transports, SC16 bridge, CRC/HMAC protocol, command codec, LinkManager and heartbeat scheduling. Key provisioning deliberately returns false, so no authenticated communication or command TX. Session replay protection, app ACK, telemetry and boat action integration remain incomplete. No compile/test has been recorded.

## Hardware docs
- [Remote test build R4](New_Handheld_Remote/PCB/REMOTE_HARDWARE_TEST_BUILD_R4.md)
- [Remote schematic capture R4](New_Handheld_Remote/PCB/SCHEMATIC_CAPTURE_SHEETS_R4.md)
- [Remote power nets R8](New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R8.md)
- [Remote logical pin map R4](New_Handheld_Remote/PCB/REMOTE_HARDWARE_PIN_ASSIGNMENTS_R4.md)
- [Remote electrical pin CSV R4](New_Handheld_Remote/PCB/REMOTE_ELECTRICAL_PIN_CONNECTIONS_R4.csv)
- [Mechanical envelope R4](New_Handheld_Remote/MECHANICAL/REMOTE_MECHANICAL_ENVELOPE_R4.md)
- [Radio architecture R3](../docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md)
- [Protocol spec R2](../docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R3.md)

## Test sketches
- [Remote nRF bench test](hardware_test/nRF24_Bench/nRF24_Bench.ino)
- [Remote E22/SC16 bench test](hardware_test/E22_SC16IS750_Bench/E22_SC16IS750_Bench.ino)
- [Hardware test instructions](hardware_test/README.md)

## Status
Not a fabrication release. Native KiCad/ERC/DRC, PCB, mechanical mockup, pin audit, power/thermal/RF test and secure protocol integration remain open. Do not operate boat.
