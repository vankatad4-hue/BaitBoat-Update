# R8 steering requirement: twin-motor differential thrust — no rudder
Confirmed by owner 2026-11-21. This is the authoritative steering model for Flytec V080.

## Propulsion hardware
The original two brushed 550 motors are the only directional actuators. The Cytron MDD20A has independent PWM_L/DIR_L and PWM_R/DIR_R control. No rudder, no steering servo, no servo PWM channel, and no additional steering motor is present or required. The physical port/starboard wiring and motor polarity must be verified at commissioning. Both motor windings must have safe power-off states; the separate independent power cutoff and MCU watchdog requirements are unchanged.

## Requested behavior
- Equal forward demand to port and starboard: approximately straight. Tune trims/drag/propeller characteristics on water.
- For left heading change: starboard forward thrust greater than port forward thrust (or safe reverse differential when approved).
- For right heading change: port forward thrust greater than starboard forward thrust (or safe reverse differential when approved).
- Zero demand: both PWM disabled and hardware gate remains safety controlled, following verified MDD20A braking behavior.
- Pivot-in-place with opposite motor signs is a *possible mode*, not yet authorized. Require explicit limits for reversed motors, loads, hull stability and cable/propeller safety.

## Existing firmware paths (inspection only; no change required)
- Remote TANK: `Remote/source/joystick.cpp` maps left Y and right Y into independent motor demands.
- Remote STEERING: the same module computes `left=throttle+steering`, `right=throttle-steering`, bounded to signed [-1000,1000]. Here `steering` is a **differential thrust mixing term**, never a rudder angle.
- Boat autopilot: `Boat/source/boat_hardware_supervisor.h` converts GPS/heading error into left and right signed propulsion commands using a provisional yaw gain and current cap.
- Motor output stage: `Boat/source/carrier_output_plan.h` maps those commands to two duty/direction outputs with reversal deadtime; `Boat/source/mdd20a_arduino_stage.cpp` is currently an uninstantiated, fail-closed stage.
- Two motor channels share the one independent external actuator power cutoff; no physical steering servo power or signal network is needed.

## Commissioning/acceptance
1. Confirm left/right motor identity and correct forward direction with props disconnected or under a safe dummy-load test.
2. Validate signed turning direction at low power; explicitly check that positive heading error results in the intended physical rotation.
3. Confirm each motor reverses only after safe deadtime and output shutdown.
4. Tune straight-drive trim, yaw gain, safe pivot permission, motor current limits and return-home tracking on actual water only after independent cutoff/STOP passes.
5. Maintain sonar hold: no model, board, or driver in this release. No physical test has been completed by this documentation update.

See `Boat/docs/BOAT_INTEGRATED_SX1262_R8.md`, `Boat/docs/BOAT_SAFETY_SOLENOID_CAPTURE_R8.md`, and `docs/CONTINUATION_CHECKPOINT_VER001.md`. Release markers remain Remote 028 / Boat 014.

## R8 implementation correction — autonomous forward-only turning
Code and host regression on 2026-11-21: `Boat/source/boat_hardware_supervisor.h` clamps autonomous navigation propulsion commands to [0, configured `maxNavMotor`] individually after heading-error differential mixing. **No negative per-motor autonomous commands** are issued, even for large heading error; positive bearing error accelerates left/port over right/starboard (right turn), negative error the reverse. Sharp-course changes can turn by propelling only the outside motor while the inside motor is stopped. This is safer than the previous automatic counter-rotating/pivot behavior until measured hull turn dynamics and reverse allowance have been signed.
Manual TANK/reverse and explicit joystick override remain independent signed-command paths, still protected by command lease, STOP and hardware safety requirements. Ground/water effectiveness, pinouts, polarity and cutoff are unverified. Automated regression is `Boat/tests/boat_hardware_supervisor_test.cpp`; real target and physical checks remain outstanding.

### R8 MDD20A five-signal harness correction
The Cytron **MDD20A** manufacturer interface drawing lists `DIR1`, `PWM1`, `DIR2`, `PWM2`, `GND`, while the retrofit's carrier logical connector currently lists `PWM_L`, `DIR_L`, `PWM_R`, `DIR_R`, `SIGNAL_GND` in positions 1–5. **Those signal sequences are not pin-for-pin equivalent.** The harness must explicitly cross-map `PWM_L→PWM1`, `DIR_L→DIR1`, `PWM_R→PWM2`, `DIR_R→DIR2`, and `SIGNAL_GND→GND`. Manufacturer document: https://www.cytron.io/p-20amp-6v-30v-dc-motor-driver-2-channels ; datasheet signal reference: https://cdn.robotshop.com/media/c/cyt/rb-cyt-287/pdf/cytron-20a-6v-30v-dual-dc-motor-driver-datasheet.pdf . Exact socket orientation and the left/right motor wiring cannot be signed without the actual board.
Manufacturer truth table: `PWM=0` actively brakes both motor terminals low; `PWM=1, DIR=0` drives MxA high / MxB low; `PWM=1, DIR=1` drives MxA low / MxB high. Forward hull thrust depends on motor winding polarity and propellers. The 20 kHz / 10-bit PWM setup in `mdd20a_arduino_stage.cpp` is a code candidate, not physically validated; the independent actuator cutoff must still remove energy on physical STOP/watchdog faults.
