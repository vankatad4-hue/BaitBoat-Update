# R29 — One common 3S battery bus and integrated software fault scenarios

**Date:** 2026-11-22 · user-directed shared-power topology, **NO ACTUATOR POWER COMMISSIONED**.

## Binding power design decision

**One existing 3S4P Li-ion pack (11.1V nominal, 12.6V full), one common protected 3S distribution bus.** Cytron MDD20A **VM** and Electrokit EKM014 **VH** connect as parallel loads/taps to that *same* bus, with a common return. There are **no separately isolated or independently switched motor and solenoid power domains**, no second battery and no custom PCB. This document supersedes **only** the historic R23 power nets `H07_MDD/MDD_MOTOR_SUPPLY` and `H08_EKM/EKM_VH` and any R20 text suggesting separately switched actuator power branches. The original R23 module signal/GPIO/logic-rail map remains unchanged.

See [R29 single-bus connection schedule](BOAT_R29_SHARED_3S_BUS_CONNECTIONS.csv): 8 UNWIRED_PENDING nets. Both loads return to one common high-current source; **do not daisy-chain heavy current through the MDD20A board to feed EKM014**. Ground reference is common, but choose proper return-conductor and termination design to avoid routing starting current through sensitive I²C or ESP ground conductors.

**Logic voltage is not the load bus:** EKM014 still requires a regulated 5V **VL** from selected TRACO TSR 2-2450 and must satisfy VH ≥ VL (Electrokit manufacturer); ESP/M100/E22 5V uses Olimex DCDC6-16-TO5; Pololu #1 nRF 3.3V and #2 sensors/INA231/MCP 3.3V remain. These are voltage-conversion rails, not "separate motor/solenoid power" supplies.

The MDD20A manufacturer states 6–30V motor supply and **no reverse polarity protection**. EKM014 accepts VH at the full 3S voltage, includes a **10A slow-blow fuse in its VH circuit**, and has low-side coil outputs. The EKM board fuse is not a safe pulse-timer or motor short protection. The selected 20A battery BMS is NOT proof that two 550 brushed motor stall currents + a ~6A solenoid + logic current can safely run together. Even without simultaneous actuation, measure worst-case motor combined stall/starting current plus support electronics and coil pulse/temperature, wiring/conductor gauge, manufacturer terminal current, fuse/breaker ratings and system transients before connecting the shared power bus. The final common-input wire protection/fuse and emergency disconnection ratings are **not yet chosen or tested**; this document does not waive electrical fire protection.

Sources: [Cytron MDD20A product](https://www.cytron.io/c-industry/p-20amp-6v-30v-dc-motor-driver-2-channels), [EKM014 manufacturer](https://docs.electrokit.com/en/modules/EKM014/).

## R29 software work

1. `Boat/source/boat_r29_shared_bus_contract.h`: selected single-pack/single-bus contract (`SEPARATE_MOTOR_SOLENOID_POWER=false`) and pure `exclusiveActuatorRequest` that denies invalid motor/coil combinations, plus a **synthetic** aggregate current-bound estimator. It makes **no guess** about real motor current and does not authorize physical hardware. `SHARED_BUS_PHYSICALLY_ACCEPTED=false`.
2. `Boat/source/boat_physical_driver_adapter.h`: applies the new shared-bus exclusive request predicate **before** any physical motor or coil HAL callbacks. Existing authorization and post-coil zero interlocks remain; no outputs enabled.
3. `Boat/tests/boat_r29_shared_bus_integration_test.cpp`: integrates the **actual R78 radio watchdog, R79 BoatBatteryMonitor, BoatActuatorSupervisor and R28 INA231 acquisition** in synthetic scenarios. It simulates manual joystick lease, total RF loss STOP, backup-only link rejecting manual stale drive, 10s Home eligibility with verified local safety sample, invalid nav, low battery holding outbound, 9.9V immediate critical STOP, failed ADC read with invalid battery, nonexistent physical driver hardware, motor/coil exclusivity and synthetic shared 20A BMS budget violations.
4. `Boat/tools/verify_r29_shared_bus.py`: static authority audit for the new 8-net overlay, all unwired, 3S common topology, R27 INA231 uncommissioned, previous independent safety gates still off and physical signoff sheets unsigned.
5. Boat host CI step builds R29 integrated C++17 host test and executes power-topology audit. New CI Actions result must be observed before reporting a passed branch workflow.

## Software vs hardware safety boundary

**R29 does not install or emulate an independent physical safety device.** The existing `BoatPhysicalDriverAdapter` still refuses initialization unless commissioned independent safety feedback exists; its historical separate motor/coil contactor feedback requirements are unsatisfied under the shared, non-independently switched power topology. Do not forge or combine those independent feedback signals to make the existing gate pass. The current physical driver cannot actuate motors/solenoids. If the user later chooses a single *common* DC-rated emergency disconnector, that would be one master device, not two separate power supplies; such a part is NOT selected in R29.

**No high-current actuator wiring has been connected.** All related R23/R25/R27 acceptance checks remain `NOT_RUN`, no GPIO audit signed, INA231 physical commissioning false, no ESP controller purchasing/measurements, no motor stall or coil pulse measurements. Boat VER014 / Remote VER028 release markers unchanged. Sonar deferred.

### Next

Verify the R29 host Actions run and complete updated ESP32-S3 target compile. Continue non-actuating application-level radio/mission/failsafe fault simulations. For later physical prototyping measure selected 3S rail performance with a bench supply, first with motor/coil leads absent, and resolve common feed protection and actual STOP behavior without inventing independent contacts.

## R29 integration regression and purchasing correction

The integrated R29 host test additionally exercises **one-hot manual release 0**, rejects a concurrent second solenoid pulse, forces all release/motor requests OFF upon primary+backup loss *during that pulse*, requires latched STOP, and rejects initiating a release on backup-only coverage. These are only software request outputs; there is no EKM014 voltage or measured coil current.

The active [R20 purchase register](../../docs/PROJECT_PURCHASE_REGISTER_R20.csv) now explicitly states that **MDD20A VM and EKM014 VH share one 3S4P bus** and removes an outdated note incorrectly calling VH a separate 3S pack. R29 static CI checks this wording as well as keeping all physical signoffs NOT_RUN. The independent STOP/cutoff feedback remains uncommissioned and does not become valid simply because only one battery is used.

## R29A verification evidence and corrected worksheet schema (2026-11-22)

- **R29 host CI PASS:** [run 45040175603](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45040175603), commit `ae230dd512c8b35195cc6031b6e71e7dffcbce49`. R29 C++17 shared-supply integration and all downstream Boat portable tests passed.
- **Current firmware-source target compile PASS:** [run 45038989512](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45038989512), Boat source commit `b9964105`. 408681 bytes sketch and 21160 bytes global data. Tests/tools/docs were the only changes following that code revision.
- Earlier runs [45039482082](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45039482082) and [45040073182](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45040073182) failed **only the R29 Python source/evidence auditor**, not the R29 C++ simulator. The R23 sheet uses 18 `GPIO_PIN` rows with `Continuity_Test=NOT_RUN` and 14 `MODULE_INTERLOCK` rows with intentionally empty continuity fields. All 32 have `Power_On_Level_Test`, `Power_Off_Level_Test` and `Test_Status=NOT_RUN`. R25/R27 use generic `Status=NOT_RUN`. The repaired validator checks these exact conditions and prevents an accidentally signed/false result from passing.

These results **do not** prove physical cutoff, motor/solenoid pulse limits or safe current wiring. No motor and solenoid high-current circuit was energized. R29 shared-bus power architecture, disabled INA231 commissioning and versions VER014/VER028 unchanged.
