# R30E — Local common-bus fault-status diagnostics and telemetry

**Date:** 2026-11-22. **Purpose:** make R30D feedback failures visible as serial diagnostic telemetry, while keeping all load outputs locked. **No new physical module, motor/solenoid power domain, GPIO assignment or Remote wire protocol is selected.**

## Current selected topology

ONE 3S4P battery feeds BOTH Cytron MDD20A VM and Electrokit EKM014 VH through a future, common high-current distribution. The 5V and 3.3V *logic regulators* remain separately regulated voltage rails but share the same battery. No real external STOP, watchdog, common-disconnect auxiliary, coil pulse/current-zero or motor current-zero reader has been installed, wired or commissioned.

## R30E implementation

- `Boat/source/boat_r30e_fault_status.h` is a **portable pure C++** status classifier. It maps R30D's `NEVER_READ`, I/O failures, unassigned/duplicate source IDs, expired safety and battery observations, and invalid battery data to stable fault codes. When a *synthetic or later real* observation is fresh and fully present, it classifies asserted STOP, watchdog, single-disconnect auxiliary, limiter and current-zero faults by priority; a flags mask preserves *all* simultaneously observed faults. An apparently healthy input frame reports `UNCOMMISSIONED`, **never OK_TO_DRIVE**.
- Every report includes `UNVERIFIED` and `COMMISSIONING_LOCK` flags; `safeToActuate` is always false. Missing/faulty inputs have battery voltage `0xFFFF` (no trusted measurement), not a cached voltage or stale battery percentage.
- The local diagnostic sidecar is eight bytes: `D5 01 <reason> <flags low> <flags high> <voltage low> <voltage high> <sequence>`. This representation is **NOT A RADIO PAYLOAD** or a new `RadioPayloadKind`. It is local-only USB serial diagnostic encoding pending a separately reviewed authenticated Remote protocol update.
- `boat_r30e::ChangeTracker` records *changes in the fault reason/flags*, not actuator or clear-STOP events. Repeated status messages do not rearm STOP.
- `Boat/source/firmware/Flytec_Boat.ino` now compiles and uses R30E every 2000ms to emit `R30E,LOCAL_ONLY,...` status plus the encoded frame as hex on USB Serial. It creates an empty `ReadOnlyCommonBusPort` but **does not attach a physical reader**. Therefore the correct default is **`NO_READER`, `UNVERIFIED`, `outputs=LOCKED`**, not healthy feedback. No new GPIO is configured; the normal application remains non-actuating and unattached from a `BoatPhysicalDriver`.

## Test coverage

`Boat/tests/boat_r30e_fault_status_test.cpp` injects six independent local safety inputs plus battery from a *fake* R30D input source, verifies fault priority and complete flags, no battery value leakage on stale/bad frames, type-safe 8-byte encoding, event transition count, cache expiry after 50ms, missing/duplicate source IDs, and absent physical driver. Existing R29/R30A/R30B/R30C/R30D host suites remain in place.

`Boat/tools/verify_r30e_fault_status.py` statically verifies the firmware uses only local Serial for R30E, no Remote/RF payload kind was introduced, the default physical commissioning flags remain false, there is no physical driver attachment, and **24 physical readiness gates remain OPEN**.

## Physical and protocol limitations

- These are **reports from a diagnostic software model**, not trusted external hardware contacts. Distinct source IDs do not prove physical safety independence.
- Do **not** route the D5 record into the existing authenticated radio STATUS packet or Remote UI as if the actual Boat has working safety contacts; the exact Remote wire layout must be reviewed on both ends in a later dedicated protocol version.
- R30E cannot power-off a stuck MOSFET, cut the 3S load rail during MCU freeze, measure actual 20A pack current, or prove bounded one-shot 6A solenoids. The physical common-bus DC-rated cutoff, externally supervised watchdog, current-zero sensors, separate coil pulse limiter and verified pack protection remain undecided.
- R30A physical G01–G24 **OPEN**, older R23/R25/R27 81 acceptance rows **NOT_RUN**, INA231 and R30D actual reader **NOT COMMISSIONED**, no motor/solenoid high-current wiring connected. Official Boat **VER014** and Remote **VER028** unchanged; CHIRP sonar deferred.

**Next:** Observe GitHub R30E host + ESP32-S3 target runs, correct actual failures, update continuation checkpoint. Once an actual supported safety input module is chosen, prepare a physical logic-only reader without adding a write-capable actuator output.
