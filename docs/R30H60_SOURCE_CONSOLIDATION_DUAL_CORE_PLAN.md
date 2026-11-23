> **R30H-64 PREVIOUS-CODE AUDIT (2026-11-23):** R30H-63 Boat host [45259307468](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45259307468) PASS; Remote host [45259275547](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45259275547) PASS; source inventory [45259277539](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45259277539) PASS. Reviewed R30H-61/62 shared queue headers and R30H-63 firmware entry points. Corrected both FreeRTOS queue headers: removed unnecessary Arduino include, enforced trivially copyable command payloads and bounded depth, and repaired partially initialized queue handles before retry. Extended source-contract tests. **No cross-core RF task migration**; current radio and application objects remain under their existing owner. The previous PlatformIO run [45259290106](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45259290106) was still running when checked; do not infer result. Next: verify fresh target compile and integrate an explicit authenticated radio-to-app handoff without sharing LVGL/Wire/SPI. Official Boat VER014 / Remote VER028 unchanged.

> **R30H-63 (2026-11-23):** R30H-62 Boat host [45258999500](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45258999500) **PASS**; Remote host [45258963074](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45258963074) **PASS**. Added one-time `xPortGetCoreID()` and `portNUM_PROCESSORS` serial diagnostics in Boat and Remote setup/loop, plus host source-contract test in Boat CI. This establishes *observability*, **not** actual two-core application scheduling. Core-0 radio migration is intentionally withheld until radio objects and authenticated state have explicit single-owner handoff and STOP priority is validated; current radio, LVGL and actuator state remain single-owner. New PlatformIO/host CI pending. Official Boat VER014 / Remote VER028 unchanged.

> **R30H-62 (2026-11-23):** Boat and Remote R30H-61 host tests PASS ([Boat 45258758451](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45258758451), [Remote 45258770779](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45258770779)). Added identical `transport_task_queue.h` to Boat/Remote using FreeRTOS queues: bounded ordinary FIFO, dedicated single-entry STOP overwrite queue, STOP-first dequeue, nonblocking operations, session-clear API and startup allocation failure handling. Added source contract test to Boat host CI. **The queue is NOT connected to active radio/application loops yet; no task has been pinned to a core.** STOP commands must be authenticated before enqueue and must still pass the existing application policy; this is not a substitute for STOP ACK. Existing `transport_mailbox.h` remains single-owner only. Next R30H-63: establish authenticated radio producer/application consumer ownership and integrate queue in small steps, then verify actual dual-core runtime, latency and failover. Official Boat VER014 / Remote VER028 unchanged.

> **R30H-61 IMPLEMENTATION (2026-11-23):** Added identical `Boat/source/transport_mailbox.h` and `Remote/source/transport_mailbox.h`: allocation-free bounded FIFO with explicit overflow rejection/count, no silent overwrites. Added corresponding C++ host regression tests and wired them into both GitHub host workflows. **Important:** this primitive is *not thread-safe* and is **not yet wired to live RF tasks**; STOP must remain out-of-band. A FreeRTOS queue or critical-section-protected handoff is required before cross-core use. No dual-core task started; existing single-owner runtime remains unchanged. Prior PlatformIO Boat+Remote PASS [45256674129](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45256674129); new tests pending. Official Boat VER014 / Remote VER028 unchanged.

# R30H-60 — Source consolidation and dual-core rollout

## Cleanup evidence
- Removed `Remote/releases/CHANGELOG_MASTER_VER024.md` because its complete UTF-8 contents were confirmed to occur verbatim inside `Remote/releases/CHANGELOG_MASTER_VER026.md` before deletion. The Git history retains the original.
- Preserved VER021/022/023/025: their contents are **not** verbatim contained in VER026, so deleting them would risk historical data loss.
- Preserved all active `Boat/source` and `Remote/source` files: filename/age alone is not proof of dead code. The source audit and PlatformIO build depend on many revision-labelled headers.
- Preserved official VER014/VER028 ZIP releases. They are versioned deliverables, not safe duplicate deletions.

## Dual-core design gate (not yet enabled)
Both ESP32-S3 targets are dual-core capable. Currently the firmware executes application scheduling through Arduino `setup()/loop()`; ESP-IDF may use other core(s) internally. Do not describe this as a validated dual-core application.

**Incremental plan:**
1. Profile loop duration, STOP dispatch and ACK latency, LVGL tick/update time, and radio service intervals in software simulation; retain a single owner for each mutable object.
2. Move **only** Remote radio I/O into a core-0 FreeRTOS task after implementing a bounded queue for commands and a bounded queue/snapshot for authenticated telemetry. Keep all LVGL, display, SD/map, joystick and `boatState` mutation on the existing loop/core. Never invoke LVGL from the radio task.
3. Boat: move **only** radio receive/transmit into a core-0 task after adding explicit command/telemetry queues. Keep application actuator policy, GNSS, navigation and motor/solenoid control under one owner on the loop core.
4. Do not share Wire, SPI, LVGL, or radio transport objects across tasks without a documented owner/lock. STOP must preempt ordinary queued commands, with bounded latency and overflow behavior; no unbounded `portMAX_DELAY` on the STOP path.
5. Test core affinity and queue overflow in target build; test both-link loss >10 s, low battery RTH, STOP/ACK ordering, session rollover and reboot. Do not enable the split merely because it compiles.

**Current status:** architecture and cleanup only; no FreeRTOS application-task migration committed. Hardware acceptance remains pending. Dedicated hardware failsafe devices are out of scope.

## Verified baseline
- PlatformIO Boat+Remote PASS: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45256674129
- Remote host PASS: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45256674397
- Boat host PASS: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45256536346
- Source inventory PASS: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45256536337
- Official versions remain Boat VER014 / Remote VER028.
