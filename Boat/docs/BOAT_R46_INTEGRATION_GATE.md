# R46 target / hardware integration checkpoint

## Verified
R45 GitHub Actions run 44945819892: **ESP32-S3 diagnostic target compile PASS**. R45 host run 44945861528: PASS. These are non-actuating diagnostic tests, not fully commissioned boat firmware.

## R46 development
Added `figure8_closed_loop.h`: requires fresh heading/speed, starts stationary, monitors angular turn progress, switches direction after 350 degrees and stops on heading discontinuity, speed envelope violation, timeout, failed safety gates, or sensor failure. **Still a guarded heading supervisor, not GPS radius/shore-distance/obstacle control.** Its nominal motor intents are not connected to the actuator driver and must remain unpowered.

## Required before hardware enable
- Exact N32R16V J1/J3 pins, octal-memory GPIO reservations, external 3.3V levels and GPIO47/48 1.8V restrictions; verify actual board revision.
- Complete `BoatPhysicalDriver` with local physical STOP, independently monitored contactor, watchdog lease, battery voltage, speed/heading, motor output feedback and EKM014 one-hot pulse/current-zero. The existing `mdd20a_arduino_stage.cpp` is an uncommissioned GPIO HAL, not an integrated physical driver.
- Independent ready-made DC contactor + safety relay + brownout latch + hardware one-hot/pulse limiter + feedback modules: exact qualified parts still unselected. Do not treat an ESP32 output, relay timing sketch, or generic 4-channel MOSFET board as independent safety.
- Autonomy needs authenticated remote command/status, geofence boundaries and distance-to-shore/obstacle envelope, GNSS-guided track/radius feedback, tested compass correction persistence with CRC and rollback, failsafe priority.
- Bench and on-water physical acceptance cannot be performed from the repository. No claims of RF range, motor torque, coil pulse, STOP, RTH or autonomous manoeuvre acceptance.
- Sonar remains deferred. Official Boat VER014 frozen. No release ZIP until physical acceptance.
