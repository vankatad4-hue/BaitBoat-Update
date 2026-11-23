# R30H-26 — persistent motor/coil output feedback after OFF and STOP

**Date:** 2026-11-23  
**Development branch:** `platformio-unified-r30h20`  
**Production code commit:** `4c42220311d49a2fac114ef318a6e137c623aa05`  
**Acceptance level:** logic-only non-actuating integration; no physically verified motor, valve or radio module.

## Safety gap and correction

Prior to R30H-26, `boatHardware_status` correctly encoded STOP as UNKNOWN if its local motor-active or solenoid release-not-idle feedback contradicted a STOP command. However the production application had no separate **persistent feedback contradiction latch** for a `BoatPhysicalDriver::apply(OFF)` that returned success while the actual local feedback continued to indicate energized output. This could leave an application alive after a serious reported driver or feedback fault.

A new portable, allocation-free class `Boat/source/boat_r30h26_output_off_feedback_guard.h` tracks separate OFF-command timestamps for propulsion and coil release. Its `commandedDisable(millis())` is called when the physical `disable()` path is used (attach or fault); `acceptedOutput(desired,millis())` runs only after `driver.apply()` reports success. Repeated OFF writes **do not reset** the timer. A genuinely new nonzero motor or release demand starts a new OFF interval when subsequently commanded OFF.

At every fresh local sample before `radio_.update()` or queued CLEAR_STOP/control commands:

- Motor mismatch: fresh `leftMotorActive` or `rightMotorActive` stays true at or beyond the 250-ms provisional OFF settle window.
- Release mismatch: fresh `releaseIdle` stays false at or beyond the 500-ms provisional OFF settle window.
- Only a valid `safetyValid`, `outputFeedbackValid`, `watchdogReady` sample less than 100 ms old can establish the mismatch. **Invalid/stale sensor evidence is already handled separately by the normal STOP/fault policy**, never accepted as physical STOP proof.
- On either persistent contradiction, `driverFaultLatched_` permanently locks this application's motor and coil control, `fault()` calls physical-driver `disable()` again, and current telemetry reports physical `STOP=UNKNOWN`. Subsequent radio handshakes, CLEAR_STOP or normal-looking feedback cannot rearm this application instance.
- When `fault()` must finish an in-flight one-shot manual release, it sends a **failed** application ACK with `stopState=2` (UNKNOWN) instead of deriving post-STOP status from a pre-STOP sensor sample.

**Thresholds of 250 ms and 500 ms are software-test parameters**, not validated component characteristics. The 500-ms period is consistent with the existing bounded software release feedback wait, but must not be interpreted as proof of safe electrical disconnect. A stuck MOSFET or welded relay cannot necessarily be disabled by sending the OFF command again; actual independent hardware cutoff and fault-current measurements remain open commissioning blockers.

## Deterministic tests

| Scenario | Expected application policy |
|---|---|
| OFF command, motor still reports active at 249 ms | No premature permanent feedback latch; STOP=UNKNOWN while active |
| Motor reports active at exactly 250 ms, including `millis()` rollover | Permanent fault/STOP latch, `disable()` repeated, no control rearm |
| OFF command, `releaseIdle=false` at 499 ms | No premature feedback latch; physical STOP not confirmed |
| Release still non-idle at exactly 500 ms | Permanent fault/STOP latch; one-shot ACK failure + UNKNOWN |
| Repeated successful OFF writes during either window | Cannot postpone original deadline |
| Fresh ON demand followed by OFF | Only genuinely new OFF interval restarts relevant timer |
| Invalid or stale local feedback | Cannot be used as stopped-current proof; preexisting sample fault path dominates |
| Subsequent healthy feedback, radio-generation change, CLEAR_STOP and manual commands | No automatic rearm after persistent mismatch |
| Valid local STOP feedback after command | Prior STOP/status rules stay in effect; no false permanent fault from an already settled signal |
| Fault during one-shot release | Negative ACK with UNKNOWN stop state; no physical stopped claim |

Test sources:

- `Boat/tests/boat_r30h26_output_off_feedback_guard_test.cpp` — portable pure-C++ time arithmetic and feedback predicate coverage
- `Boat/tests/boat_r30h26_stuck_motor_application_test.cpp` — actual `BoatApplicationPort.cpp` with local motor-feedback stuck high despite accepted OFF
- `Boat/tests/boat_r30h26_stuck_coil_application_test.cpp` — actual `BoatApplicationPort.cpp` with release-idle never restored despite STOP

Tests execute through the existing deterministic radio/driver **host stubs**, not actual ESP32 I/O. The Boat host and Boat cell of the PlatformIO workflow run the same tests with `-std=c++17 -Werror`. Remote firmware/radio command schema was not changed.

## Commands / evidence

```powershell
python tools/flytec.py audit
python tools/flytec.py test-host
python tools/flytec.py build-all
python tools/flytec.py snapshot
```

- [Boat host CI for R30H-26](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45211469334)
- [Remote host CI](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45211469197)
- [Conservative source audit](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45211469211)
- [Boat + Remote PlatformIO target compile, no flashing](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45211469212)

The report links the **source commit** that contains the executable test changes. Verify CI conclusions before announcing complete PASS. Documentation-only changes do not rerun firmware target compilation.

## Hardware acceptance / next step

Official firmware versions stay **Boat VER014 / Remote VER028** pending actual assembly. R30H-10 stays **12 NOT_RUN / 6 BLOCKED**; R30H-12 stays **7 NOT_RUN / 5 BLOCKED**. Source-level GPIO, antenna and dangerous output enabling flags remain 0/locked. No real current, watchdog, module supply, antenna or STOP latency was measured.

**Next suggested R30H-27:** review whether physical feedback flags are derived from independent instrumentation or from software-command echoes; prepare a hardware commissioning matrix mapping each `BoatHardwareSample` field to its real sensor, physical test point, active-low/high polarity, update age, and failure reaction. Software cannot compensate for absent independent feedback by inventing current-zero evidence.
