# R30B — Shared 3S power fault simulation and application-level evidence veto

**Date:** 2026-11-22  
**Scope:** Boat software only; selected ESP32-S3-DevKitC-1-N32R16V, one 3S4P supply shared by MDD20A VM and EKM014 VH, Ardboard INA231 voltage-only BUS sensing, R79 low-battery and 10-second radio-loss conditional Home. **No new modules, independent high-current rails, new regulator, custom PCB or actuator wiring.**

## Code changes

- `Boat/source/boat_shared_supply_fault_guard_r30b.h` provides an explicit *negative-permission* software battery-evidence check, not a power-cutoff controller. It requires local 3S electrical-health evidence, `voltageValid`, a plausible 9000–13000 mV raw voltage, battery policy snapshot valid and noncritical, and sample age no more than the current 1000 ms limit. Any missing/stale/implausible/critical measurement returns `STOP_INVALID_OR_CRITICAL`. Red/low returns `LOW_BATTERY` but never autonomously commands Home.
- `Boat/source/boat_application_port.cpp` invokes that check **after** the existing R79 `BoatBatteryMonitor` update and **before** radio command/mission/actuator arbitration. The result is propagated into `sample_.batteryHealthy` and `fix.batteryHealthy`. The existing STOP path stops the executor, aborts the radio mission and calls `driver_.disable` when that physical-local evidence is invalid. R79 itself continues to determine whether red battery means outbound hold, a pending 3s low-battery Home request or critical STOP.
- `Boat/source/physical_driver_commissioning_gate.h` removes an outdated source comment claiming ADS1115. INA231 is the selected voltage-only part; no separate battery comparator or hardware contact is selected.
- `Boat/tests/boat_r30b_shared_supply_fault_matrix_test.cpp`: direct C++17 tests for healthy vs stale/future/invalid voltage, raw zero and overrange, failed physical flag, valid red/low only with Home/valid navigation/release-idle/STOP released prerequisites, 10-second authenticated dual-radio loss, sudden 12.6->9.9 V critical STOP despite filter smoothing, critical STOP latch through rebound, 32-bit timestamp rollover, motor-vs-solenoid exclusivity and fail-closed absent independent hardware.
- `Boat/tools/verify_r30b_power_fault_lock.py`: static audit that the new guard really is used by the application, the R27 INA231 commissioning flag and R29 shared-bus-physically-accepted flag remain **false**, output-driver independent contacts still gate activation, and the 24 R30A physical acceptance gates remain **OPEN**.

## Verification

- [Boat host R30B run 45043402159](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043402159) **PASS**, commit `5fe8e00fffdc2b6bb7f846ea6ef53a165e7c45b6`. Includes new C++17 failure matrix and the R30B source-lock validator, plus existing R29/R30A host regressions.
- An earlier **intermediate** workflow run 45043384796 reported failure because it started before the R30B Python validator file existed at that commit; its C++17 simulation itself **passed**. The next run including the validator passed. This is not a physical safety failure or compiler regression.
- ESP32-S3 target diagnostic compile is separately tracked as [run 45043323189](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043323189); do not report its status as success until observed after the updated application source change.

## Operational boundaries

1. **One common high-current 3S pack**, no separate motor or coil power. The logic rails remain regulator-separated only by voltage/noise requirements.
2. Neither `inspectBoatBattery` nor `canRequestConditionalHome` performs power switching or creates an autonomous mission. The latter is a pure prerequisite predicate for logic regression; current runtime retains its pre-existing authenticated mission/Home validator. The new runtime-connected check is `inspectBoatBattery`.
3. The present default firmware `Flytec_Boat.ino` still attaches **no** physical `BoatPhysicalDriver`, and INA231 remains uncommissioned. No motors, release coils or shared load bus have been powered.
4. A software STOP cannot guarantee deenergization after a CPU freeze, stuck-on FET or severed IO. The independent physical STOP, cable/fuse sizing, shared BMS peak current and current-zero/pulse limits remain unresolved.
5. All R30A 24 physical gates OPEN and 81 R23/R25/R27 physical checks NOT_RUN. Boat VER014 / Remote VER028 unchanged. No CHIRP sonar selected/implemented.

## Next engineering action

Audit a **single shared DC-rated high-current STOP interface** against the user's no-separate-bus decision without selecting new hardware on speculation. Keep it independent of firmware for freeze/stuck-output scenarios; document exact unavailable feedback signals and a power-off test sequence. Continue only logic-level simulation until part ratings and actual wiring measurements are available.
