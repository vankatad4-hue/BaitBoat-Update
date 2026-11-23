# R30H-70 — STOP retry and concurrent handoff (development gate)

Date: 2026-11-23. Branch: `platformio-unified-r30h20`.

## Changes committed
Both `Boat/source/transport_task_queue.h` and `Remote/source/transport_task_queue.h` have identical R30H-70 revisions.

- STOP latches atomically before any mutex acquisition.
- STOP is offered to its own FreeRTOS one-slot overwrite queue without waiting for the ordinary queue mutex. A false result still requires caller retry and keeps STOP latched.
- Ordinary queue entries carry an epoch; STOP, clear, and authorized rearm invalidate old epochs.
- `receive(cmd, isStop, epochToken)` carries the epoch to the application; `executeIfCurrent(token, fn)` checks immediately before bounded dispatch.
- `clearStopLatchAfterAuthorization()` returns success/failure; caller must not assume rearm succeeded.
- `end()` is only valid after both tasks are stopped.

## Not yet validated / release blockers
This is **not** an end-to-end reliable STOP guarantee. The application still needs to wire authenticated STOP into the queue, implement retry on a false offer, enforce epoch at every actuator command execution boundary, and ensure that an atomic STOP arriving during an already-running output callback interrupts further output. A callback already performing a hardware write cannot be undone by the queue.

A STOP producer race also remains: concurrent STOP producers can overwrite one another; mandate a single serialized authenticated STOP owner or sequence-check before activating worker tasks. A pending STOP queue entry is not a correlated STOP ACK, a stopped STATUS or physical current-zero proof.

The legacy two-argument `receive()` remains for source compatibility and MUST NOT be used for actuator delivery. The new `executeIfCurrent()` callback must not block or re-enter the queue.

## Required executable tests before enabling Core 0 radio workers
1. Two ordinary producers plus one consumer; verify unique IDs, no duplicates and FIFO within each producer.
2. Force mutex contention while STOP arrives; verify STOP latch is immediate and STOP remains retrievable.
3. Dequeue ordinary command, then STOP; verify stale epoch cannot execute.
4. Race STOP against `executeIfCurrent()` and actuator write; verify output owner fails closed.
5. Repeated STOP producers and queue overwrite ordering; enforce single STOP owner or ordered sequence.
6. Queue full, mutex busy, startup allocation failure, session reset, authorized rearm and shutdown.
7. Validate command-ID-matched ACK then fresh primary stopped-STATUS; drop/reorder/duplicate packets.
8. Run ThreadSanitizer-capable host model or target FreeRTOS stress test, then both PlatformIO target builds.
9. Confirm upload remains denied, radio workers remain disabled and no live actuator tests are implied.

## Current verdict
**PARTIAL IMPLEMENTATION — NOT VALIDATED FOR CONCURRENT ACTUATOR DELIVERY.** No target compile, executable concurrency stress result, radio ACK test or physical test is claimed by this change. Keep Core 0 radio migration disabled. Official Boat VER014 / Remote VER028 unchanged.
