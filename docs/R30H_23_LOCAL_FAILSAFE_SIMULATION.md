# R30H-23 — integrated dual-radio-loss, 10-second RTH and local safety simulations

**Date:** 2026-11-23  
**Branch:** `platformio-unified-r30h20`  
**Scope:** Non-actuating portable host tests of **existing production Boat safety classes**, not radio or physical STOP certification.

## Real software modules exercised

- `BoatRadioWatchdog`, `boatRadioLossRthDue`, `boatEvaluateLinkFailsafe`: source `Boat/source/boat_radio_safety.h/.cpp`; two authenticated radios, immediate software STOP on complete signal loss, and the separate 10,000-ms RTH qualification.
- `BoatMissionController` and `MissionExecution`: actual prepared/outbound plan and irreversible Home diversion with Home-only navigation and no release replay.
- `BoatActuatorSupervisor` and `boatHardware_inputs`: fresh local safety sample, output feedback, interlocks, primary-only coil release, primary-only manual lease, 100-ms sample freshness, 300-ms heading and 2,000-ms GNSS freshness.
- `BoatBatteryMonitor` and `boat_r30b::inspectBoatBattery`: a 3S4P shared supply, 3,000-ms low-battery RTH confirmation, critical undervoltage immediate STOP, stale/invalid INA231 evidence.
- `boatHomePointValid` / matching mission Home and `boatHardware_status` / wire STATUS decoder: no RTH with absent Home and no stopped feedback inferred from requested commands.

The test file is `Boat/tests/boat_r30h23_failsafe_integration_test.cpp`. The harness constructs **injected** trusted/untrusted measurements and steps the actual software state machines; it never opens a real SPI/UART link, drives GPIO, or energizes an actuator.

## Simulated fault matrix and required behavior

| Scenario | Expected software outcome |
|---|---|
| Both radios lost after authenticated mission | Actuator demands OFF immediately, outbound plan held |
| 9,999 ms since last authenticated frame | No Home permission |
| 10,000 ms, both stale, stored Home and valid GNSS/heading/battery/interlocks | One-way Home diversion; only INTENT_NAV_HOME permitted, coils remain OFF |
| Primary restored during local Home | Continue Home-only, not former target and never automatic manual rearm |
| Backup remains fresh | No dual-loss RTH timer action; may sustain outbound navigation but not release authorization |
| No prior authenticated packet or a fresh later frame | No premature RTH; timer measured from latest accepted authenticated frame |
| Missing/mismatched Home, invalid GPS fix, stale GNSS/heading, invalid accuracy/geofence, low electrical confidence, STOP active, stale safety sample, invalid output feedback, release busy | Deny Home even after 10 seconds (15 independent negative tests) |
| Seven distinct local navigation/interlock failures injected after Home entry | No motor demand on failing input |
| Low battery 10.7 V sustained for 3 seconds | Stop outbound and allow only independently qualified Home diversion |
| Abrupt valid 9.8 V battery reading | Latched critical STOP, no automatic clear after nominal voltage |
| Stale or invalid battery conversion | Deny Home / actuator authority |
| Active solenoid interrupted by both-link loss | Cancel output immediately; failed completion, STOP latch, no replay after RF recovery |
| STOP already commanded but motor feedback nonzero, feedback invalid or watchdog unavailable | Report STATUS UNKNOWN instead of claiming stopped |
| Unsigned `millis()` rollover | 10-second gate time arithmetic remains correct |

**Do not interpret the 10-second counter as measured RF latency.** It is a deterministic local software timer from the last accepted authenticated link event. Real antenna/RF24/LoRa response, power rail brownout, actuator current-zero, and GNSS quality under motion remain **not measured**.

## Source/CI verification

A static source audit and mutation tests lock three critical source constraints:

1. RTH elapsed-time condition uses unsigned `nowMs-lastAuthenticatedRxMs` and does not bypass the 10,000-ms threshold.
2. With both radios stale and without locally authorized Home, Boat actuator execution demands zero output.
3. Shared-bus battery inspection rejects critical energy evidence rather than granting Home.

Run locally from the VS Code + PlatformIO repository workspace:

```powershell
python tools/flytec.py audit
python tools/flytec.py test-host
python tools/flytec.py build-all
python tools/flytec.py snapshot
```

The Boat host workflow and Boat cell of the PlatformIO matrix both compile the test with `g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror`. The ordinary R30H-22 dual-controller STOP simulator, Remote host matrix and both firmware targets are retained.

- [Boat host CI](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45205207217)
- [Boat + Remote PlatformIO compile](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45205207291)
- [Source audits](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/source-audit.yml)
- [Version-frozen project archive](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/package-flytec-zips.yml)

These URLs are evidence of **specific commits** and require checking completion status before claiming a final PASS. The latest documentation-only commit does not change executable firmware.

## Unresolved hardware acceptance

**Boat official VER014 / Remote official VER028 remain frozen.** Hardware R30H-10 **12 NOT_RUN / 6 BLOCKED** and R30H-12 **7 NOT_RUN / 5 BLOCKED**. The project has no measured STOP-to-current-zero, genuine redundant RF timing, module wiring acceptance, verified EN/watchdog cutoff or assembled Boat/Remote prototype. All GPIO hardware admission locks and firmware uploads remain disabled until physical signoff.

**Next software handoff, R30H-24:** stress the interaction of hardware-sample ages and mission/control timestamps with `millis()` wrap, backup-only interruption, repeated low-battery alerts and release timeouts in the application adapter. That test must not weaken the existing STOP, mission and release permissions.
