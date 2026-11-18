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
This is a documented starter scaffold, not operational boat firmware. It contains no motor, solenoid, nRF24L01+ PA/LNA radio, GNSS, compass, sonar, or autopilot drivers. It must not be connected to powered actuators or used to operate the boat.

The selected remote-to-boat radio is nRF24L01+ PA/LNA at 2.4 GHz, with 250 kbps as the initial test data rate. This is an architecture decision only; radio pins, power, driver, packet protocol and failsafe are not implemented. See `docs/RADIO_LINK_DESIGN.md` and `docs/PROJECT_MASTER_STATUS.md` for requirements and open items.

## Latest release note — ver002

Selected matching nRF24L01+ PA/LNA radio modules for both boat and remote. This release updates project documentation and the safe firmware scaffold comments only; it does not add an operational radio driver.


## Latest hardware selection — ver003

- Main boat processor + GNSS: Heltec Wireless Tracker V1.2 (ESP32-S3FN8 + UC6580), replacing the generic ESP32 carrier and HGLRC M100 PRO B101.
- Heading/motion sensor: GY-91-style MPU-9250 + BMP280, replacing separate QMC5883L and MPU6050 modules. MPU-9250 includes accelerometer, gyroscope and magnetometer; BMP280 is atmospheric pressure/temperature only.
- External nRF24L01+ PA/LNA remains the selected radio; Tracker SX1262 LoRa is not used for the command link.
- This release updates architecture documentation and scaffold comments only. Firmware remains non-operational; no sensor, GNSS, radio or actuator drivers have been implemented.
- See `docs/HARDWARE_SELECTION_VER003.md` for integration cautions and verification requirements.


## Hardware update — ver004

- Modular architecture: central carrier PCB with removable cables; no plug-in backplane.
- Selected separate motor driver: Cytron MDD20A dual-channel board, one channel per 550 motor. It is not mounted on the 77 × 79 mm carrier PCB.
- See `docs/CARRIER_PCB_CONNECTION_PLAN_VER004.md` for proposed logical connectors, power domains and verification gates.
- This release adds documentation only; firmware remains non-actuating and no GPIO assignments are approved.
