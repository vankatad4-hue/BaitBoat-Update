# Flytec V080 Boat Project — Master Status R3

Updated 2026-11-20. Development only; not compiled, bench-validated, or operational.

## Scope and retained hardware
- Keep Flytec V080 hull, 2×550 brushed motors, battery indicator and master switch.
- Replace control electronics with central carrier PCB and removable connectors.
- Preliminary carrier envelope 77×79×14mm; lid clearance ~30mm. Mechanical fit not CAD-verified.
- Battery reported 3S4P Li-ion / 20A BMS; pack label, capacity, BMS and wiring must be confirmed.
- DHT11 is excluded.
- Bait solenoid FW0630 009 DC12V; hook solenoid FW0630 001 DC12V. Coil current/resistance, duty/thermal, flyback and driver sizing remain unknown.

## Selected architecture
- Main controller/GNSS candidate: Heltec Wireless Tracker ESP32-S3 + UC6580. Exact board revision is not physically identified.
- Heading/motion candidate: GY-91-style MPU-9250 + BMP280; chip markings, voltage, I2C and calibration need confirmation.
- Primary radio: nRF24L01+ PA/LNA, 2.4GHz, initial 250kbps.
- Backup radio: separate EBYTE E22-900T22S UART module at both boat and remote. Tracker integrated SX1262 is not used for initial pairing.
- Both radio antennas separate. Boat radio pin GPIOs remain disabled/TBD.
- Cytron MDD20A motor-driver candidate, separate from carrier; its 88.9×78.74mm outline conflicts with preliminary 77×79mm carrier envelope.
- 500m is a desired range, not demonstrated.

## Boat GPIO/resource audit
See [Controller Pin Audit R1](BOAT_CONTROLLER_PIN_AUDIT_R1.md) and [Radio Pin Assignments R3](BOAT_RADIO_PIN_ASSIGNMENTS_R3.md).
- Reserve integrated SX1262 SPI/NSS/reset/BUSY/DIO, GNSS UART and power control, TFT, USB/console, boot/strap and onboard power/ADC resources until exact board revision is confirmed.
- GPIO17/18 are only tentative UART candidates; mode/AUX pins are TBD.
- `boat_radio_hw_config.h` CE/CSN remain -1.
- Bench sketches must remain disabled until pin crosswalk/continuity audit is signed off.

## Radio profile (provisional bench profile)
- nRF24: channel 76, 250kbps, fixed 32-byte frame, CRC16, PA_LOW, auto-ack/retries.
- E22: UART 9600 8N1, channel18/868.125MHz, 62.5kbps air rate, 32-byte subpacket, 10dBm, transparent mode. Confirm exact variant and readback.
- Backup traffic restricted to safety/low-rate functions; no sonar, mission-start or bait/hook-release commands.
- Duty accounting is RAM-only and estimated airtime; no field operation pending RF compliance, persistent accounting and measured airtime.

## Software implementation status
Current code scaffold includes:
- nRF24 and E22 transports, frame CRC/HMAC codec and command policy.
- Boat dual-radio polling, receive validation and bounded command queue.
- duplicate-action cache and local failsafe decision helper.
- Key provisioning fails closed; no authenticated TX/control.
- Boat radio port is not integrated into main application. No actuator output driver, GNSS/IMU driver, mission/autopilot, telemetry/application ACK or failsafe execution.
- Source has not been compiled or hardware-tested.

## Safety gates
1. Identify exact Tracker hardware revision and matching official schematic.
2. Pin crosswalk, continuity and resource/strap/voltage audit; keep pin macros disabled.
3. Validate power supplies and current transients with dummy loads.
4. Measure motor running/stall current and solenoid coil electrical/thermal data.
5. Select/validate motor driver and MOSFET/flyback/fuse/thermal design.
6. Native schematic, ERC, footprints, PCB layout and DRC.
7. Secure key/session/replay, ACK, command executor and boat-side watchdog.
8. Test radio/antenna/RF compliance in Bulgaria/EU.
9. HIL and controlled water test only after independent failsafe review.

## Project documents
- Boat test build: `BOAT_HARDWARE_TEST_BUILD_R2.md`
- Electrical design: `BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md`
- Capture sheets: `BOAT_SCHEMATIC_CAPTURE_SHEETS_R2.md`
- Pin audit: `BOAT_CONTROLLER_PIN_AUDIT_R1.md`
- Pin table: `BOAT_HARDWARE_PIN_CONNECTIONS_R3.csv`
- Motor driver fit: `BOAT_MOTOR_DRIVER_FIT_REVIEW_R1.md`

## ZIP status
- Preserved ver004 archive; package marker is `releases/VERSION` = 005.
- Automated package artifacts and SHA256 are pending Actions verification.
- See repository `docs/ZIP_RELEASE_POLICY_R1.md`.

Do not connect motor/solenoid power or operate the boat.


## Shared payload decoder mirror (2026-11-20)
- Added portable HEARTBEAT/ACK/TELEMETRY payload validator files mirrored with Remote source.
- Decoder is currently unused by boat receive path; telemetry generation and application ACK remain unimplemented. No compile/hardware validation of endpoint firmware.


## Remote integration state note (2026-11-20)
Remote now decodes authenticated telemetry/ACK frames and applies freshness/safety gates. Boat firmware still does not emit these payloads or execute/ACK commands. Mirrored codec is currently a portable validator only on the boat side. Do not connect actuators.
