# Flytec V080 Boat Project — Master Status R4

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
- Boat radio port is not integrated into main application. Session echo, pending-command deduplication and queue-level ACK scaffolds exist; no actuator output driver, GNSS/IMU driver, mission/autopilot, telemetry source, EXECUTED ACK or failsafe execution.
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
- Current development snapshots: Remote ver023 / Boat ver014. Boat ver014 checksum and archive structure independently verified; Remote ver023 binary remains unverified.
- See repository `docs/ZIP_RELEASE_POLICY_R1.md`.

Do not connect motor/solenoid power or operate the boat.


## Shared payload decoder mirror (2026-11-20)
- Added portable HEARTBEAT/ACK/TELEMETRY payload validator files mirrored with Remote source.
- Boat payload decoder/encoder remain portable utilities; telemetry generation and EXECUTED application ACK are not implemented. Queue-level ACCEPTED/DUPLICATE/BUSY ACK scaffold exists. No compile/hardware validation of endpoint firmware.


## Remote integration state note (2026-11-20)
Remote decodes authenticated telemetry/ACK and applies freshness/safety gates. Boat now has queue-level ACK and session echo scaffolds, but does not emit application telemetry or EXECUTED ACKs and has no command executor. Payload codec/encoder are portable utilities. Do not connect actuators.


## Boat payload encoder milestone (2026-11-20)
- Added portable wire encoders for HEARTBEAT, ACK, and all six telemetry payload subtypes.
- Added host tests for valid round-trips and malformed/range inputs. These tests are protocol-only and do not validate radio frames or hardware.
- Boat ver009 notes record this addition. No ESP32 compile or device testing.

## Boat session/ACK scaffold milestone (ver010)
- Boat now uses the remote session epoch, replies to SESSION_START, and tracks primary/backup sequences separately.
- Queue deduplicates pending command IDs across radios; queued non-manual commands receive ACCEPTED, executed duplicates DUPLICATE, queue overflow BUSY.
- ACCEPTED does not imply execution. No command application, interlocks, telemetry sender or actuator path.
- Review: BOAT_ACK_SESSION_INTEGRATION_R1.md. Source is not compiled or tested on ESP32.

## Boat ver014 fail-closed command policy milestone (2026-11-20)
- Added portable command policy scaffold with boot STOP latch, link-specific freshness, mode prerequisite checks, steering limits and independent release interlocks.
- Release requests are not outputs; no driver is connected. CI workflow now includes policy tests, but run status not yet verified.

## Boat ver013 host-test build cleanup milestone (2026-11-20)
- Removed duplicate encoder definitions identified during review of the host test build command.
- Added CI workflow to compile/run the portable Boat host tests with warnings-as-errors. Run result is not yet observed.

## Boat ver012 queue/ACK integration scaffold milestone (2026-11-20)
- Extracted bounded four-entry command queue into portable utility; BoatDualRadioPort now delegates enqueue/dequeue/pending lookup/session clear to it.
- Implemented missing `commandPending()` and authenticated ACK frame sender `sendCommandAck()`; response frame uses matching command ID/type and same receiving radio. STOP/interlock fields remain unknown placeholders.
- Added queue FIFO/full/wrap/epoch test. Test sources committed but not executed; network checkout unavailable.

## Boat ver011 protocol test milestone (2026-11-20)
- Cross-radio command replay classification is now explicit and unit-testable. A stale sequence/new command ID is dropped; only exact pending/executed ID duplicates can receive duplicate/pending acknowledgements.
- Added host tests for cross-radio classification, deduplication cache scope and eviction, failsafe decision logic, sequence wrap/stale handling, and backup command policy/throttle limit.
- Test source and host stubs are committed, but tests were **not run** because network checkout was unavailable. No compile claim is made.

## Package revision status
Current package markers: Remote ver023 / Boat ver014. Boat ver014 ZIP checksum and structure independently verified; Remote ver023 binary remains unverified. Boat ver014 ZIP checksum and structure independently verified; Remote ver023 binary remains unverified. Deterministic packaging workflow is in place. Boat payload decoder/encoder are portable utilities. Session echo, queue-level ACK and duplicate handling are scaffolded, but no telemetry source, application command executor, EXECUTED ACK or actuator integration exists.


## Continuation checkpoint synchronization — 2026-11-20
- Canonical restart guide: `../../docs/CONTINUATION_CHECKPOINT_VER001.md`.
- Remote UI changes and current integration boundaries are captured there. Boat remains scaffold-only: no command executor, actuator output, telemetry publisher, mission/autopilot, or failsafe execution.
- Package marker Boat 014 is not evidence of an operational release; current branch archive is unverified. No target build/hardware validation.


## Source audit / compile preparation — 2026-11-20
- Boat dual-radio source was reviewed with Remote session-start retry logic.
- Boat now responds idempotently to repeated authenticated same-session markers, preventing a lost reply from blocking peer session confirmation.
- This correction is not compiled or integration-tested. Secure key provisioning remains disabled/fail-closed.
- Boat receive/queue/ACK integration is not covered by current portable tests; the sketch remains a non-operational placeholder with no command executor, telemetry, failsafe execution or actuator outputs.
- Build manifest/dependency lock, exact controller SKU/pins, target compile and RF/hardware validation remain blockers.
- Source audit report: `docs/SOURCE_CONSISTENCY_AUDIT_R1.md`; changelog VER022.


## Shared project compile-preparation update — 2026-11-20
- Remote target compile run #11 progressed to C++ compilation but failed on Remote UI/map sources. Remote fixes are committed and awaiting retry; this does not change Boat's status.
- Boat firmware remains uncompiled, unintegrated and non-operational; no actuator output, telemetry publisher, executor, or failsafe execution. Continue to keep power loads disconnected.


## Cross-project compile status note
Remote Target Compile #14 failed on a Remote offline-map source declaration (`g_map_attribution`), corrected in commit `9aa20b0d987f94ea5b49e46509ae0ba10cc9d36b`. Compile #15 pending. This does not change Boat firmware status: Boat remains uncompiled and non-operational.


## Compile status update — 2026-11-20
Remote CI target compile run #19 (ID 44781879101) succeeded on commit 3f189cb1ac65a764aec678893f1df857d113c815, including compile/link and firmware artifact upload. ESP32-S3 16MB/OPI PSRAM huge_app configuration; 1,430,957-byte sketch (45%), 178,136-byte globals (54%, 149,544 bytes remaining); no C++ errors/warnings reported. Boat remains uncompiled and non-operational. This is not hardware validation. Next work: audit and test Remote runtime lifecycle, map import validation, renderer allocation/cache and settings Wi-Fi timers; keep all outputs disconnected.


## Eight-lake development checkpoint — 2026-11-20
Remote package marker remains 028; Boat 014. Source now has eight SD lake profiles, sixteen mission slots and separate Home per profile, checked active selection, recoverable name and mission records, and A/B offline map installation. Lake selection UI uses standard LVGL keyboard and confirmed STOP/manual/connected-radio/inactive-motor gates. Mission execution remains blocked; boat-side synchronization is not implemented.
Latest observed before this update: host #41 passed (6a3d88c), packaging #977 passed, target #42 running. Host #41 did not exercise the new SD storage. This update adds a mock SD host test compiling actual lake_storage.cpp: eight profiles, safe switching, name persistence, isolated paths, map marker rename failure, short writes and unavailable card. Results pending. The mock does not test actual FAT/SD durability, mission serialization or LVGL. Verify new CI and fix real failures before bumping the release marker. Hardware remains untested.


## Verified test handoff — 2026-11-20
Source SHA 8030ecab6cbccb64204e638b4c398b588681515e. Host #43 failed on missing sdMissionAvailable reference after extraction; corrective commit 8030eca replaced it with lakeStorage_ready. Host #44 passed (run 44793207359), including actual mission serialization for 128 waypoints, per-lake Home and CRC/rename/short-write simulation. Packaging #980 passed (44793207717). Target compile #45 (44793207383) is running; previous #43/#44 were cancelled by newer pushes, not passed. Do not change firmware merely to retrigger CI while the current target run is active.
Next: inspect #45 compiler result and fix actual diagnostic if any; extend coverage/import parser and fault-injection tests, audit remaining source modules in manageable batches (repository inspection is not an exhaustive line-by-line audit), validate UI lifecycle and SD runtime, then bump Remote package marker 028 to 029 only after software gates pass. Boat remains 014 and hardware/actuation unvalidated. Repository project marker docs/PROJECT.md is not a ChatGPT Project. Existing root legacy map and NVS-only legacy data require follow-up; standard keyboard is not a dedicated Bulgarian keyboard.
