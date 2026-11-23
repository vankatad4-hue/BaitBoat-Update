# R30H-25 — Driver apply failure latch, brownout and manual neutral after session recovery

**Date:** 2026-11-23  
**Branch:** `platformio-unified-r30h20`  
**Executable commit:** `6f51c14cf9a1b07b4caa6a3808dda8baf406f1a9`  
**Safety classification:** software-only, non-actuating, requires external real hardware acceptance.

## Code-level safety changes

1. `Boat/source/boat_hardware_supervisor.h` adds `requireManualNeutral()`, dropping the current manual lease and setting `manualNeedsNeutral_`. This must be satisfied by a **new authenticated primary neutral-stick input** before a nonzero motor command can be accepted.
2. `Boat/source/boat_application_port.cpp` calls that gate on **noninitial** session-generation changes. It preserves normal initial attach behavior; no prior valid motor lease survives radio re-handshake. `fault()` also requires fresh neutral after local STOP.
3. A new `driverFaultLatched_` member in `Boat/source/boat_application_port.h` is set whenever `BoatPhysicalDriver::apply()` returns false. Once latched, each subsequent `tick()` reasserts `disable()`, processes the radio manager without executing controls, rejects queued non-STOP commands, and offers STATUS with physical STOP=**UNKNOWN**.
4. This driver-failure latch **cannot** be reset by a recovered radio, a healthy battery, a new local sensor read, a new session generation or a remote `CLEAR_STOP`. The current application instance must remain disabled. Actual recovery requires a separately reviewed cold restart, physical output feedback and independent watchdog commissioning; none of those are implemented as an automatic remote pathway.
5. A transient local sensor-read failure still calls `fault()` and forces STOP. Once the local sensor is valid again, only an authenticated primary `CLEAR_STOP` with stationary/idle interlocks can clear software STOP, and the next nonzero stick command requires an explicit neutral command first. **This is not proof that actual current is zero.**

## Host regression tests

The tests compile the **real** `BoatApplicationPort.cpp` and `BoatActuatorSupervisor` using a host-only stub radio and injected `BoatPhysicalDriver` functions. They contain no GPIO, SPI, UART, motor PWM, battery acquisition or real radio airtime.

| New test | Expected result |
|---|---|
| `boat_r30h25_driver_session_fault_test.cpp`: normal authenticated manual control | Nonzero motor demands permitted only by software authorization |
| Increment `sessionGeneration()` while manual motor lease is active | Clear old output; reject nonzero sticks until a new neutral command |
| Sensor-read failure while motors are requested | Output disabled, STOP latched, mission aborted |
| Attempt backup-only CLEAR_STOP | Rejected; primary-only safety policy remains |
| Primary CLEAR_STOP after a real fresh simulated sensor sample | May clear software STOP; still requires new neutral before nonzero sticks |
| `apply()` rejects a motor command | Permanently latch driver error and reassert disable |
| Recover physical writer, radio session and fresh measurements after failed apply | Cannot clear STOP or restart outputs, even when neutral/control commands arrive |
| STATUS when driver fault is latched | UNKNOWN physical STOP; no fabricated current-zero proof |
| `boat_r30h25_brownout_recovery_test.cpp`: 9.8-V abrupt local voltage sample | Immediate STOP |
| Battery rebounds to 12.6 V and radio session increments | STOP remains latched; no automatic motor/solenoid restart |

## Verification and developer commands

Portable and PlatformIO workflows invoke both host tests and existing R30H-18 through R30H-24 suites, with `-Werror`. Static source audit protects the driver-fault latch, locked-command branch and new-session neutral gate; mutation tests intentionally remove these clauses to confirm the audit fails.

```powershell
python tools/flytec.py audit
python tools/flytec.py test-host
python tools/flytec.py build-all
python tools/flytec.py snapshot
```

- [Boat host tests — PASS on executable commit `6f51c14c`](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45209114670).
- [Both PlatformIO target compiles — inspect final verdict for `6f51c14c`](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45209114729).
- [Source inventory / mutation CI](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/source-audit.yml).
- [Complete version-frozen project ZIP with SHA-256](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/package-flytec-zips.yml).

## Limitations and handoff

This code is an additional software guard. Real actuator STOP/current-zero cannot be established from the success of `disable()`, the result of a `driver.apply()` call or a logical watchdog. The boat still requires an independently validated emergency power-cutoff/watchdog architecture on its shared 3S4P bus, actual nRF24 and E22 measurement, GNSS calibration and controlled motor/solenoid commissioning.

**Physical gates:** R30H-10 **12 NOT_RUN / 6 BLOCKED**; R30H-12 **7 NOT_RUN / 5 BLOCKED**. GPIO auditable-signoff flags remain disabled, firmware flashing forbidden. The old branch remains archived; Boat **VER014** and Remote **VER028** stay official.

**Next, R30H-26:** evaluate software acknowledgment/feedback semantics when driver disable is issued but actual output feedback remains active, and add no-actuation fault matrix for persistent stuck output. Do not treat an ACK or an asserted STOP pin as measured current zero.
