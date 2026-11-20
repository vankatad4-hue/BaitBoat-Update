# Flytec V080 Boat Software

This is the initial boat-side software project scaffold.

## Versioning
- Archive format: `Flytec_Boat_verXXX.zip`
- Initial archive: `Flytec_Boat_ver001.zip`
- Increment XXX by exactly one for every project modification.
- Preserve every previous version; never overwrite an older release.

## Project purpose
Boat-side controller for the Flytec V080 retrofit. The remote-controller firmware is a separate project and is archived separately under `Flytec_Remote/`.

## Current status
This is a documented starter scaffold, not operational boat firmware. It contains no motor, solenoid, nRF24L01+ primary radio, LoRa backup radio, GNSS, compass, sonar, or autopilot drivers. It must not be connected to powered actuators or used to operate the boat.

R2 architecture: nRF24L01+ PA/LNA primary. Backup remote uses E22 UART LoRa; boat can use the Heltec Wireless Tracker candidate's integrated SX1262 if exact SKU/band is confirmed. Raw SX1262/E22 interoperability needs matching PHY settings and testing. The remote protocol/link-manager scaffold is not yet a hardware transport; boat receive/authentication/action-cache integration remains incomplete. See `docs/BOAT_LORA_HARDWARE_DECISION_R2.md`, `docs/BOAT_RADIO_PIN_ASSIGNMENTS_R1.md`, `docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R1.md`, and `../../Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R2.md`.

## Latest release note — ver002

Selected matching nRF24L01+ PA/LNA radio modules for both boat and remote. This release updates project documentation and the safe firmware scaffold comments only; it does not add an operational radio driver.


## Latest hardware selection — ver003

- Main boat processor + GNSS: Heltec Wireless Tracker V1.2 (ESP32-S3FN8 + UC6580), replacing the generic ESP32 carrier and HGLRC M100 PRO B101.
- Heading/motion sensor: GY-91-style MPU-9250 + BMP280, replacing separate QMC5883L and MPU6050 modules. MPU-9250 includes accelerometer, gyroscope and magnetometer; BMP280 is atmospheric pressure/temperature only.
- nRF24L01+ PA/LNA remains the primary radio; boat-side backup LoRa may use the Heltec Tracker's onboard SX1262; an additional module is not planned unless board variant or RF test requires it.
- This release updates architecture documentation and scaffold comments only. Firmware remains non-operational; no sensor, GNSS, radio or actuator drivers have been implemented.
- See `docs/HARDWARE_SELECTION_VER003.md` for integration cautions and verification requirements.


## Hardware update — ver004

- Modular architecture: central carrier PCB with removable cables; no plug-in backplane.
- Selected separate motor driver: Cytron MDD20A dual-channel board, one channel per 550 motor. It is not mounted on the 77 × 79 mm carrier PCB.
- See `docs/CARRIER_PCB_CONNECTION_PLAN_VER004.md` for proposed logical connectors, power domains and verification gates.
- This release adds documentation only; firmware remains non-actuating and no GPIO assignments are approved.


## Dual-radio architecture update (2026-11-20)
- nRF24 primary plus LoRa EU868 backup; matching radios are required on both sides.
- Backup supports safety/low-rate traffic only; STOP/RETURN, limited steering, heartbeat and telemetry. Sonar data is not transported over LoRa.
- Link failover/failback, packet freshness, command IDs, authentication, duplicate suppression and local boat failsafe must be implemented before operation.
- Boat hardware must include a dedicated 868MHz antenna; RF placement and power/current validation remain open.
- See `../../Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.


## Hardware electrical design R1
- Logical pin/harness table: `docs/BOAT_HARDWARE_PIN_CONNECTIONS_R1.csv`.
- Power/radio/motor/solenoid electrical interfaces: `docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R1.md`.
- Motor driver fit: MDD20A candidate is ~88.9×78.74mm, exceeding 77×79mm control-carrier footprint; review `docs/BOAT_MOTOR_DRIVER_FIT_REVIEW_R1.md`. Do not freeze boat layout until driver placement and current are validated.
- Safety helper source: `source/boat_radio_safety.*` provides action-dedup cache and local failsafe decision API only; no actuator integration.
