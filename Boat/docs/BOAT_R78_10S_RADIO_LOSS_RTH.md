# Boat R78 — 10-second dual-radio-loss Return to Home

Date: 2026-11-22  
Branch: `hardware-ready-dual-radio-r1`  
Status: software implementation added; host and ESP32 target verification pending; physical acceptance not performed.

## Authoritative behavior

1. **Either authenticated radio is fresh:** retain the existing policy. The nRF24 primary is required for manual steering and all hopper/hook releases. The E22-900T22S backup can maintain the eligible autonomous mission, but cannot issue manual or irreversible release actions.
2. **Both radio paths become stale:** immediately request STOP for manual motors and every solenoid output. Discard stale joystick values and queued non-emergency actions. Do not replay a release after reconnection.
3. **10-second threshold:** measure 10,000 ms from the last accepted authenticated frame received on either radio. The timer is wrap-safe. A single fresh radio resets the loss condition.
4. **At or beyond 10 seconds:** initiate RTH only if all of the following are true:
   - an already-started autonomous mission is in a return-eligible phase;
   - the Boat has a CRC-valid Home record in its own NVS and it matches the Home coordinate in the active mission plan;
   - current GNSS, heading, battery and geofence/navigation data are valid and fresh;
   - the physical STOP/watchdog/feedback sample is healthy, release interlocks are valid, and all solenoid channels are idle;
   - the mission executor can safely divert to Home.
5. **No valid Home or any failed gate:** remain STOP. No new mission is created and no motor output is permitted. A later safe navigation sample may allow the still-active mission to attempt the Home diversion while the outage remains active.
6. **During detached RTH:** only the local `INTENT_NAV_HOME` motor-navigation intent is allowed without radio. Every hopper/hook output remains OFF. If GNSS, heading, battery, geofence or a physical safety input becomes invalid, the executor stops/aborts and the actuator supervisor returns all outputs OFF.
7. **Recovery before 10 seconds:** an authenticated restored link clears the outage hold and the eligible held mission resumes. Stale stick values are not replayed; manual control still requires fresh neutral input.
8. **Recovery after RTH has started:** do not resume the old outbound route. RTH continues to Home. An explicit primary-radio `MCTRL_SWITCH_MANUAL` may take over only after a fresh neutral stick sample and stationary/idle interlocks. Backup E22 cannot authorize this takeover. Emergency STOP remains available.
9. **At Home:** stop motors and keep all solenoids OFF. Do not automatically restart the old mission.
10. **Power-cycle:** Home persists in Boat NVS; the active mission does not. A reboot therefore never resumes propulsion automatically.

## Home NVS

The Remote retains its Home point in its own persistent storage. During authenticated Boat mission preparation, the Home coordinate from the prepared mission plan is copied into the Boat ESP32's internal NVS. The Boat uses a two-slot record with explicit little-endian serialization, version, sequence counter and CRC32. It writes the inactive slot, reads it back and verifies it before acknowledging mission preparation. A corrupt/missing record or failed read-back rejects preparation and blocks automatic RTH.

No additional hardware is introduced for this behavior; the storage uses the selected ESP32-S3's internal NVS.

## Implementation

- `Boat/source/boat_home_point_record.h` — portable Home record and CRC validation.
- `Boat/source/boat_home_point_nvs.h/.cpp` — two-slot Preferences/NVS adapter.
- `Boat/source/boat_dual_radio_port.cpp` — authenticated RX timer, Home read-back, timeout and RTH transition.
- `Boat/source/mission_execution.h` — detached Home-only navigation exception.
- `Boat/source/mission_controller.h` — RTH eligibility and safe manual takeover.
- `Boat/source/boat_hardware_supervisor.h` — no-radio exception restricted to local Home navigation.
- `Boat/source/boat_application_port.cpp` — passes local RTH authorization to the actuator supervisor.

## Safety boundary

This remains a **software watchdog on the same ESP32-S3**. It is not a physically independent safety channel and cannot guarantee a cutoff if the MCU locks up, its power rail fails, or a driver output transistor fails. The user's no-new-hardware constraint is preserved: no separate PLC, contactor, watchdog module or custom PCB has been added. Actual motor and solenoid power must remain disconnected until the dummy-load bench procedure and physical wiring checks are complete.
