# Boat R79 — low-battery warning and automatic Home diversion

Date: 2026-11-22  
Branch: `hardware-ready-dual-radio-r1`  
Status: implementation committed; host/target verification and physical acceptance tracked separately.

## Sensor path

The Boat reads its 3S battery pack using the existing I²C ADC acquisition path:

- ADS1115 module at I²C address 0x48, AIN0.
- Premade breakout powered at 3.3V.
- Candidate divider: 100kΩ battery-positive to AIN0, 33kΩ AIN0 to ground, both 1%; 100nF AIN0-to-ground.
- Divider ratio approximately 4.0303:1; 12.6V pack gives approximately 3.13V at AIN0.
- Candidate gain/offset is stored with the example profile uncommissioned. DMM calibration is mandatory before enabling the commissioned hardware profile.
- A valid battery sample must be no older than 1,000ms. Missing, invalid or stale data fails closed and stops propulsion.

The ADS1115 was already expected by the Boat's sensor-port ADC code but was omitted from the previous parts register. R20 adds it explicitly. The physical-driver battery-health gate now derives sample validity from this fresh ADC sample rather than requiring a separate unselected battery comparator. It is not an independent cutoff or safety controller.

## Thresholds and timing

The current R79 commissioning defaults for the 3S4P pack are provisional and must be calibrated against the actual cell model and pack sag:

| Pack voltage | State / action |
|---|---|
| Above 11.4V | Green / OK |
| 10.8–11.4V | Yellow / medium battery; recharge recommended |
| At or below 10.8V | Red / low battery warning; hold outbound mission movement |
| At or below 10.8V continuously for 3 seconds | Latch low-battery RTH request |
| At or below 10.2V continuously for 1 second | Latch critical STOP |
| At or below 9.9V | Immediate critical STOP |

A 3:1 IIR filter reduces short transients. A 200mV recovery hysteresis resets the confirmation timer before a latch. Once RTH is latched, it remains active even if voltage rebounds. Critical STOP overrides RTH and remains latched until the normal power-cycle/service recovery.

Estimated percentage uses a generic 3S Li-ion voltage curve. It is not coulomb counting and is not accurate enough to replace actual cell/pack calibration.

## Mission behavior

1. When the pack enters red, outbound autonomous motion is held immediately. Mission start/resume and new release requests are blocked.
2. After the 3-second confirmation, the Boat requests local low-battery RTH for an already-running eligible mission.
3. The outbound target and all remaining bait/hook release actions are discarded.
4. If a release pulse is active, the supervisor deenergizes the output and waits for a newer physical release-idle/current-zero feedback sample. If the feedback does not become safe, the boat remains STOP.
5. RTH is permitted only with:
   - valid CRC-checked Boat NVS Home matching the active mission Home;
   - fresh GNSS fix and heading;
   - valid navigation/geofence data;
   - battery above critical threshold;
   - physical STOP released, watchdog healthy, release interlocks valid and all release channels idle.
6. If any RTH gate fails, the application latches STOP. It does not continue outbound or guess a Home route.
7. During low-battery RTH, only Home-navigation motor intent is allowed; every hopper and hook output remains OFF. If a release was active, the output is deenergized and RTH waits for physical release-idle/current-zero feedback; if the selected hardware cannot provide that feedback, remain STOP. The existing radio-loss policy can still allow local Home navigation if both radios subsequently fail.
8. At Home, motors stop. The previous mission is not resumed.
9. If no autonomous mission is active, low battery is displayed as a warning and the operator must manually return the boat. The Boat does not synthesize a mission from a Home coordinate alone.
10. If battery reaches critical at any time, including during RTH, all motor and release outputs stop.

## Remote behavior

- Top-level Boat battery label shows voltage and estimated percentage.
- Green: above 50%; yellow: 21–50%; red: 20% or lower or Boat low-battery status flag.
- Yellow warning: `СРЕДНА БАТ. — ЗАРЕДИ`.
- Red warning: `НИСКА БАТ. — RTH`. Critical battery sets the separate status flag bit 7 and displays `КРИТ. БАТ. — STOP` (low/RTH remains bit 5).
- Stale/missing Boat battery telemetry shows `ЛОДКА: --` in muted grey.
- Mission preparation is blocked if battery telemetry is stale or estimated charge is 20% or lower; the Boat independently enforces the red-battery gate.

## Safety boundary

This is a local software policy on the Boat ESP32-S3, using a premade voltage ADC module. It is not a separate hardware cutoff and cannot protect against MCU lockup, loss of the MCU rail or failed motor-driver transistors. The existing independent physical STOP/watchdog/driver feedback wiring must be verified during commissioning.

### Latest interrupted-release protection

A red battery detected while a release is active immediately commands the EKM014 output OFF. The supervisor still advances its post-off validation while outbound motion is held: a strictly newer independent coil-current-zero sample permits safe Home diversion, whereas missing feedback for 500ms latches STOP. This was added after uncovering a state-machine corner case that previously left the release-idle gate permanently pending. A regression test covers both verified feedback and timeout-to-STOP.

A raw voltage sample at or below 9.9V bypasses the low-pass smoothing filter to ensure immediate STOP on sudden pack collapse; a 12.6V → 9.8V step has a regression test.

## Validation

- Portable battery policy tests cover percentage curve, filtering, yellow/red bands, 3-second RTH latch, critical STOP and stale-sample behavior.
- Mission tests cover outbound hold and Home diversion.
- Application bridge tests use simulated battery telemetry and dummy actuator outputs.
- ESP32 target compile and Remote target compile must pass after this change.
- Physical ADC calibration, actual radio behavior, dummy-load outputs and controlled water test remain pending.

Do not connect real motors or solenoids until [the R79 dummy-load procedure](BOAT_R79_LOW_BATTERY_BENCH_TEST.md) is passed.
