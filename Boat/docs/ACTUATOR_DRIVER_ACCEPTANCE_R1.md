# Local actuator driver boundary and acceptance gates

The default Boat firmware instantiates the application bridge with **no physical driver attached**. It consumes radio diagnostics without execution. The named hardware driver contract is not a GPIO mapping, a safety certification, or permission to attach motor power.

## Interface
- `read`: acquire local GNSS, heading, interlock, physical STOP, battery, stationary and release-idle measurements with receipt timestamps. A 100 ms safety-sample limit, 2 s GNSS limit and 300 ms heading limit are provisional software engineering values. A watchdog-ready flag may be asserted only after an independent electrical cutoff is verified.
- `apply`: convert signed normalized motor requests to the commissioned differential driver and energize at most one release channel; enforce a **100 ms independent hardware output lease**, physical STOP, current and thermal protection. Use actual feedback to detect outputs off; writing a zero PWM value alone is not confirmation.
- `disable`: independent deenergization of propulsion and release power; failure shall not be masked by software.
- Configuration defaults disable output. All four solenoid pulse durations, safe motor demand and heading gain must be characterized on the actual boat. Sample test values are fixtures, not operating values.
- The bridge now consumes mission navigation/release intent and temporary override lease, with a tokenized post-pulse idle completion. Ordinary manual-release execution, final confirmed manual transition and status telemetry still require their physical feedback integration.
- No radio command can create or update local sensor timestamps. A lost or invalid trusted sample inhibits output and latches STOP.

## Required bench tests (not executed)
Use the exact selected Tracker revision, carrier PCB, motor driver and four coils. Disconnect propellers and high-current supply for early software tests. Verify mapped GPIO, boot reset levels, current limit, hardware STOP/cutoff, watchdog independent of firmware hang, four simultaneous requests, per-channel pulse duration and current, actual idle feedback, battery brownout, GNSS/heading loss, and radio interruption/rekey with reset. Only after bench acceptance perform restrained then supervised water testing.
