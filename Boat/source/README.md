# Boat Source — Current Development Status

Updated 2026-11-20. The repository is canonical; this is not operational firmware.

## Release naming
- ZIP: `Flytec_Boat_verXXX.zip`
- Preserve all older releases.
- Package version marker: `releases/VERSION` (currently `014`).
- Increment by one for every modification included in a distributed package; see root `docs/ZIP_RELEASE_POLICY_R1.md`.

## Current controller and radios
- Controller/GNSS candidate: Heltec Wireless Tracker ESP32-S3 + UC6580; exact board revision is unknown.
- External primary nRF24L01+ PA/LNA; backup EBYTE E22-900T22S UART module matched to remote.
- Onboard Tracker SX1262 is unused for initial dual-radio pair.
- All boat radio GPIO macros remain disabled (-1) until the pin audit is completed. See `../docs/BOAT_CONTROLLER_PIN_AUDIT_R1.md`.

## Source inventory
- `nrf24_link.*`: nRF24 transport scaffold, fixed 32-byte frame, RX queue, non-blocking TX.
- `e22_uart_link.*`: E22 config/readback, transparent UART frame parser, duty limiter.
- `boat_dual_radio_port.*`: dual-link polling, command restrictions and bounded receive/command queue.
- `radio_protocol.*`: CRC and HMAC frame format.
- `radio_command_codec.*`: command encode/decode and policy checks.
- `radio_key_store.*`: key-store scaffold; deliberately fails closed without key provision.
- `boat_radio_safety.*`: duplicate-action cache and failsafe decision helper only.

## Bench test sketches
- `hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `hardware_test/E22_UART_Bench/E22_UART_Bench.ino`
- Both disabled until exact pin assignments are set. Fixed diagnostic frames only; no actuators.
- See `hardware_test/README.md`.

## Boat ACK/session integration milestone (ver010)
- Boat echoes the remote session epoch and sends session-start replies on configured radios.
- Independent primary/backup sequence guards; command-ID deduplication handles cross-radio retries.
- Pending command IDs are deduplicated; non-manual queue results return ACCEPTED/DUPLICATE/BUSY ACKs.
- ACCEPTED is only queue receipt. No application consumes the queue and no EXECUTED ACK is generated.
- Review: `../docs/BOAT_ACK_SESSION_INTEGRATION_R1.md`.

## Portable payload encoder milestone (ver009)
- Added encoder counterparts for the existing eight-byte HEARTBEAT, ACK and TELEMETRY payload contract.
- Host round-trip/invalid-field tests cover status, battery/speed, GNSS coordinates, navigation, sonar summary, heartbeat and ACK. Run instructions: `tests/README.md`.
- These utilities are not connected to the boat receive queue, telemetry sensor source, ACK dispatch, or command execution.

## Not implemented or validated
- No main boat application invokes the radio port.
- Queue-level ACKs are scaffolded (ACCEPTED/DUPLICATE/BUSY); no EXECUTED ACK, command executor, telemetry source, motor PWM/DIR, solenoid driver/interlock, GNSS/IMU driver, waypoint storage, autopilot, return-home/watchdog implementation.
- No source compile, hardware bring-up, RF/antenna range, ERC/DRC or water testing has been completed.

## Electrical/mechanical gates
- Battery is reported 3S4P / 20A BMS but must be confirmed.
- 2×550 motor current/stall measurement and solenoid coil resistance/current/thermal data are missing.
- Cytron MDD20A candidate does not fit the preliminary carrier footprint; separate placement or redesign required.
- High-current actuator power stays disconnected during radio bench work.

## Canonical docs
- `../docs/PROJECT_MASTER_STATUS.md`
- `../docs/BOAT_CONTROLLER_PIN_AUDIT_R1.md`
- `../docs/BOAT_RADIO_PIN_ASSIGNMENTS_R3.md`
- `../docs/BOAT_HARDWARE_TEST_BUILD_R2.md`
- `../docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md`

## Continuation checkpoint
Read `../../docs/CONTINUATION_CHECKPOINT_VER001.md` for project-wide current status and prioritized next work. Boat ver014 is a source-package marker only; the current branch ZIP is unverified. No boat target build, application executor, telemetry source, actuator or autopilot implementation is complete.

## R8 follow-up — integrated Heltec SX1262, 2026-11-21

Owner-selected architecture: one Heltec Wireless Tracker ESP32-S3, external Cytron MDD20A with native two-channel PWM/DIR, nRF24 primary, and **Heltec onboard SX1262 SPI LoRa** (Boat E22/E227 UART retired; no Boat SC16IS750). No RP2040. Four independent bait/hook solenoid drivers and separately latched physical watchdog/STOP power gate remain required. Sonar stays on hold: no chosen model, hardware, firmware or circuit.

New Boat source: heltec_sx1262_link.* and heltec_sx1262_airtime.h; transport construction now selects integrated LoRa. All GPIO remain -1, V1.2 board audit/remote PHY/regulatory sign-offs false: no radio TX, motor or solenoid activation authorized. RadioLib 7.3.0 pinned for compile attempts, host GPIO/airtime tests added. No compile evidence for this R8 source until Actions checks finish.

GPIO demand drops from historical R7 12 to **10 external native controls**; 11 optimistic provisional candidates yields a count-only pass, but conservative uncontested pool of five still fails. Onboard SX1262 pins 8–14 remain internally occupied, and shared nRF SPI routing/header access is not signed. No pin freeze.

**Critical Remote incompatibility:** Remote remains E22 UART via SC16IS750. Its embedded E22 transparent RF packetization is not automatically compatible with a raw SX1262 SPI radio. The Boat's LoRa backup is intentionally unavailable until a matching RF protocol/remote revision is validated. At SF9 BW125, estimated 32-byte airtime ~247ms; 3s heartbeat and 6s freshness are not certified compatible with 1% duty. Regulatory and reset-persistent airtime enforcement unverified.

Remaining R7 KiCad ERC: **15 violations**, not production-ready. High-current latch, four MOSFET/coil drivers, rails, footprints, PCB and physical testing still outstanding. Current release markers Remote 028 / Boat 014 retained; 029/015 withheld. See Boat/docs/BOAT_INTEGRATED_SX1262_R8.md and Boat/docs/BOAT_LORA_HARDWARE_DECISION_R4.md.

Commit evidence: source 975088133dbd8c4e9cb81d3760a5b65292f7426f; GPIO/CI/documentation 557d601b1a561fd0a908e90d24839e12b3ab3072.
