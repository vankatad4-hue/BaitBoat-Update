# Boat Hardware and Firmware

## Baseline
- Retain Flytec V080 hull, two 550 brushed motors, main switch and battery indicator.
- Reported3S4P pack/20A BMS; confirm.
- Controller candidate Heltec Wireless Tracker ESP32-S3+UC6580 GNSS; exact revision/pin map TBD.
- Primary nRF24L01+ PA/LNA 2.4GHz.
- Backup matching E22-900T22S UART LoRa module; integrated SX1262 unused in first test.
- Bait FW0630 009 and hook FW0630 001 nominal12V coils.

## Power
- Pack→BMS→main fuse→retained switch→BUS_3S.
- Controller/E225V electronics buck candidate≥2A peak.
- nRF3.3V rail measured/selected.
- Solenoid rail direct pack only if coil tolerance confirmed; otherwise regulated12V buck-boost.
- Motor-driver current/fuse/cable/BMS coordination pending.

## Fit
- Control carrier77×79×14mm measured preliminary.
- Cytron MDD20A candidate88.9×78.74mm; does not fit carrier footprint. Separate placement or compartment redesign required.
- Lid clearance~30mm; full CAD fit not done.

## Firmware
The active version has a diagnostic ESP32-S3 application, authenticated dual-radio and mission software, sensor acquisition source, manual/mission release supervisor, telemetry, and direct MDD20A output planner. The owner has removed the companion MCU architecture. No physical driver attached; all actuator GPIOs remain unassigned and secure key provisioning is disabled by default. Not operational.

## Hardware docs
- [Active Heltec-only board integration R5](docs/BOAT_BOARD_INTEGRATION_R5.md)
- [Active R5 GPIO/connector schedule](docs/BOAT_HARDWARE_PIN_CONNECTIONS_R5.csv)
- [Boat test build R2](docs/BOAT_HARDWARE_TEST_BUILD_R2.md)
- [Boat electrical capture R2](docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md)
- [Boat block/power diagrams](docs/BOAT_BLOCK_AND_POWER_SCHEMATICS_R1.md)
- [Boat radio pin map R3](docs/BOAT_RADIO_PIN_ASSIGNMENTS_R3.md)
- [Controller resource/pin audit R1](docs/BOAT_CONTROLLER_PIN_AUDIT_R1.md)
- [Boat electrical connector CSV R3](docs/BOAT_HARDWARE_PIN_CONNECTIONS_R3.csv)
- [Older connector CSV R2](docs/BOAT_HARDWARE_PIN_CONNECTIONS_R2.csv) (superseded)
- [Boat E22 connector CSV R1](docs/BOAT_E22_UART_PIN_CONNECTIONS_R1.csv)
- [Motor-driver fit review](docs/BOAT_MOTOR_DRIVER_FIT_REVIEW_R1.md)
- [Boat BOM candidates](docs/BOAT_BOM_CANDIDATES_R1.md)

## Test sketches
- [Boat nRF bench](hardware_test/nRF24_Bench/nRF24_Bench.ino)
- [Boat E22 UART bench](hardware_test/E22_UART_Bench/E22_UART_Bench.ino)
- [Test instructions](hardware_test/README.md)

## ZIP snapshots
- [Shared build status](../docs/ZIP_BUILD_STATUS_R1.md)
- Current Boat version marker: `releases/VERSION` = `014`; development snapshots in Actions. VER015 gated on acceptance.
- Packaging automation: [ZIP release policy](../docs/ZIP_RELEASE_POLICY_R1.md) and `.github/workflows/package-flytec-zips.yml`. Workflow artifacts are generated after a branch push; integrity must be verified before calling them released.

## Status
Native KiCad/ERC/DRC, exact board pin audit, converter/driver selection, current measurements, RF/antenna tests, secure key/session, failsafe and actuator integration remain open. Bench only.


## Latest verified target baseline
Remote #87 and Boat #16 were verified before this architecture rollback; rerun Boat target and regression on the new source. Physical acceptance remains NOT RUN.
