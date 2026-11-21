# R56 — Boat prototype release gate (NOT a hardware acceptance)

## Required target
A physically assembled Flytec V080 Boat retrofit with modular, purchasable modules only, no custom PCB, differential thrust using two 550 brushed motors, MDD20A, EKM014 four-channel solenoids, ESP32-S3-DevKitC-1-N32R16V, HGLRC M100 PRO GNSS, GY-91 IMU, nRF24 primary and E22 UART backup. Remote's integrated SD stores maps/waypoints; Boat uses internal NVS for compass calibration. Sonar is deferred.

## Software state audited 2026-11-22
- `Boat/source/firmware/Flytec_Boat.ino` is explicitly non-actuating. It creates `BoatApplicationPort` but does not attach `BoatPhysicalDriver`. It cannot drive motors or coils.
- `Boat/source/boat_physical_driver_adapter.h` is a callback contract, not a physical GPIO implementation. `Boat/source/mdd20a_arduino_stage.*` is not attached to application.
- EKM014 software guard is not a substitute for an independent hardwired one-hot pulse limiter and DC-rated cutoff.
- `figure8_calibration_nvs.*` exists but is not wired to boot-time load, candidate validation, or calibration application.
- R46 figure-eight supervisor is heading-progress only; no verified GPS geofence/radius/obstacle avoidance, no Remote authenticated Start/Abort end-to-end.
- Exact E22 module/Remote PHY compatibility, nRF24 PA supply and coexistence, GNSS/compass variants, board GPIO headers, STOP/contactor/coil feedback, and watchdog are not physically validated.
- No physical commissioning, motor stall/thermal tests, radio range, GPS navigation, solenoid pulse measurements, failsafe/RTH water acceptance, or sonar.

## Mandatory independent hardware acceptance (do not bypass)
1. Identify exact purchased module SKU/revision and supplier for each module. Verify datasheet, input/output logic, DC current and breaking ratings, voltage, safe power-up state and pinouts.
2. Establish independent normally-safe STOP loop, appropriately DC-rated motor and coil power disconnect, external watchdog, low-voltage cutoff and coil one-hot pulse interlock. Verify with MCU held in reset and simulated stuck GPIO.
3. Test isolated 3.3V and 5V power, battery BMS, wiring fuses, motor stall current and solenoid pulse current with a current-limited bench supply. Keep propellers and coil loads disconnected initially.
4. Implement actual fail-closed `BoatPhysicalDriver` HAL with signed pin audit, read-back of physical feedback and hardware lease. Ensure motor and coil never run together.
5. Verify radio authentication, link-loss RTH, emergency STOP and STOP reset only with neutral sticks; verify RTH geofence and GPS freshness with real telemetry.
6. Test figure-eight autonomously only after safe open-water operating envelope, geofence, GPS heading agreement and human physical abort; validate calibration NVS rollback.
7. Run complete host tests and actual ESP32-S3 target compilation, RF integration, HIL fault injection and on-water acceptance. Record dated evidence and measured values.
8. Only then cut a verified release ZIP and change Boat VER014 to next official version.

## Release status
**NOT PROTOTYPE-READY / NO ACTUATOR COMMISSIONING.** This document is a release blocker matrix, not a substitute for measurements. No motor or solenoid output is authorized by the current firmware.
