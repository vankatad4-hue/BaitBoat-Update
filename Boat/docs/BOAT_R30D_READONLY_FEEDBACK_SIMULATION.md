# R30D — Read-only common-bus feedback transport and fault simulations

**Date:** 2026-11-22  
**Status:** host simulation and ESP32-S3 diagnostic source integration, **NOT COMMISSIONED**.  
**Design:** one common 3S4P battery bus supplies MDD20A VM and EKM014 VH in parallel. **No separately switched motor and solenoid power rails.**

## Purpose

R30C defined a *prospective safety-evidence model* to replace a historical two-contactor assumption that is inapplicable to the user's one shared high-current bus. R30D adds a **transport-neutral, input-only reader** so a future vetted commercial safety/feedback assembly can be inspected without pretending there are physical contacts today. No module, pin, connector or separately switched power supply was selected.

- `Boat/source/boat_common_bus_readonly_port_r30d.h`: `ReadOnlySource::read(RawFrame&)` reads seven observations: hardwired STOP, externally supervised watchdog, ONE common DC load-disconnect auxiliary status, externally bounded coil limiter status, coil-current zero, motor-current zero, and voltage-only local INA231 battery sample. No method to enable or cut power, pulse coils, drive motors, acknowledge keys or rewrite commissioning flags exists. The reader itself has no Arduino/GPIO dependency.
- Each of the first six inputs carries its **own original acquisition timestamp and nonzero source identifier**. Battery carries a separate timestamp and source ID. The diagnostic rejects missing or duplicate source IDs; duplicate detection is useful for spotting a wiring/mapping mistake but **does not certify electrical independence**. Hardwired STOP, external watchdog, contact, limiter, motor/coil current sensors and their instrument IDs are **UNSELECTED**, not fictitiously installed.
- Sensor freshness: feedback at most 49ms old, INA231 battery at most 1000ms old, using wrap-aware `uint32_t` arithmetic. An I/O failure, missing/duplicate provenance, expired sample, future timestamp or implausible 3S 9000–13000mV value clears the entire previous capture to prevent reuse. A healthy diagnostic observation **never rewrites any source timestamp** to the time of read.
- The raw observation remains viewable in a diagnostic snapshot, including `coilPulseLimiter.asserted`; this **does not** set `coilOneHotPulseLimiterCommissioned`, `commonDisconnectDcRatingSigned`, `commonDisconnectPowerOffTestSigned`, `independentlyObservedCoilCurrentZero` or `independentlyObservedMotorCurrentZero`. They remain false, and `localBatteryHealthy` is false while R27 INA231 physical commissioning is false.
- Compile-time `ACTUAL_READER_COMMISSIONED=false` and inherited `COMMON_BUS_HARDWARE_COMMISSIONED=false` ensure that `mayEnergizeActuators(...)` denies all requests regardless of mock read values. `Boat/source/firmware/Flytec_Boat.ino` **includes** the new header and compile-time asserts both locks, but **constructs no reader, configures no new GPIO and attaches no physical Boat driver**. This makes the target diagnostic compile the new API without actuating anything.
- Existing `Boat/source/boat_physical_driver_adapter.h` is a **historical R48 dual-contactor contract** and is still intentionally unattached. No bridging of `motorContactorFeedbackClosed` and `coilContactorFeedbackClosed` to a shared status bit is allowed.

## Fault-injection coverage

`Boat/tests/boat_r30d_common_bus_readonly_faults_test.cpp` uses a fake read-only transport, **not** physical or production feedback. Cases: first read not valid; fresh observation; physically impossible commissioning remains false; I/O NACK after prior valid frame; unassigned/duplicated reader source identifiers; 50ms expired contacts and unchanged cached acquisition timestamps; future timestamps; stale/invalid/overrange battery; STOP open; watchdog lost; common cutoff auxiliary feedback open; current-zero lost; limiter not asserted; oldest-channel timestamp aging; 32-bit `millis()` rollover; missing physical driver and no actuator authorization. External source independence or DC interrupting capacity cannot be demonstrated by host tests.

`Boat/tools/verify_r30d_readonly_feedback.py` statically audits the input-only interface, false commissioning flags, no active application driver, selected common bus, R27 INA231 sensor lock and unchanged **24 R30A physical gates OPEN**.

## Safety and wiring hold

**Do not wire or energize MDD20A VM, EKM014 VH, or release coils using this diagnostic.** Before any real high-current wiring approval, the owner must separately select and physically verify a DC-rated common-bus safety disconnect that opens under STOP and watchdog failure, actual current-zero measurement and independent 6A-class one-hot bounded coil pulse limiting, correct fuses/conductors/pack and BMS current behavior, battery voltage scaling/INA231 pad topology, boot-state defaults and water-safe installation. A software STOP cannot make a stuck FET or stalled MCU safe.

No physical safety part was chosen in R30D, no current sensor was purchased, and the 73-net effective assembly wiring remains `UNWIRED_PENDING`/`NO_WIRE`/`REFERENCE_ONLY`; R30A's 24 acceptance gates and R23/R25/R27's 81 physical checks remain unsigned. Boat **VER014** / Remote **VER028** unchanged; CHIRP sonar still deferred.

## Next steps

1. Run the new R30D C++17 host simulation, static contract audit and pinned ESP32-S3 diagnostic compiler workflow.
2. Continue logic-only fault tests around separate origin timestamps and intentional I/O resets.
3. Do not implement a write-capable high-current HAL until the single-bus independent STOP/contact and pulse/current-zero hardware have been physically specified, measured and approved.
