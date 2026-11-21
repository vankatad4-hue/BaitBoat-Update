# R44 — Fully automatic figure-eight: trajectory intent implementation and commissioning gates

## User requirement
No manual steering during compass calibration. The boat shall automatically drive a horizontal figure-eight while GY-91 and M100 compass readings are collected, then stop and apply **only validated** calibration. A boat is nominally level, but vertical field and roll/pitch still affect heading; XY calibration does not eliminate the need for orientation and tilt checks.

## Implemented
- Portable `Figure8AutonomousMotion` generates bounded alternating differential-thrust intents (two opposing turns, nominal 12s each). No rudder. Stops when the dual compass candidate is ready, 180s expires, physical STOP/watchdog/battery/GNSS/geofence/interlocks/radio fail, operator overrides, or control loop exceeds 250ms.
- Requires authenticated request, fresh primary radio, physical STOP released, watchdog ready, battery healthy, fresh GNSS, geofence OK, idle solenoids, independent motor interlock and stationary start.
- Motor intents **are not connected to actual actuator driver**. The current Boat firmware is intentionally diagnostic-only and cannot energize motors; no unsafe workaround or simulation of physical safety inputs.
- Added host tests and CI gate.

## Required before enabling autonomous movement
1. Integrate authenticated start/abort command and status into Boat/Remote radio protocol, remote Bulgarian UI and physical STOP. Manual control and STOP must preempt calibration instantly.
2. Install and independently test physical DC-rated motor cutoff, watchdog lease and fuses; verify the motor driver current limit, differential turn direction, hull turning radius and safe speed in a controlled water area. This is essential: alternating thrust alone is not proof of a closed figure-eight track.
3. Use live GNSS/heading closed-loop turn controller to achieve two circles with bounded radius, obstacle/shore clearance and geofence; stop on stale sensors. Existing open-loop intent is a preliminary planner, not a proven autonomous path.
4. Support actual M100 magnetometer silicon (QMC5883L vs QMC5883P), validate compass orientation and magnetic interference from running motors. The M100 GNSS parser must support its configured UBX stream.
5. Verify dual-compass agreement and GPS course when speed/conditions permit; reject bad candidates, retain last known-good calibration, and implement CRC-checked atomic persistent storage with recovery.
6. Physically validate the complete behavior on water. No ground-based rotation required. Never enable autopilot based on unvalidated figure-eight data.

## Status
**Automatic steering trajectory intent and safety gating coded and tested at host level. Not commissioned; automatic driving, saving, and navigation calibration are not yet end-to-end operational.**
