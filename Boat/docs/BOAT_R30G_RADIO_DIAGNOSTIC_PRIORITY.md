# R30G — nRF priority, bounded optional diagnostics and simulated link loss

**Date:** 2026-11-22. **Scope:** Boat optional R30F subtype-8 transmit budget, host-only workload simulation and no-control-freshness assertions. No Remote radio protocol, sonar, custom PCB, physical hardware or power selection changed. The owner-selected **one common 3S4P pack** remains planned for both MDD20A VM and EKM014 VH in parallel.

## Actual contention found

The selected nRF24 `NRF24Link::sendFrame` has **one nonblocking in-flight TX slot**. A second call fails while `txPending_` is set. Before R30G, the non-actuating Boat firmware called `sendHardwareTelemetry` for the optional R30F diagnostic every 2 seconds directly before `BoatApplicationPort::tick` ran the radio update. A diagnostic TX could use the only slot needed for a subsequent 100ms heartbeat or mission feedback. Real RF timing has not been measured.

## Actual implemented mitigation

- New `Boat/source/boat_r30g_optional_diag_budget.h`: allocation-free, **one replaceable eight-byte payload**. There is **no FIFO backlog**. A new observation overwrites an older one. Any observation over **500ms** old is discarded and cannot be replayed after session loss/recovery. Successful diagnostic TX attempts are rate-limited to **one per 2000ms**, regardless of how often the diagnostic is offered.
- `BoatDualRadioPort::sendHardwareTelemetry` treats **ONLY `RADIO_PAYLOAD_SAFETY_DIAGNOSTIC`** as this best-effort offer; other STATUS, BATTERY, NAV and WEATHER payloads retain their existing direct send behavior. A successful offer return value **does not claim RF delivery** or create an ACK. Invalid subtype-8 payloads are not offered. An unready or stale primary session discards the offer.
- `BoatDualRadioPort::update` continues to service primary/backup RX, watchdog, conditional Home, command queue, mission feedback and both heartbeat schedules **before** considering one optional diagnostic frame. Sending the diagnostic requires a fresh authenticated primary session, nRF **not** TX-pending, no queued command, no mission/override/return-home/failsafe, no valid command or mission-stick activity within **250ms**, and a previously successful nRF heartbeat **15–40ms** earlier. No diagnostic is ever sent on E22 backup.
- The 100ms primary heartbeat timer advances only when `sendHeartbeat` actually **starts** a TX. A busy nRF no longer makes the software falsely claim it sent a heartbeat. The next tick can retry the heartbeat before any optional diagnostics.
- Only when these conditions hold does the Boat construct/sign a subtype-8 radio frame and attempt a nonblocking nRF `startWrite`. A failed attempt drops the record; it is **not retried/requeued**. Radio frame size, HMAC/session, sequence limits, command types and existing physical STOP/failsafe behavior remain unchanged.
- Both `resetSession` and acceptance of a fresh authenticated handshake explicitly **clear diagnostic offers**, command-activity state and the prior-heartbeat indication. Link loss or a stale authentication state discards the pending offer.
- The Remote R30F decoder continues to treat diagnostic subtype-8 as informational only. It cannot refresh authenticated-control freshness, STOP confirmation, normal battery/mission timestamps, or `actuatorSafetyReady`. Its diagnostic status expires after 6000ms and is hidden on loss of the primary link.

## Tests and boundaries

`Boat/tests/boat_r30g_optional_diag_budget_test.cpp` covers no-heartbeat, heartbeat TX busy/early/due, pending STOP/command, active mission, recent manual control, RTH/link loss, unverified session, primary timeout, replacement of obsolete observation, 500ms expiry, 2s TX pacing, underlying TX failure, authenticated session reset, `uint32_t` wrap, sustained manual control and resumption after idle.

`Boat/tools/verify_r30g_optional_diag_priority.py` checks that the real Boat radio source uses that bounded scheduler, that `update` schedules optional TX **after** mission/heartbeat work, that the R30F strict authenticated primary-only path is still present on both endpoints, that the Remote excludes subtype-8 from control-link freshness, and that all hardware commissioning/safety gates remain false and **24 OPEN**. GitHub host CI runs the test and audit; target CI compiles full ESP32-S3 diagnostic after the firmware-source changes.

**What these tests do NOT prove:** a hard real-time guarantee on nRF packet latency, RF retries or measured STOP responsiveness. The single-slot radio hardware may still delay a frame briefly once a diagnostic TX has started. That timing requires a real paired nRF rig, a logic analyzer and physical acceptance. This policy is intentionally aggressive about dropping diagnostic data whenever there is any doubt.

**Power/physical status:** R29 common VM/VH power unchanged; no motor/solenoid power connected, no commissioned R30D feedback reader, no independent hardwired STOP/DC rated common disconnect/watchdog/current-zero hardware yet. `COMMON_BUS_HARDWARE_COMMISSIONED=false`, `ACTUAL_READER_COMMISSIONED=false`, `INA231_PHYSICAL_COMMISSIONED=false`. R30A **24** physical readiness gates OPEN, R23/R25/R27 **81 NOT_RUN**. Official Boat **VER014** and Remote **VER028** remain frozen; CHIRP sonar deliberately deferred.

**Next engineering action:** verify live GitHub Boat host CI and ESP32-S3 target compile for R30G, resolve any errors, and review Remote behavior when optional diagnostic observations are lost for more than 6s. No physical hardware permission is granted by software-only tests.

## R30G observed verification — 2026-11-22

- [Boat portable host Actions #45049989270](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45049989270): **SUCCESS**, including the new R30G C++11 one-slot scheduler / heartbeat / command / mission / link-loss / timestamp-rollover simulation, the R30G source ordering audit, the updated R30F protocol audit and all pre-existing Boat host regressions.
- [Boat ESP32-S3 target compile Actions #45049610592](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45049610592): **SUCCESS**, compiled the actual R30G Boat source and the locked INA231 diagnostic alongside the non-actuating main Boat diagnostic. Subsequent changes relevant to the target were to tests, static validators and this documentation only.
- Earlier failing host runs #45049572006–#45049733913 were **intermediate CI consistency/test-fixture failures**: the old R30F static audit checked a now-retired inline send expression, and the first R30G host test incorrectly expected the slot to survive a deliberately fail-closed link-loss condition. Updated audit now verifies the deferred authenticated send path; test now expects discard after link loss. The full host workflow later succeeded.
- These results show code-level low-priority scheduling and compatibility. **Physical nRF24 preemption latency, measured STOP response, RF interference/loss and independent DC safety have NOT been tested**. Remote source and R30F authenticated protocol are unchanged in R30G; previous [Remote host #45048847021](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048847021) and [Waveshare Remote target #45048618202](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048618202) remain the latest relevant Remote software builds.

**Release status:** R30G engineering iteration only; Boat VER014 / Remote VER028 unchanged; one 3S4P battery common VM/VH architecture and all 24 physical gate states unchanged.
