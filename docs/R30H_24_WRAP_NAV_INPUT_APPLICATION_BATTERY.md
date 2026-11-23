# R30H-24 — Local navigation actuator gate, 32-bit monotonic rollover and repeated-battery RTH testing

**Date:** November 23, 2026  
**Development branch:** `platformio-unified-r30h20`  
**Code commit:** `1c21d511945c748ff7098cbb42da81b0c0adfad4`  
**Scope:** Logic-only C++ tests and conservative Boat fail-closed source fix. Hardware/antenna/GPIO/supplies are unchanged and **not approved**.

## Issue found and fixed

Previously, `BoatActuatorSupervisor::tick()` checked `nav.valid` and finite navigation error/distance, but did not independently check the contemporaneous local `ExecInputs.fix` fields immediately before producing `INTENT_NAV_TARGET` or `INTENT_NAV_HOME` motor demand. The caller typically computes navigation from validated local measurements; however, a cached or erroneously stale `MissionNavigationResult` could otherwise retain apparent authority at the last software boundary.

**Fix:** `Boat/source/boat_hardware_supervisor.h` now rejects autonomous motor demands when local GNSS freshness, heading freshness, geofence validity, positive finite positional accuracy, or in-range finite compass heading is missing. This re-check is applied **after** the explicit navigation-intent gate and before `nav.valid`; the independent mechanical/electrical watchdog and motor disable are still required. A valid primary operator's manual override follows its existing separate authorization, and no STOP/coil/backup radio privilege was expanded.

## Integrated regression matrix

| Simulated input | Required software outcome |
|---|---|
| `millis()` wraps while safetyAt is close to `UINT32_MAX` | Fresh until age 99 ms, unsafe at exactly 100 ms |
| Heading sensor crosses 300 ms age | Valid below 300 ms; rejected at 300 ms |
| GNSS sample crosses 2,000 ms age | Valid below 2,000 ms; invalid at 2,000 ms |
| Both radios stale across 10-second timeout and counter wrap | Conditional RTH gate respects elapsed time and requires Home/nav eligibility |
| Cached `nav.valid=true` but locally stale heading or GNSS | No autonomous motor demand, including locally authorized Home-only RTH |
| Invalid geofence / NaN accuracy / compass angle 360° | No autonomous motor demand |
| Fresh, valid local inputs and local Home-only authorization | Nonzero bounded navigation motor request allowed by *software*; never coil output |
| Manual stick lease expires at exactly 200 ms across rollover | Motors OFF, no stale stick replay |
| Backup-only heartbeat and restored primary after manual loss | Manual output OFF until a fresh neutral command then new stick input |
| Solenoid pulse crosses 32-bit rollover and configured duration | At most one active coil; at pulse deadline output OFF, separate post-OFF idle feedback required |
| Missing physical post-OFF idle feedback at 500-ms deadline | Latched STOP, unsuccessful release completion, no retry |
| nRF primary lost during active coil, backup healthy | Coil OFF, failed completion, STOP latched; same token cannot replay |
| 10.7 V sustained across counter wrap | RTH request latched at 3-second confirmation; no repeated Home requests |
| Repeated low 3S readings after transition to Home | Exactly **one** application RTH request, no discharge/release commands |
| New 9.8 V valid reading during Home | Immediate critical software STOP, no automatic recovery or actuator rearm |
| Invalid/stale local output/current feedback | STATUS remains UNKNOWN, never fabricated stopped/current-zero |

**Test sources:** `Boat/tests/boat_r30h24_wrap_failsafe_test.cpp` (direct production `BoatActuatorSupervisor`, `BoatBatteryMonitor`, `boatHardware_inputs`, `BoatRadioWatchdog` and `boatHardware_status`), and `Boat/tests/boat_r30h24_application_wrap_test.cpp` (actual `BoatApplicationPort.cpp` staged beside the already existing non-actuating `Boat/tests/application_stubs/boat_dual_radio_port.h`). Neither test invokes real SPI/UART/GPIO, and test values are **synthetic**, not calibrated voltage/radio or motor-current measurements.

## Source consistency and CI

The source auditor at `Boat/tools/verify_platformio_source_layout_r30h13.py` preserves the final actuator GNSS/heading/geofence gate. Mutation regressions in `Boat/tests/test_platformio_source_layout_r30h13.py` intentionally remove it and verify that the audit fails. An additional mutation test prevents silent removal of the single-RTH-request assertion from the adapter regression.

Run in VS Code terminal from the repository root:

```powershell
python tools/flytec.py audit
python tools/flytec.py test-host
python tools/flytec.py build-all
python tools/flytec.py snapshot
```

- [Boat host tests: PASS on source commit `1c21d511`](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45207416020)
- [Boat and Remote PlatformIO, exact source commit `1c21d511`](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45207416065) — check final CI conclusion
- [Version-frozen full project ZIP packaging](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/package-flytec-zips.yml)

## Remaining boundaries and next work

This is **software-only safety logic evidence**, not proof of actual boat motor-current zero, nRF/E22 field range/airtime, correct pin wiring, battery pack brownout stability, electrical watchdog, interlock sensor independence, or solenoid mechanical completion. Continue treating `Boat VER014` and `Remote VER028` as release-frozen pending physical commissioning.

Physical acceptance remains **R30H-10: 12 NOT_RUN / 6 BLOCKED; R30H-12: 7 NOT_RUN / 5 BLOCKED**. GPIO and upload are not enabled. No sonar implementation until the ready-made CHIRP model is selected.

**Next candidate R30H-25:** actual application-port isolated driver error injection, repeated hardware STOP-after-brownout and manual-neutral rearm across a simulated radio session rollover; compare against hardware gate lists before any build-ready signoff.
