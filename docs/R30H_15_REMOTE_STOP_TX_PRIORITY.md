# R30H-15 — Remote STOP priority during an outstanding nRF24 TX

**Date:** 2026-11-23 | **Software-only candidate** | **VS Code + PlatformIO PRIMARY** | Boat **VER014** / Remote **VER028** unchanged.

## Fault found in the previous implementation

The Remote nRF24 driver accepts only one pending TX at a time. A physical STOP press that happened while the existing nRF frame was still transmitting called `radioSendTransport(LINK_NRF_PRIMARY, STOP)`, which returned **false** on `txPending_`. The Remote had an existing rate-limited pending STOP for LoRa, but lacked an equivalent **primary** STOP queue. If LoRa was unavailable, the STOP request could be lost from both radio command paths, although the Remote's local STOP confirmation latch remained asserted and the Boat had its independent radio-loss watchdog logic. This is a software safety defect, not evidence of a physical actuator event.

## Corrective implementation

- `Remote/source/r30h15_primary_stop_retry.h` contains a portable, single-slot `PrimaryStopRetry<BoatCommand>` state class: local STOP command + its stable logical `commandId` and monotonic enqueue time. Only STOP uses this queue; no stale motor/mission/release operation is stored.
- In `boatRadio_sendCommand()`, STOP is sent immediately on nRF if possible, otherwise **locally queued** if the primary transport is configured and an authenticated session is active. The existing E22 STOP path is **also attempted**, independently.
- In `boatRadio_update()`, `g_nrf.update()` processes the previous TX completion **before** any normal heartbeat opportunity. When the primary is ready and its TX slot is free, the queued STOP is attempted ahead of heartbeat and mission traffic. It retains the logical command identity while the existing radio encoder assigns a fresh authenticated transport sequence.
- Optional heartbeat traffic is skipped while a STOP is pending or nRF TX is busy; on successful *buffer admission* the local STOP queue clears. **Buffer admission is not RF delivery, authentication at Boat, execution, or physical current-zero proof.** Only Boat status/ACK may confirm execution.
- The STOP queue is cleared on authentication/session reset, nRF bridge fault, or after **2,500 ms** local queue age. It cannot migrate into a new session or silently replay indefinitely. The Remote's `g_stopMustConfirm` latch remains asserted until actual confirmation, and no ordinary movement/release command becomes authorized on a queue timeout.

## Regression and source-integrity checks

- `Remote/tests/r30h15_primary_stop_retry_test.cpp` runs entirely on a host compiler, no ESP/physical radio. It tests blocked TX, ready TX, no session, no transport, STOP supersession, 2,500 ms expiration, millis wrap, session reset, and that heartbeat is **not permitted** while STOP is pending.
- Both [Remote host CI](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/remote-host-tests.yml) and the [Primary PlatformIO Remote job](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/platformio-migration-build.yml) now compile and execute the portable test before firmware compilation.
- `Boat/tools/verify_platformio_source_layout_r30h13.py` checks that the source order remains **nRF update → pending STOP attempt → optional heartbeat**; a mutation test in `Boat/tests/test_platformio_source_layout_r30h13.py` intentionally swaps STOP and heartbeat to ensure the audit fails.
- Primary/backup command authentication, 32-byte frame, source/destination, CRC, replay guard, 250 kbps channel 76 nRF PHY profile, 10-second dual-radio RTH policy and all Boat output interlocks are **unchanged**.

## Physical R30H-10/R30H-12 handoff — NOT PERFORMED

Before the Radio/STOP interface can be approved, connect only **separately current-limited, non-actuating** prototype logic hardware with reviewed ready-made modules. Independently instrument Remote STOP button, outgoing nRF CE/CSN, actual on-air RX/ACK, Boat authenticated command receipt, and an independent Boat output-current-zero sensor using a *common* analyzer clock. Repeat 30 physical STOP trials under deliberately busy nRF TX, active SD card/UI/ADS1115 traffic, E22 pacing and dual-radio loss. Record raw captures and delay distributions in the existing R30H-12 evidence register (not synthetic data).

**This software queue cannot interrupt a blocking Wire I²C call, shorten an in-progress RF packet, prevent CE from being stuck HIGH during Remote reset or independently disconnect Boat actuator power.** These remain physical blockers. R30H-10 **12 NOT_RUN + 6 BLOCKED**, R30H-12 **7 NOT_RUN + 5 BLOCKED**. Radio audit flags stay zero; firmware uploads and motor/solenoid test activation remain denied.

## Commands (host source simulation; no hardware)

```sh
python Boat/tools/verify_platformio_source_layout_r30h13.py
python -m unittest discover -s Boat/tests -p test_platformio_source_layout_r30h13.py -v
python Boat/tools/verify_r30h10_physical_lock.py
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/source Remote/tests/r30h15_primary_stop_retry_test.cpp -o /tmp/remote_stop_retry
/tmp/remote_stop_retry
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

Check the **final GitHub Actions verdict for the latest commit**, not a prior commit; a host PASS/target PASS is evidence of software correctness only within tested paths.
