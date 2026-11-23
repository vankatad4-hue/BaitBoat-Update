# R30H-27 — Feedback provenance, current-zero meaning and physical STOP evidence

**Date:** 2026-11-23  
**Working branch:** `platformio-unified-r30h20`  
**Executable source commit:** `5fec767f47a22aa98464efbaa1093855d8511152`  
**Classification:** READ-ONLY, logic-only, **no physical safety acceptance**.

## Findings from the actual active firmware

1. `Boat/source/firmware/Flytec_Boat.ino` invokes `application.tick()`, but does **not** attach a commissioned `BoatPhysicalDriver`. The existing R30D source is only a **diagnostic** read-only interface, with no selected real reader or approved sensor/contactor pins.
2. `Boat/source/boat_physical_driver_adapter.h` and `physical_driver_commissioning_gate.h` implement an **historical two-contactor** model. The selected Boat power topology is **one common 3S4P pack**, feeding MDD20A VM and EKM014 VH from a shared distribution bus. Reusing two virtual contact bits for one common bus would manufacture false independence.
3. `Boat/source/boat_common_bus_feedback_r30c.h` represents the proper prospective function: **one DC-rated common actuator disconnect**, actual contact feedback, independently supervised STOP/watchdog, one-hot non-retriggerable coil limiter, and real independent motor/coil current-zero observations. Every commissioning gate remains false.
4. `Boat/source/boat_common_bus_readonly_port_r30d.h` consumes seven origin-ID/timestamp-tagged observations. It catches absent/duplicate input IDs and stale data, **but distinct IDs are not proof of electrical independence**. It leaves `ACTUAL_READER_COMMISSIONED=false`, all source signoff bits false and does not authorize power.
5. `BoatHardwareSample` holds `stationary`, `releaseIdle`, `leftMotorActive`, `rightMotorActive` and `outputFeedbackValid` as ordinary booleans. In current portable tests these are **fake driver inputs**. There is no installed real sensor establishing their hardware origin.
6. `boatHardware_status(...).stopState==1` denotes a **reported locally sampled idle/stopped state** under the current software contract. It is **not** signed proof of zero physical current. RF STOP ACK, a successfully requested OFF signal, MCP23017 output latch readback and a BMS label likewise cannot prove physical motor/coil current zero. R30H-26's 250/500 ms reported-output mismatch gates use the same software sample and do **not** become independent protection solely by passing host CI.
7. The selected INA231/ARDBOARD1491 configuration is **BUS-voltage-only** (not a commissioned 20 A shunt-current measurement). Never use its 0.1-ohm R100 shunt in the common high-current circuit or treat a 12 V voltage value as proof of current-zero.

## New implementation

- `Boat/source/boat_r30h27_feedback_provenance_audit.h`: pure-C++ read-only origin classifier, using the **actual R30D Capture**. It requires seven correct source IDs, type-appropriate external observations (STOP, watchdog, common auxiliary: dry-contact; limiter: external limiter; motor/coil zero: current measurement; pack battery: BUS-voltage measurement), freshness and externally asserted safety. It refuses command echo, MCP23017 register readback, MCU-derived estimates and unsigned origin claims.
- `Boat/tests/boat_r30h27_feedback_provenance_test.cpp`: live R30D fake-reader host test exercises all seven origin/type/claim combinations; missing/duplicated/stale observations; reported physical faults; 32-bit clock wrap; and the explicit separation between a simulated wire `stopState=1` and independent physical current-zero.
- `Boat/docs/BOAT_R30H_27_FEEDBACK_PROVENANCE_MATRIX.csv`: **18** individually tracked safety/navigation/feedback signals, measured source and polarity placeholders, test IDs, raw-capture hashes and reviewer fields. All physical results are `NOT_RUN`; none are implicitly signed.
- `Boat/tools/verify_r30h27_feedback_matrix.py` and `Boat/tests/test_r30h27_feedback_matrix.py`: reject fabricated signatures, altered commissioning flags, undocumented polarity assumptions, changed physical-gate counts, GPIO command-echo substitutions, actual driver attachment and source-path drift.
- All checks are integrated into Boat host CI, read-only source audit, Boat PlatformIO CI and `python tools/flytec.py audit/test-host`. Remote source and on-wire radio protocol are unchanged.

**Important:** Even a host fixture that claims all sources are external and reviewed returns `PHYSICAL_EVIDENCE_NOT_SIGNED`; `mayCertifyCurrentZero` remains false. This classifier is not an alternate physical power-control interface.

## Required physical STOP/current-zero validation

The controlled physical procedure is at [BOAT_R30H_27_STOP_CURRENT_ZERO_BENCH_PROCEDURE.md](../Boat/docs/BOAT_R30H_27_STOP_CURRENT_ZERO_BENCH_PROCEDURE.md).

**Preconditions:** determine actual module revision and DC ratings; choose an independently operating, suitably DC-rated common actuator cutoff and its separate auxiliary contact; select actual independent current measurement for MDD motor and EKM coil functions; validate external watchdog response and the coil pulse limiter; review actual wiring polarity/pin levels/fusing/energy budget. These tasks are still **BLOCKED**. The existing pin map entries are proposal/PROBE_ONLY, not approved GPIO.

**Capture requirements:** record the software STOP request and authenticated Boat receipt separately from physical common-bus voltage (both MDD VM and EKM VH), independent contact state, both motor currents and each relevant release current, with a **shared external analyzer clock**. Record test configuration, cal/serial numbers, raw waveform hash, reviewer signature and times. H10-09 separately requires **at least 30 STOP button trials**; no timing metric is inferred from software CI.

**Fault injections for an approved, externally supervised dummy-load setup:** frozen MCU/watchdog, reset/brownout, open STOP loop, welded/stuck output emulation, lost feedback contact, contact polarity reversal/reader failure, stale current sample, simultaneous release requests and failed one-hot pulse cutoff. **Every case must physically deenergize both load classes and produce measurable current-zero** or fail acceptance. Actual high-current energized tests must not start before independent STOP cut-off and installation are reviewed and signed.

## Status and handoff

Software CI links for source commit `5fec767f`:
- [Boat host tests](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45213945400)
- [Remote host tests](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45213945696)
- [Source provenance and project audit](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45213945722)
- [Boat + Remote PlatformIO builds, no firmware flash](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45213945498)

**Physical:** R30H-10 **12 NOT_RUN / 6 BLOCKED**, R30H-12 **7 NOT_RUN / 5 BLOCKED**, R30A **24 OPEN**. Actual MDD20A VM/EKM014 VH and their outputs remain uncommissioned; `COMMON_BUS_HARDWARE_COMMISSIONED=false`, `ACTUAL_READER_COMMISSIONED=false`, `PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE=false`, and `SHARED_BUS_PHYSICALLY_ACCEPTED=false`.

**Version freeze:** Boat **VER014**, Remote **VER028**. Do not flash or energize motors/solenoids. No sonar added.

**Suggested next R30H-28:** check all producer-to-consumer safety sample data flows, especially `stationary`, `outputFeedbackValid` and timestamps, and prevent uncommissioned read-only diagnostic values from becoming operational safety authority. Existing host samples can represent hypothetical valid hardware for testing but must not promote themselves into physical truth.
