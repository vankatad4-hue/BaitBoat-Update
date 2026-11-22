> **SUPERSEDED RAIL ASSIGNMENT:** R20/R79 moved E22-900T22S VCC to the 5V DCDC6 main rail; D24V10F3 unit 1 now powers nRF24 and unit 2 powers sensors/I/O. Use [R20 GPIO/header map](BOAT_GPIO_HEADER_MAP_R20.md) and [R79 bench procedure](BOAT_R79_LOW_BATTERY_BENCH_TEST.md) as current authority. The R78 test sequence remains useful for radio-loss behavior only.

# Boat R78 — radio-loss and recovery bench test (dummy loads only)

Date: 2026-11-22  
Status: procedure prepared; physical test not run. Selected modules are still to be purchased/assembled.

## Purpose

Prove the software and module-output behavior for primary-radio loss, backup-radio loss, total loss, 10-second Home diversion and radio recovery **without connecting the actual 550 motors or any hopper/hook solenoids**.

## Hard bench rule

- Disconnect and physically isolate both real motors and all four real solenoids.
- Do not connect the 3S4P battery directly for first power-up. Use current-limited bench supplies with conservative limits.
- Use logic-analyzer/LED loads for first output checks. For power-stage checks, use appropriately rated resistive or indicator dummy loads, with current and thermal limits derived from the selected module specifications and measured supply current. Do not guess a high-current resistor value.
- Keep EKM014 solenoid supply output disabled until the logic and one-at-a-time tests pass.
- No dummy load is a substitute for a later physical acceptance test with the actual wiring, fuse and feedback chain.

## Required selected modules and rail assignments

| Function | Selected part | Bench check |
|---|---|---|
| Boat controller | ESP32-S3-DevKitC-1-N32R16V | Flash diagnostic firmware; verify flash/PSRAM match |
| GNSS/compass | HGLRC M100 PRO with QMC5883L | 5V supply, UART, fresh fix and compass telemetry |
| IMU/barometer | GY-91 (MPU-9250 + BMP280) | Dedicated 3.3V rail, I2C/SPI and sensor freshness |
| Primary RF | nRF24L01+ PA/LNA | Dedicated stable supply/decoupling and authenticated heartbeat |
| Backup RF | EBYTE E22-900T22S | Dedicated D24V10F3 3.3V rail; UART and authenticated heartbeat |
| Motor driver | Cytron MDD20A | Motor outputs connected only to rated dummy loads |
| Four solenoid channels | EKM014 | Logic and one-hot checks first; dummy loads only |
| Controller/GNSS 5V | Olimex DCDC6-16-TO5 | One converter; second remains optional pending measured noise/current |
| EKM014 logic 5V | TRACO TSR 2-2450 | Dedicated VL rail; verify polarity, current and temperature |
| 3.3V rail 1 | Pololu D24V10F3 #2830 | E22 supply |
| 3.3V rail 2 | Pololu D24V10F3 #2830 | GY-91 and the assigned low-voltage sensor/I/O branch |

The exact nRF24 supply/carrier/decoupling, EKM014 VH high-current wiring, motor/solenoid fuse values and final GPIO assignments remain acceptance items. Do not parallel regulator outputs. Use the common-ground arrangement from the signed wiring plan only; do not improvise grounds through RF or sensor cables.

## Stage A — visual and unpowered checks

1. Verify module part numbers and board revisions against the selection register.
2. Confirm all actual motor and solenoid connectors are physically unplugged and insulated.
3. Check battery polarity, converter input/output polarity, common-ground topology, connector keying and fuse-holder placement.
4. Confirm no motor/solenoid output pin is configured by the diagnostic firmware. The current firmware deliberately leaves actuator GPIO assignments disabled.
5. Confirm the selected ESP32-S3 module reports the expected N32R16V flash/PSRAM configuration.

## Stage B — power rails, without actuator loads

1. Power each rail separately from a current-limited bench supply.
2. Measure unloaded output voltage and polarity before connecting any module.
3. Connect the ESP32-S3 and M100 PRO to the DCDC6-16-TO5 rail; observe boot, GNSS acquisition and supply droop during radio transmission.
4. Connect E22 to its own D24V10F3 rail. Connect GY-91 and its assigned low-voltage sensor/I/O branch to the second D24V10F3 rail.
5. Power EKM014 logic VL from TSR 2-2450, with VH/solenoid supply and all channel loads disconnected.
6. Record input/output voltage, peak current, ripple, module temperature and ESP32 reset count during idle and simultaneous radio/GNSS activity.
7. Reject any rail that backfeeds another rail, exceeds the selected regulator's ratings, overheats, or causes brownout/reset.

## Stage C — logic-only radio and actuator tests

1. Flash the Boat diagnostic firmware and verify that the Home NVS namespace opens.
2. On the Remote, set Home from a fresh GNSS fix and confirm the mission UI reports Home valid in its SD-backed mission storage. Prepare a mission with that Home coordinate. Confirm the Boat writes a local NVS Home record, reads it back, validates CRC and sequence, and reports the matching coordinates.
3. Power-cycle the controller. Confirm the Home record remains valid but the mission does **not** auto-start.
4. With both radios connected, verify authenticated heartbeats and link freshness.
5. Disable nRF24 while E22 remains fresh: manual motor commands and all releases must be rejected; an eligible autonomous navigation mission may remain active.
6. Restore nRF24 and verify manual authority returns only after fresh authenticated primary frames and neutral-stick qualification.
7. Disable E22 while nRF24 remains fresh: normal primary operation continues.
8. Disable both radios. Use the last accepted authenticated frame timestamp as T0:
   - immediately after both links are classified stale: manual motor and all four solenoid outputs are OFF;
   - at T0 + 9,999 ms: outputs remain OFF;
   - at T0 + 10,000 ms: if Home NVS, navigation and all local safety gates are valid, the mission changes to Home navigation;
   - all solenoid outputs remain OFF throughout detached RTH.
9. Restore either radio before T0 + 10 seconds: the held mission may resume after authentication; no stale stick or queued release may replay.
10. Restore primary after RTH has started: RTH remains active. Send only a neutral primary stick sample, then an explicit manual-switch request while the boat is stationary and all release interlocks are idle. Verify the old outbound mission does not resume.
11. Send emergency STOP during RTH: all dummy motor outputs must go OFF immediately and remain latched until the normal local clear-STOP procedure.
12. Repeat with invalid/missing Home record, corrupted CRC, stale GNSS, invalid heading, unhealthy battery, invalid geofence, active release, physical STOP and failed feedback. Every case must remain STOP; no detached navigation or solenoid pulse is allowed.

## Stage D — power-stage dummy-load checks

1. Only after Stage C passes, connect rated dummy loads to MDD20A outputs. Keep current limit below the dummy-load and driver ratings.
2. Verify left/right motor channel polarity, direction, neutral output, STOP behavior, Home navigation response and output lease timeout.
3. Connect four separate low-current indicator/resistive dummy loads to EKM014 outputs with the selected logic rail. Verify correct channel mapping:
   - CH1 left bait hopper;
   - CH2 right bait hopper;
   - CH3 left hook;
   - CH4 right hook.
4. Verify exactly one channel can energize at a time; a second request is rejected while the first channel is active or awaiting idle feedback.
5. Verify pulse timeout, held-input non-retrigger, primary-link-loss abort and no release replay after recovery.
6. Record dummy-load current, driver temperature, supply droop and output-feedback state. Do not raise current beyond the safe dummy-load envelope to simulate unknown motor stall or coil current.

## Acceptance log

Record: firmware commit SHA, test date, radio link state/timestamps, T0 and T0+10s transitions, NVS Home sequence/CRC result, GNSS/heading validity, battery voltage, STOP/feedback state, each dummy output, current/voltage/temperature and pass/fail reason.

## Exit criteria before real motors/solenoids

- All software/host tests pass.
- ESP32-S3 Boat target diagnostic compile passes.
- Home NVS write/readback/power-cycle check passes on the actual board.
- Radio tests pass on the actual nRF24 and E22 modules.
- Dummy-load tests pass for both MDD20A channels and all four EKM014 channels.
- GPIO/header mapping, fuse values, wire gauges, connector pinouts, battery/BMS margin and physical STOP/feedback behavior are signed off.

**Current status:** the procedure is prepared; none of the physical stages above is claimed as completed. Actual motors and solenoids remain disconnected.
