# R30C — One common 3S bus: physical STOP/watchdog feedback contract

**Date:** 2026-11-22  
**Status:** interface design + host-only fault simulation; **NOT** hardware selected, installed or commissioned.

## Fixed user decision

The Boat has **one 3S4P Li-ion pack and one shared actuator high-current 3S bus**, which feeds Cytron MDD20A **VM** and Electrokit EKM014 **VH** as two **parallel taps**. They share the same battery positive and return distribution; there is NO second battery, motor-only master rail or separately isolated coil master rail. Regulated 5V and 3.3V *logic voltage rails* remain as selected; they do NOT constitute separate motor/coil power supplies.

R29/R30B software uses fail-closed inputs and denies simultaneous motor+coil *commands*, but cannot shut down a stuck output stage or a frozen MCU.

## Problem discovered in the actual source

The existing `Boat/source/physical_driver_commissioning_gate.h` and `Boat/source/boat_physical_driver_adapter.h` are a **historical dual-contactor safety model**. They require distinct:
- `motorContactorFeedbackClosed`
- `coilContactorFeedbackClosed`

The selected R29 common-bus architecture provides neither separate contactor. **Do not alias both booleans to one imagined feedback input**, assume a BMS gives them, or sign them via the ESP32/MCP23017 output latch. The existing driver remains **unattached** in the default `Flytec_Boat.ino`, so no actuator power is enabled.

## R30C proposed mapping (single common high-current bus)

| Historical input / purpose | R30C common-bus authority | Real physical evidence still missing |
|---|---|---|
| `stopLoopClosed` | `stopCircuitHealthy` | Hardwired STOP circuit that does not rely on MCU software |
| `watchdogContactClosed` | `independentlySupervisedWatchdogHealthy` | External freeze/timeout supervision and verified load-off function |
| `motorContactorFeedbackClosed` and `coilContactorFeedbackClosed` | **ONE** `commonDisconnectClosedFeedback` | A **single common DC-rated load disconnection** plus **actual feedback**, not two duplicated contacts |
| `isolationTestSigned` | `commonDisconnectDcRatingSigned` AND `commonDisconnectPowerOffTestSigned` | DC inductive ratings, interruption/recovery, welded/stuck-on fault tests |
| `batteryHealthy` | `localBatteryHealthy` | Fresh calibrated INA231 BUS voltage and R79/R30B software policy; does not replace current protection |
| `coilCurrentZero` and `Ekm014Feedback.allCoilCurrentsZero` | `independentlyObservedCoilCurrentZero` plus `coilCurrentZero` | Independent, physically measured current-zero after coil power-off; expander register readback is insufficient |
| `motorCurrentZero` | `independentlyObservedMotorCurrentZero` plus `motorCurrentZero` | Motor current zero on transition to any release pulse; DO NOT require motor zero during active propulsion |
| `independentPulseLimiterReady` | `coilOneHotPulseLimiterCommissioned` | One-hot non-retriggerable pulse ceiling **outside the MCU**, verified on each of four 6A-class releases |

The proposed one common DC-rated disconnect is a future **function**, not an approved commercial part or extra battery. Selection, wiring gauge, fuse, fault-current capacity and contact feedback still need real engineering evidence. Without a suitable physically independent cutoff and current/limit checks, actuator commissioning **cannot be marked complete**, even if all radio and software simulations pass.

## Code and test evidence

- `Boat/source/boat_common_bus_feedback_r30c.h` is a **pure, transport-neutral inspection contract**, not a real I/O driver. It rejects expired evidence (50ms), missing STOP/watchdog, missing common disconnect, missing DC-rating/power-off signoff, missing local battery, absent independent release limiter, absent independent motor/coil zero, invalid >100ms lease, >500ms release pulse and motor/release overlap. In motor mode it requires coil-current-zero but *does not* require motor-current-zero once motors are moving; before release it demands both actual current-zero states.
- Crucial `COMMON_BUS_HARDWARE_COMMISSIONED=false` stays in source. `mayEnergizeActuators(...)` **always returns false** for the checked-in revision, even if an entirely synthetic host fixture has every evidence bit set. The `inspect(...)` return value `DESIGN_EVIDENCE_CONSISTENT` means only that a hypothetical vector has internally coherent booleans; it is **NOT** authority to energize power.
- `Boat/tests/boat_r30c_common_bus_feedback_test.cpp` injects absent STOP, failed watchdog, unqualified/welded common disconnect, missing current-zero, invalid pulse/time lease, over-age evidence, reboot clock wrap and stale battery, and verifies every attempted energization is denied.
- R30C updates the legacy R48 header comments to explicitly prohibit faking two distinct contactor contacts from one common bus. Legacy host-only driver fixtures remain historical coverage and **must not** be used as an R30C commissioning witness.
- `Boat/tools/verify_r30c_common_bus_contract.py` checks source-level physical locks, the unified R29 netlist, unsigned R30A acceptance gates and real `Flytec_Boat.ino` diagnostic having no `BoatPhysicalDriver` attachment. The GitHub Boat host workflow runs both tests automatically.

## Physical bench acceptance — **NOT RUN**

1. Identify exact purchased modules and battery/BMS; calculate worst-case DC short/stall current, length/area of conductors, connectors, thermal and polarity protection; **no guessed fuse rating or undocumented module model**.
2. Prove single shared-bus architecture on a power-off wiring diagram; no daisy chain of motor current through MDD board into EKM, no exposed energized wire or 0.1-ohm INA231 shunt in the high-current circuit.
3. Select and document ONE common, DC-rated actuator disconnect / emergency-stop function capable of deenergizing both loads despite firmware freeze, and independently detect its actual state. No separate motor/coil batteries.
4. Prove hardware watchdog loss interrupts **all** actuator supply under logic power brownout, MCU reset, frozen loop or broken STOP wire; test whether a welded/stuck FET can still energize either load.
5. Verify an independent one-hot bounded-pulse mechanism and independent current-zero observation on the four EKM outputs, plus motor-current zero before any release pulse.
6. Only then implement a **new physical HAL adapter** that supplies these verified common-bus values, link it to the application after signed commissioning and run target/bench failure tests. Do NOT refactor the old dual-contactor driver by simply assigning both feedback inputs to a single contact.

### Release boundary

Software design and diagnostic builds can progress while all physical safety gates remain **OPEN**. R30A's **24** current physical gates and the R23/R25/R27 **81** NOT_RUN rows remain unchanged. No new module or power architecture is adopted. Boat VER014 / Remote VER028 unchanged; sonar deferred.

**Next engineering priority:** verify the R30C host suite, then continue code-level design of a read-only common-bus feedback HAL and its failure tests (without physically enabling any actuator), while the actual common cutoff and sensing hardware remain unselected.
