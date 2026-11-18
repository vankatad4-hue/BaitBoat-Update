# Flytec V080 Boat Software — Project Master Status

## 1. Project scope
Retain the Flytec V080 hull and existing propulsion hardware while replacing the original boat control electronics with a custom central controller PCB. The remote controller is a separate software project.

## 2. Agreed hardware and requirements

### Retained hardware
- Existing Flytec V080 hull.
- Two standard 550 brushed DC motors, intended 12 V operation.
- Existing battery indicator and main power switch.
- Existing battery pack reported as 3S4P Li-ion, 12 V nominal, with a 20 A BMS. Reported capacity has varied; verify the actual pack label and cell configuration before final design.

### New control architecture
- One central custom PCB with removable connectors.
- ESP32 mounted as a removable carrier/module.
- Available PCB envelope: approximately 77 × 79 × 14 mm.
- Compartment floor-to-lid clearance: approximately 30 mm.
- DHT11 is explicitly excluded.

### Release actuators
- Bait solenoid: FW0630 009 DC12V.
- Hook solenoids: FW0630 001 DC12V.
- Coil current, resistance, duty cycle, and thermal limits are not yet verified. Do not select driver ratings from part numbers alone.

### Navigation and radio
- **Selected main controller/GNSS board: Heltec Wireless Tracker V1.2** (ESP32-S3FN8 + UC6580 GNSS), replacing the generic ESP32 carrier and separate HGLRC M100 PRO B101 GNSS module. Confirm exact V1.2 schematic, pin map, GNSS power control, UART routing and antenna configuration before wiring.
- **Selected heading/motion sensor: GY-91-style MPU-9250 + BMP280 module.** MPU-9250 integrates 3-axis accelerometer, gyroscope and magnetometer, replacing the separate QMC5883L compass and MPU6050 proposal. BMP280 is for atmospheric pressure/temperature, not underwater depth. Verify exact breakout revision, chip markings, supply range, logic levels and I²C address.
- Prefer I²C for initial sensor integration. No GPIO assignments are approved until the exact Tracker V1.2 pin map and peripheral conflicts are checked. Calibrate the magnetometer after installation and mount away from motors, motor leads, solenoids, high-current wiring and steel.
- The Tracker also includes SX1262 LoRa; leave it unused. The selected boat/remote command and telemetry link remains the external nRF24L01+ PA/LNA module.
- **Selected radio:** nRF24L01+ PA/LNA module on the boat, paired with the same class of module in the remote; 2.4 GHz band. Initial test data rate: 250 kbps.
- Desired radio range: up to 500 m in mixed conditions; this remains an unverified test target, not a guaranteed range.
- Radio connects to the ESP32 by SPI (SCK, MOSI, MISO, CE, CSN; IRQ optional). Exact GPIOs remain unassigned until the final ESP32 carrier/PCB pin map is checked.
- Provide a dedicated, low-noise 3.3 V supply sized for PA/LNA transmit-current peaks; do not assume the ESP32 board regulator can supply it.
- Verify exact module RF front-end, antenna, channel, output power and applicable Bulgarian/EU 2.4 GHz EIRP requirements.
- On radio loss during autopilot, desired response is return-to-home only when home position, GNSS, heading, and control health are valid. Otherwise the boat-side safety policy must enter a separately defined safe state.

### Mission workflow
1. Manually drive to a location.
2. Save it as a waypoint.
3. Select a saved waypoint later.
4. Navigate there autonomously.
5. Operate bait and hook release actuators.
6. Return home.
7. Allow manual control during autopilot using a selectable behavior: temporary manual override/resume, or switch fully to manual.

### Sonar
- Intended architecture: ready-made CHIRP sonar module.
- Full echogram and fish arches should be shown on the remote's existing 4.3-inch display.
- Sonar data path, module model, electrical interface, protocol, and boat-side driver are not selected yet.

## 3. Critical measurements still required
- Motor running and stall current, measured safely.
- Solenoid coil resistance/current and permitted activation time.
- Exact battery voltage range, capacity, BMS limits, and wiring.
- ESP32 module/board exact model and pin availability.
- GPS/compass connector pinout and voltage.
- Exact nRF24L01+ PA/LNA board variant, antenna, channel and legal regional EIRP configuration.
- PCB mounting-hole locations and mechanical clearances.
- Sonar module model and interface.

## 4. Software status
No verified boat-side firmware implementation is included in this initial archive. The `firmware/` directory contains only a safe, non-actuating starter sketch. No motor or solenoid outputs are configured.

Not yet implemented or verified:
- GNSS UART parser and compass driver.
- nRF24L01+ PA/LNA driver, packet protocol, pairing, application-level authentication/replay protection, and telemetry decoder.
- Motor PWM/direction control and current/thermal protection.
- Solenoid drivers and activation interlocks.
- Waypoint storage and navigation controller.
- Return-to-home and radio-loss behavior.
- Sonar interface and telemetry.
- Watchdogs, emergency-stop handling, and boat-side failsafe state machine.
- Hardware-in-the-loop and water testing.

## 5. Safety principles
- The boat, not the remote, must enforce actuator safety and radio-loss behavior.
- A stale or invalid command must never keep motors running indefinitely.
- Emergency STOP must have priority over all other commands.
- GNSS loss, invalid heading, low battery, overcurrent, watchdog reset, and radio loss require explicit, tested handling.
- Never activate motors or solenoids during initial firmware bring-up.
- Validate drivers with current-limited bench supplies and dummy loads before connecting real actuators.
- Do not test autonomous navigation on water until all safety gates and recovery behavior are independently validated.

## 6. Versioning
All boat software release archives use `Flytec_Boat_verXXX.zip`. Start at `ver001` and increment the number by one for every modification. Keep all older ZIPs unchanged in the `Flytec_Boat/` Library folder.


## 7. Radio decision update — 2026-11-16

The user selected nRF24L01+ PA/LNA modules on both the boat and remote. Use 250 kbps as the initial test setting. Hardware pin assignments, power supply, antenna/channel, RF24 library version, packet format, security, acknowledgements, heartbeat, watchdog and measured 500 m performance remain unimplemented/unverified. See `docs/RADIO_LINK_DESIGN.md`. The radio must not be treated as operational until bench and range testing are complete.


## Hardware selection update — ver003 (2026-11-16)

See `HARDWARE_SELECTION_VER003.md`. Heltec Wireless Tracker V1.2 is selected as the main processor/GNSS board; GY-91-style MPU-9250 + BMP280 is selected for heading/motion/barometric sensing. This supersedes HGLRC M100 PRO B101 + QMC5883L and the separate MPU6050 proposal. nRF24L01+ PA/LNA remains the selected external command radio. This is not an implementation of firmware drivers.


## Modular hardware update — ver004 (2026-11-17)

The user approved a central carrier PCB with removable cables for both boat and remote. The boat carrier is a low-voltage interconnect hub; high-current motor and solenoid paths remain on separate protected harnesses/driver boards. The selected Cytron MDD20A dual-channel motor driver is a separate replaceable board, one channel per motor. See `CARRIER_PCB_CONNECTION_PLAN_VER004.md`.

The connector schedule is logical only. No Heltec GPIO numbers are approved. Confirm the exact Wireless Tracker V1.2 pin map, GY-91 breakout, MDD20A revision/control truth table, radio module, motor current, solenoid current, battery/BMS and hardware-stop design before PCB release. The 20 A BMS is a system constraint and the MDD20A channel ratings do not imply the battery can supply 40 A.

This is a documentation-only update. No motor, solenoid, radio, GNSS, IMU or failsafe firmware has been implemented. The boat sketch remains non-actuating.
