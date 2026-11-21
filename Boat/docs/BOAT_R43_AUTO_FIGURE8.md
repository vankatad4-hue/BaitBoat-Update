# R43 automatic on-water figure-eight calibration

## Selected operator procedure
Only on-water figure-eight steering is required. Boat remains approximately level. Both the GY-91 AK8963 and the M100 PRO QMC5883L magnetometer independently collect horizontal XY samples. No 3D/ground manoeuvre is required. Pitch/roll still exist; accelerometer tilt compensation and orientation mapping remain mandatory for navigation.

## Implemented firmware
- Diagnostic serial command **c** starts both sweeps simultaneously, requiring both sensors to be present.
- The operator steers figure-eights manually. After at least 30 seconds, the firmware automatically finishes **without an operator finish command** when both have >=150 samples, >=9/12 horizontal sectors and plausible spans.
- If either sensor stops supplying valid samples for >5 seconds, or elapsed time exceeds 180 seconds, session fails closed. Command **x** aborts.
- Results are two separate horizontal offset/scale **candidates**, printed to serial. Neither motor nor solenoid is controlled by this calibration module.

## Remaining necessary integration — do not claim done
1. Verify exact M100 compass silicon: current implementation recognizes QMC5883L only. QMC5883P is unsupported.
2. Verify board-mounted axis alignment, magnetometer field units, both compass sensor freshness, temperature/magnetic interference and compass agreement while underway.
3. Add quality-gated automatic application of candidate offsets to calibrated body-frame heading, retain prior known-good calibration on rejection, and persist atomic CRC-protected calibration records with sensor model, orientation and version. This MUST precede any claim of automatic navigation calibration.
4. Implement authenticated remote command/status and a Bulgarian calibration screen with Start, progress, Abort, and clear failure reasons; calibration must not interfere with manual driving, emergency STOP or independent interlocks.
5. Add on-water GPS-course cross-check on straight motion; GPS course is not yaw at low speed or in current/wind. Do not enable autopilot solely from a figure-eight candidate.
6. Commission M100 GNSS UBX parsing, audited ESP32-S3 GPIO and supply voltages; test both radios, physical STOP and watchdog, and fail-safe behavior before powered actuator testing.

## Physical acceptance
User does not have ground testing available. Perform figure-eight verification on water, with manual control and a safe recovery option. **This diagnostic implementation is not yet a complete operator-ready remote-controlled calibration flow.**
