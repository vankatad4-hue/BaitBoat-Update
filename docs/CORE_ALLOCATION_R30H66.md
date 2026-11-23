> **R30H-69 (2026-11-23):** Both Boat/Remote `transport_task_queue.h` now serialize ordinary queue operations, STOP overwrite/reset, dequeue and authorized rearm using a FreeRTOS mutex. Atomic STOP latch is set before attempting the mutex; on lock contention, STOP admission **fails closed and must be retried by its caller**. All queue operations use zero-timeout mutex acquisition; this is not yet a proven end-to-end STOP delivery guarantee. Queue `end()` requires stopped worker tasks (never destroy queues while active). Source-contract test extended. **Not yet integrated into radio tasks; already-dequeued commands still require application-level epoch enforcement.** Physical/target concurrency and STOP ACK tests remain pending. Official Boat VER014 / Remote VER028 unchanged.

> **R30H-67 (2026-11-23):** Extended identical Boat/Remote `transport_task_queue.h` with priority STOP admission that resets the pending ordinary-command FIFO and sets an atomic STOP latch. `offer()` and ordinary `receive()` reject work while latched. Only an application-authorized `clearStopLatchAfterAuthorization()` may rearm the queue; `clear()` on session reset remains latched. Source-contract regression extended. **Limitation:** this queue is not connected to live RF/application tasks, and a command already dequeued before STOP still requires an application-level epoch check before actuation. The present design does not yet prove race-free STOP ordering under concurrent producers; full integration and target tests remain pending. No hardware failsafe additions. Official Boat VER014 / Remote VER028 unchanged.

# R30H-66 — ESP32-S3 core ownership and migration acceptance

## Current verified source behavior
Both Boat and Remote use Arduino `setup()/loop()` as their single application execution path. `core0_probe.h` contains an opt-in, self-terminating task pinned to Core 0; `FLYTEC_ENABLE_CORE0_PROBE` defaults to **0**. Neither endpoint currently has a live RF worker task pinned to Core 0. `xPortGetCoreID()` diagnostics report the **actual** Arduino loop core when run on the device. The host build does not prove the runtime core number.

## Intended ownership after migration

| Core | Boat | Remote |
| --- | --- | --- |
| Core 0 (pinned task) | Authenticated nRF24/E22 radio I/O, session management, RX validation, ACK/telemetry transmit | Authenticated nRF24/E22 radio I/O, session management, STOP/ACK handling, telemetry receive |
| Core 1 (Arduino loop, subject to on-device verification) | GNSS/IMU, navigation/autopilot, motor and solenoid command policy, battery monitoring, software STOP/RTH state | LVGL, display/touch, SD/maps, joystick/physical buttons, command generation, UI state |
| Crossing | Bounded FreeRTOS queue of validated commands and status snapshots; dedicated STOP-priority channel | Bounded outbound command queue, STOP-priority channel and immutable inbound telemetry snapshots |

## Non-negotiable software contracts
- Only the radio worker may own and call RF24/E22/SC16IS750/SC18IS606 transports after migration; no direct loop-thread calls to those instances.
- LVGL/display/SD, I2C/SPI buses and shared `boatState` remain single-owner unless a documented mutex/queue handoff is introduced.
- Authenticate and sequence-check STOP **before** inserting it into the priority queue. A queue flag is not authorization.
- A STOP event must invalidate/drain previously queued motion/release commands; subsequent commands require explicit existing STOP-clear policy. This is not yet implemented.
- Preserve STOP ACK and subsequent STOP status; do not infer success from queue admission alone.
- Never block STOP on a full ordinary queue; bound latency under primary/backup loss, session rollover and queue saturation.
- Start tasks only when both queues and transport drivers initialize; failure leaves current single-loop behavior and disabled outputs, not a partial dual-core mode.
- Software-only STOP/RTH is the chosen design. No dedicated hardware failsafe components or hardware safety tests are introduced.

## R30H-66 next integration gate
1. Capture baseline radio `begin/update/send` call graphs on both endpoints, including callback entry points.
2. Introduce a *single* worker-owned RF interface and immutable command/telemetry DTOs.
3. Add tests for queue overflow, STOP stale-command invalidation, ACK ordering, 10-second both-radio loss and LoRa recovery.
4. Compile Boat and Remote, run portable host tests, then enable the pinned RF worker behind a feature flag only after ownership tests pass.
5. On physical hardware, log both core IDs and measure STOP and telemetry latency; no physical measurements are claimed here.

**Release policy:** Boat VER014 / Remote VER028 remain unchanged until physical acceptance.
