# R50 — Physical driver adapter, uncommissioned

Implemented `Boat/source/boat_physical_driver_adapter.h`: a callback-based bridge for `BoatHardwareSample`, independently sourced safety contacts, motor PWM/DIR stage and EKM014 single-coil pulse guard. `Boat/tests/boat_physical_driver_adapter_test.cpp` exercises motor requests, coil pulse timeout and STOP feedback faults. CI host compilation added.

**Not wired to active firmware**: `BoatApplicationPort::attach` still has no real local driver registered. Hardware HAL callbacks have no GPIO bindings and must not be stubbed with invented feedback. No motor or solenoid supply may be connected.

### Next technical steps
1. Verify exact ESP32-S3 DevKitC-1-N32R16V header/strap/1.8V restrictions and module schematics, EKM014 logic polarity, coil current and independent pulse hardware.
2. Write commissioned GPIO/HAL implementation with actual input pull configuration and fault-on-boot output state; physically prove STOP, watchdog and contactor auxiliary contacts independent of MCU.
3. Adapt the bridge to the application `BoatPhysicalDriver` function-pointer contract with measured local samples and timebase; connect MDD20A LEDC stage and EKM014 expander only after hardware verification.
4. Audit transition between motor demand and solenoid pulse; require motor-current-zero before coil energization. Guard is software-only and does not prevent stuck MCU outputs.
5. Implement geofenced GPS figure-eight, persistence and authenticated Remote Start/Abort; full Boat and Remote target builds and RF/physical tests remain outstanding.

Official Boat VER014 unchanged; no verified hardware release.
