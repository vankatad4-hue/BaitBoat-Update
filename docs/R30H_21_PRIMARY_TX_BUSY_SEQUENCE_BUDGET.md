# R30H-21 — STOP/STATUS priority and primary nRF sequence-budget safety

**Date:** 2026-11-23  
**Development branch:** `platformio-unified-r30h20`  
**Status:** source implementation and host/CI regression; **not** an RF or actuator acceptance.

## Root cause under heavy primary-TX contention

Boat's `BoatDualRadioPort::sendHeartbeat()` retried overdue 100 ms heartbeats every main radio update. Before R30H-21, its code built and authenticated a new frame with `++txSequence_` **before** `sendFrame()` checked whether a STOP ACK or Boat STATUS held the primary transmitter. Under sustained queue contention, a large number of rejected heartbeat attempts could burn the finite 16-bit sequence budget (`RadioSessionCounter::REKEY_AT=30000`) without an RF transmission, causing an unnecessary key/session reset. The same pattern existed in other ordinary frame builders.

This is a software sequence-accounting defect, *not* a measured radio problem. STOP itself remained latched; the risk was loss of control/telemetry continuity and delayed confirmation due to avoidable rekey.

## Actual code changes

1. New portable `Boat/source/boat_r30h21_primary_tx_budget.h` defines the ordinary primary TX admission contract: authenticated session ready, radio ready, TX idle, no STOP ACK pending, no authoritative STATUS pending.
2. `BoatDualRadioPort::primaryOrdinaryTxReady()` binds that contract to actual Boat radio state. `sendHeartbeat()`, `sendMissionFeedback()`, noncritical `sendHardwareTelemetry()` and primary *non-STOP* `sendCommandAck()` check it **before allocating a sequence ID**.
3. Prioritized STOP ACK transmission continues through its existing bounded `stopAck_` path (eight attempts; 75 ms minimum gap; 2500 ms total age). Its TX admission is never treated as Remote receipt.
4. Current `statusOutbox_` still holds one freshest **locally sampled** physical safety/output-feedback status, expires after 300 ms and is transmitted after STOP ACK, before ordinary traffic. Unknown status cannot serve as stopped evidence.
5. E22 backup and its availability/radio-loss watchdog are not modified. The update cannot enable any GPIO, motor, coil, real RF board or firmware flashing.

## Regression evidence

`Boat/tests/boat_r30h21_primary_tx_priority_test.cpp` verifies all 32 combinations of admission-state flags, 50,000 synthetic scheduler iterations while STOP/TX-busy is asserted, ACK RF failure with 75 ms retry, priority of unknown STOP STATUS, new STOP supersession, 5000 ms controlled busy-channel scheduling and 30,000-sequence rekey budget preservation, and purging outboxes on session failure. The 50,000 iterations are **NOT physical milliseconds** and do not represent real RF throughput or measured STOP latency.

`tests/r30h20_radio_frame_fault_matrix_test.cpp` now includes an additional **1,024-frame** authenticated stream: deliberately lost heartbeats, duplicate/replayed and out-of-order packets, STOP STATUS arriving before ACK, a matching accepted ACK, an unknown STATUS and subsequently fresh stopped primary STATUS. Compile the same test against **each endpoint's own** real `radio_protocol.cpp` and codec, with the host-only OpenSSL-backed HMAC shim. Production targets continue to use mbedTLS.

Source audits explicitly enforce TX priority and check that the primary heartbeat admission guard precedes the actual frame constructor. Mutation tests purposefully remove the guard and STOP/STATUS pending constraints and verify that the audit rejects those modifications.

### Run from repository root in VS Code terminal

```powershell
python tools/flytec.py audit
python tools/flytec.py test-host
python tools/flytec.py build-all
python tools/flytec.py snapshot
```

GitHub Actions: [Boat host](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/boat-host-tests.yml), [source audit](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/source-audit.yml), [PlatformIO both targets](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/platformio-migration-build.yml), [complete project ZIP](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/package-flytec-zips.yml). Do **not** cite earlier commits as proof of a later commit; verify exact SHA and CI status.

## Physical blockers / handoff

No hardware is commissioned. R30H-10 **12 NOT_RUN / 6 BLOCKED**; R30H-12 **7 NOT_RUN / 5 BLOCKED**. STOP/current-zero independence, actual RF24 TX contention, bridge CE/CSN reset behavior, nRF/E22 reconnection, GPS clock synchronization and solenoid/motor power isolation still require **measured physical** acceptance. No upload, actuator power, official release or declared measured STOP latency. Boat **VER014**, Remote **VER028** remain frozen.

**Next software task:** run a non-actuating, two-endpoint simulated STOP-to-ACK-to-feedback exchange with explicit failure injection and signed/seq-checked frames at actual scheduler boundaries, distinct from isolated portable queue/protocol tests. Do not invent GPIO/RF metrics.
