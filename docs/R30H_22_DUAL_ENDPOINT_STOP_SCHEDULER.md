# R30H-22 — Real-codec two-endpoint STOP / ACK / STATUS scheduler fault injection

**Date:** 2026-11-23  
**Working branch:** `platformio-unified-r30h20`  
**Scope:** Pure host simulation; real endpoint 32-byte wire codec and existing production priority/retry/gate classes; **no hardware / radio / GPIO / current-zero evidence**.

## Why this is the next test

R30H-17 through R30H-21 separately proved STOP ACK correlation, bounded STOP command retries, Boat TX-busy ACK scheduling, newest physical STATUS scheduling, HMAC/CRC/replay checks and primary sequence-accounting under busy TX. They did not yet run a single deterministic timeline including **both** endpoints, a radio queue, STOP command loss, Boat ACK loss and local safety/status feedback.

This new test composes actual existing production objects rather than reimplementing their policy:

- Remote: `flytec_r30h15::PrimaryStopRetry<BoatCommand>`, `RadioAckTracker`, `flytec_r30h17::StopConfirmationGate`
- Boat: `boat_r30h18::StopAckOutbox`, `boat_r30h19::StatusOutbox`, `boat_r30h21::ordinarySlotFree`
- Both: real `radio_protocol.cpp` 32-byte HMAC/CRC, `radio_command_codec.cpp`, and payload codecs/encoders, separately compiled against Boat and Remote source trees. Actual `RadioSessionCounter` guards replay.
- A discrete 25-ms **simulated** scheduler tick handles Remote STOP priority before heartbeat, Boat RX/STOP ACK first, newest STATUS next, then ordinary heartbeat. Injected physical STOP-status samples are explicitly controlled test inputs, **not** inferred from STOP command delivery.

Host OpenSSL is used only to model the target's HMAC-SHA256 mbedTLS API via the existing test-only `tests/r30h20_stubs/mbedtls/md.h` shim. Target PlatformIO firmware continues using real mbedTLS.

## Tested cases

| Test case | Required outcome |
|---|---|
| Normal STOP / ACK / physical sampled stopped-STATUS | Logical STOP confirmation only after matched ACK and fresh stopped-STATUS |
| Remote nRF busy at startup, Boat TX busy during ACK, transient contention | No false confirmation; retries and latest status eventually confirm |
| First STOP, ACK and STATUS lost | Same logical command ID re-sent; later valid ACK and fresh STATUS can confirm |
| Boat ACK RF TX failure | ACK outbox retries after minimum 75 ms; no acknowledgement inferred from TX admission |
| Physical output feedback UNKNOWN, later stopped | UNKNOWN never confirms; later real input may confirm |
| Every STOP command dropped | Eight Remote admission attempts at most; STOP stays unresolved |
| Every Boat ACK dropped | Stopped STATUS alone cannot release Remote STOP latch |
| Every Boat STATUS dropped | ACK alone cannot release Remote STOP latch |
| Physical output feedback always UNKNOWN | STOP confirmation stays unresolved |
| Remote nRF TX busy for longer than queue deadline | Expire pending queue; no fake retry/heartbeat while actual PHY busy |
| Duplicate ACK after confirmed STOP; new command ID; new epoch/key | Old ACK and old session cannot confirm a new STOP |

The simulator also checks the Remote ACK tracker **timeouts** after 2500 ms, Boat ACK bounded retry count, STOP latch persistence, and that ordinary heartbeat scheduling is subordinate to higher-priority STOP traffic. Time values are test inputs, **not measured RF response time**. It does not exercise `BoatDualRadioPort::update()` or `boatRadio_update()` directly, and therefore cannot prove scheduling jitter, real RF transmission latency, non-stalling I²C bridge calls, independently supervised physical STOP, driver-current zero or actuator power cutoff.

## Cross-endpoint header inclusion

`Boat/source/radio_payload_codec.h` and `Remote/source/radio_payload_codec.h` share the same `RadioDecodedPayload` enum/struct ABI but intentionally declare slightly different helper functions. Their duplicate portable declarations conflicted when the test composed the actual Boat outbox and Remote ACK tracker in a single translation unit. Both headers now have a matching `FLYTEC_RADIO_PAYLOAD_CODEC_SHARED_TYPES_H` guard. No on-wire field or radio binary layout was changed. Both endpoint target builds remain separately compiled.

## Build and CI

From the VS Code project root:

```powershell
python tools/flytec.py audit
python tools/flytec.py test-host
python tools/flytec.py build-all
python tools/flytec.py snapshot
```

CI workflows now compile and run `tests/r30h22_dual_stop_scheduler_test.cpp` against **Boat and Remote codecs individually** in host jobs and in the dual target migration workflow. Tests have been marked PASS on source commit `e1475248`:

- [Boat host tests](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45201870070)
- [Remote host tests](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45201870153)
- [Source inventory / cross-endpoint audit](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45201870177)
- [Boat + Remote PlatformIO builds](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45201870051) — check the **last** CI conclusion before stating a PASS.

The package workflow publishes separate Boat/Remote source snapshots and a complete tracked-project ZIP with SHA-256, **not** an officially signed firmware release.

## Remaining blockers and follow-on work

Physical acceptance remains **R30H-10 12 NOT_RUN / 6 BLOCKED** and **R30H-12 7 NOT_RUN / 5 BLOCKED**. All live GPIO/antenna/supply/bridge admission flags stay disabled; no motors or solenoids are energized; no firmware upload authorization. Boat official **VER014**, Remote official **VER028** are frozen.

**Next:** incorporate bounded, independently measured hardware-only test vectors when modules are assembled: handshake re-establishment, dual radio-loss RTH/STOP and independently monitored motor/coil STOP and current-zero. Until then continue non-actuating simulation with physical safety interlock failures and runtime/clock-staleness input injection. Do not treat source-only ACK/STATUS as a physical safety certification.
