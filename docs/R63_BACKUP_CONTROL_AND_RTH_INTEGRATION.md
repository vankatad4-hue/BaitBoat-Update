# R63 — Safe backup LoRa policy and end-to-end active-mission Return Home

**Date:** 2026-11-22. **Status:** non-actuating development and host tests; physical commissioning pending.

## Root cause and change
- The Remote's E22 LoRa link is paced around **one application frame every 3 s** to remain inside a preliminary EU868 duty budget. The Boat software accepts manual motor control only with a **~200 ms local input lease**, and its physical driver expects at most **100 ms output lease**. A 3 s backup steering frame is not a continuous or demonstrably safe actuator command stream.
- The R62 backup policy still permitted `CMD_MANUAL_MOTORS`. R63 now **rejects manual motors on LoRa** at the Remote link manager, Remote radio transport, both Remote/Boat authenticated command decoders, and Boat independent policy. The regular primary nRF24 manual path is unchanged.
- On loss of authenticated primary contact, the Boat link failsafe, mission controller and pure mission execution supervisor no longer treat an alive LoRa heartbeat as continuous mission supervision. If a **previously started and locally verified mission** has a valid Home, GNSS, heading, battery and geofence, software requests an irreversible safe RTH diversion; otherwise propulsion must stop. Manual joystick commands on the backup remain inhibited.
- Backup E22 remains able to carry **emergency STOP** and **RETURN_HOME request for an existing active mission**. This cannot manufacture a new Home destination or start a mission, release a hopper or clear STOP. Independent wired emergency STOP remains the only physical STOP, especially under radio delay or outage.

## Fixed missing Boat application path
- Before R63, a validated `CMD_SET_MODE / MODE_RETURN_HOME` could be queued/ACKed but ignored by `BoatApplicationPort::tick()`. Added explicit handling in `Boat/source/boat_application_port.cpp`.
- Added `BoatDualRadioPort::requestReturnHome()` that validates active mission phase, local fresh trusted safety/navigation inputs, current STOP/release-idle and authenticated link. Calls existing `BoatMissionController::divertHome` and immediately requests STOP while transitioning to Home. It does not create or activate a mission without a previously started plan.
- Updated the protected command filter to permit only this return intent inside an active mission; replies now use the **originating radio**, rather than silently attempting nRF24 for every action.
- The Remote's `boatCommands_setMode(MODE_RETURN_HOME)` can now submit the request while the last confirmed mission phase is active. Boat acceptance is authoritative; transport submission is **not** evidence of successful RTH.
- Manual/autopilot/release and override commands still require a healthy authenticated primary and all existing safety gates.
- The Home request can be delayed or return unknown due to LoRa duty restrictions or lack of link; no automatic retries may violate airtime limits. Local primary-loss diversion must work without Remote delivery when a safe prior mission exists.

## Regression coverage
- Remote `Remote/tests/dual_radio_manager_r62_test.cpp`: LoRa manual/release rejection, STOP and Home allowed, stale packet rejection, primary recovery and primary manual unchanged.
- Remote `Remote/tests/radio_command_backup_r63_test.cpp`: actual Remote command codec denies backup steering/auto/release and accepts STOP + Home. Added to `remote-host-tests.yml`.
- Boat `Boat/tests/radio_command_codec_test.cpp`, `boat_command_policy_test.cpp`, `boat_radio_safety_test.cpp`, `mission_link_loss_test.cpp`, `mission_execution_test.cpp`: symmetric backup policy, no new Home plan from backup, primary loss despite backup.
- Boat `Boat/tests/boat_application_integration_test.cpp` and `application_stubs/boat_dual_radio_port.h`: test physical STOP denies, no active mission rejects, active mission permits only validated Home, ACK targets originating backup; still **simulated**, not an RF hardware test.

## Open blockers
No radio device has been purchased or paired. No independent STOP/watchdog/DC-rated cutoffs, hardware one-hot nonretriggerable coil limiter, current feedback, motor stall tests or closed-loop GPS water acceptance. Selected endpoints remain Boat ESP32-S3-DevKitC-1-N32R16V and Remote Waveshare ESP32-S3-Touch-LCD-4.3 Standard, with modular ready-made parts only. Remote SD stores maps/waypoints; Boat NVS stores calibration. Sonar undecided and not implemented.

**Important:** LoRa RETURN_HOME is **mission diversion only**, not a new autonomous trip from MANUAL. The Boat's physical-driver HAL still has no commissioned actuator pins. Official Boat VER014 / Remote VER028 stay frozen. CI should be checked on the latest source commits before signoff.

## Verification recorded
- **PASS** Remote portable host tests [44991125196](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44991125196) with R63 backup decoder/manager regression.
- **PASS** Boat portable host tests [44991241882](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44991241882) covering the cross-radio policy, primary-loss divergence, previously unhandled Home command, fail-closed no-mission behavior and original-link ACK.
- An initial new application test failed because the fake command lacked `MODE_RETURN_HOME`; after fixing it, a later test failure came from counting the deliberately rejected pre-mission request. Both test-fixture errors were corrected, and the current host suite passed. No actuator hardware was enabled.
- **Target compiles for latest R63 Boat/Remote source:** scheduled at this review; check their eventual Actions conclusions, do not claim success from older R62 binaries. These are diagnostic/Remote software compiles, not physical safety acceptance.
