# R77 — Dual-radio link watchdog: stop all on total loss, resume on recovery

**Date:** 2026-11-22  
**Owner instruction:** add no new physical elements. Use the already selected nRF24 primary + E22-900T22S backup radio and the existing Boat ESP32-S3 firmware.  
**Implementation type:** software link watchdog and actuator command policy. **This is not an independent hardware safety controller.**

## 1. Required behavior implemented

| Radio state | Firmware behavior |
|---|---|
| Authenticated fresh nRF24 primary | Normal manual control, autonomous navigation and eligible release commands |
| nRF24 stale, E22 backup fresh | Manual/stick control is cancelled; autonomous navigation may continue; hopper/hook release remains primary-only |
| Both links stale | Motor requests become zero; EKM014 request becomes all-off (releaseChannel = -1); active mission is held, not diverted Home |
| Authenticated link returns | Held autonomous navigation resumes from its retained mission phase |
| Primary returns after backup-only operation | Manual control requires neutral joystick then a new fresh command; no old stick value is replayed |
| Primary lost during release intent/pulse | Abort the release transaction; if a pulse was active, command OFF and latch STOP; never replay the release after reconnect |
| Explicit user-requested Return Home | Still supported when the existing local mission and safety conditions allow it |

## 2. Freshness and timing used by the existing radio manager

- Primary nRF24 freshness: <=250 ms since the last accepted authenticated frame.
- Backup E22 freshness: <=6000 ms since the last accepted authenticated frame.
- Mission manual-stick lease: <200 ms.
- Radio session inactivity/re-handshake threshold: 6500 ms.

The software watchdog is alive when **either** authenticated link is fresh. Total link loss is declared only when both freshness checks fail. The 6-second backup freshness window reflects the current low-rate backup link; it is not a certified safety response time. Manual steering remains primary-only.

## 3. Recovery behavior

For an outbound, returning or paused autonomous mission, a complete radio outage changes the output request to STOP while preserving the mission phase in RAM. Ordinary RF inactivity no longer tears down the authenticated session: both endpoints retain the existing session key and replay counters, allowing a fresh authenticated nRF24 or E22 frame to restore the same session even after a long outage. A power-cycle or sequence-limit rekey still uses the existing primary nRF24 handshake. Eligible mission phase is preserved across sequence rekey and resumes only after a valid fresh authenticated frame arrives.

The following are deliberately not replayed:
- queued primary-link commands from a stale link;
- queued non-emergency actions after total radio loss;
- old manual joystick values;
- a release transaction that was in its execution window when primary authority was lost.

An emergency STOP already received into the queue is retained during queue purging.

## 4. Code changed

- Boat/source/boat_radio_safety.h and .cpp — BoatRadioWatchdog transition state and STOP_ALL decision; no automatic RTH on radio loss.
- Boat/source/boat_dual_radio_port.h and .cpp — watches authenticated freshness across both links; purges stale primary actions; preserves eligible mission state across link/session recovery; cancels temporary manual override on primary loss.
- Boat/source/boat_command_queue.h and .cpp — discard commands from a lost link and purge non-emergency actions while preserving an already queued emergency STOP.
- Boat/source/mission_controller.h — total loss holds navigation; primary loss during a release intent aborts it; temporary override yields back to the pre-override autonomous phase.
- Boat/source/mission_execution.h — no-radio hold, backup-only navigation, primary-only release authority, and no release replay after primary loss.
- Boat/source/boat_hardware_supervisor.h — output-off request on total link loss; invalidate stale manual control; latch an interrupted release.
- Boat/source/boat_application_port.cpp — normal session re-handshake no longer permanently disables the physical-driver adapter; an in-flight release remains a latched stop.
- Remote/source/boat_radio.cpp — ordinary link inactivity no longer resets the authenticated session, allowing backup-only recovery after an extended RF outage.
- Remote/tests/dual_radio_manager_r62_test.cpp — backup-link recovery after a prolonged no-link state.

## 5. Tests added/updated

- boat_radio_safety_test.cpp: watchdog LOST/RESTORED edges; STOP_ALL on total loss; backup freshness keeps the link watchdog alive.
- mission_link_loss_test.cpp: total loss holds mission, link recovery resumes it, backup-only navigation, primary-only release, no release replay, explicit Home still works.
- mission_execution_test.cpp: backup-only navigation, target wait, resume on primary recovery, abort on primary loss during release.
- mission_controller_test.cpp: lost joystick override returns to autonomous phase and does not replay stale sticks.
- boat_hardware_supervisor_test.cpp: all outputs off on total loss, neutral-before-manual recovery, backup-only navigation, recovery.
- boat_command_queue_test.cpp: stale link commands removed while emergency STOP is preserved.

The host workflow is the acceptance gate for these software changes. A host pass validates decision logic only; it does not test RF range, actual output voltages, contactors, MOSFET failure or motor/coil power.

## 6. Critical limitation — independent hardware safety is still NOT implemented

This request adds no new physical parts, so the watchdog runs on the same main ESP32-S3 that runs the application. It cannot independently interrupt power if:
- the ESP32 freezes, crashes or loses its clock;
- a GPIO remains high or is shorted;
- the MDD20A or an EKM014 MOSFET fails short;
- the driver HAL or output wiring fails;
- the load-power disconnect is welded or absent.

Therefore this R77 change implements the requested **radio-loss watchdog behavior**, not the complete independent hardware safety architecture. The R65/R66/R17 physical requirements (independent STOP/load isolation, feedback, watchdog and bounded one-hot pulse hardware/current-zero) remain open. Do not connect live motors or solenoids or claim independent hardware safety on the strength of this firmware change.

## 7. Release and commissioning status

- No new hardware elements added; no GPIO assignments or BOM changes.
- No radio, motor or solenoid hardware was physically tested.
- Boat VER014 and Remote VER028 remain unchanged; official releases remain withheld pending target and physical acceptance.
- Next: verify host workflow and diagnostic target compile, then bench-test radio timeout/recovery with dummy loads only after the physical output interface is commissioned.
