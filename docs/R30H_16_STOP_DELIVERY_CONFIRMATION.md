# R30H-16 — bounded nRF24 STOP retransmission until Boat confirms STOP

**Updated:** 2026-11-23. **Evidence class:** source-only host regression and diagnostic PlatformIO compilation. The firmware remains uncommissioned, with all physical RF, GPIO and independent STOP/current-zero gates unsigned. **Primary development environment: VS Code + PlatformIO.**

## Why R30H-15 needed a second correction

R30H-15 successfully retained a STOP that arrived while nRF24 was TX-busy. But the Remote then discarded the STOP queue as soon as `g_nrf.sendFrame()` returned **true**, even though that merely means the driver accepted a payload for radio transmission. A lost on-air frame, lost hardware ACK, or absent Boat status was not retried. Software admission must never be confused with real Boat STOP execution.

## Implemented source changes

The existing `Remote/source/r30h15_primary_stop_retry.h` (retained filename for compatibility) now manages **up to eight primary nRF admissions**, at **least 150 ms apart**, within a **2,500 ms maximum local age**. Admission increments a counter without clearing the queue. The logical command ID is unchanged across attempts; the authenticated transport sequence is regenerated normally by `radioSendTransport()`.

`Remote/source/boat_radio.cpp` now:
1. Enqueues a new authenticated STOP intent *before* the immediate nRF attempt, and independently tries the backup E22 STOP path. A failed nRF buffer admission leaves the STOP waiting; no ordinary motor/mission/release command enters the queue.
2. Calls `g_nrf.update()` at the start of the radio service step and attempts pending STOP **ahead of nRF heartbeat and mission traffic**, if the slot is idle, the session is valid and the retry/backoff/age limits permit.
3. Retains the primary STOP queue after nRF buffer admission and after an application `RADIO_ACK_ACCEPTED`. A valid **authenticated, session-bound, sequence-accepted and telemetry-decoded Boat STATUS with `stopState==1`** is needed to clear the queue and `g_stopMustConfirm`. This is **logical STOP status**, not a measured actuator power cut.
4. Clears queued primary STOP intent if the session resets, primary transport faults or the time window expires. The **Remote confirmation latch remains fail-closed** even if the queue expires without valid Boat STOP status.
5. Preserves all existing signed/unsigned radio restrictions, no direct Waveshare nRF CE/CSN GPIO, LoRa pacing, mission safety and RTH logic. It does **not** initiate RF or control actuators on unsigned builds.

The existing Remote `radio_ack_tracker` is not treated as proof of STOP execution: `ACCEPTED` can mean Boat command admission and must not clear the safety latch.

## Regression tests and repeatability

`Remote/tests/r30h15_primary_stop_retry_test.cpp` tests pending-TX exclusion, no session/transport, initial nRF admission, retransmit after 150 ms, zero retransmit before the gap, no heartbeat while waiting for status, command ID stability, transmission cap of 8, 2,500 ms expiry, newer STOP supersession, session flush, and `millis()` wrap. The host fake performs **no radio transmission, SPI/I²C or GPIO output**.

`Boat/tools/verify_platformio_source_layout_r30h13.py` and its mutation tests protect the source order `radio.update() → queued STOP retry → heartbeat`, verify that successful re-admission calls `noteAdmitted` rather than `reset`, check R30H-14 nRF PHY profile and enforce all physical RF audit flags remaining zero.

Run from repository root:

```sh
python Boat/tools/verify_platformio_source_layout_r30h13.py
python -m unittest discover -s Boat/tests -p test_platformio_source_layout_r30h13.py -v
python Boat/tools/verify_r30h10_physical_lock.py
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/source Remote/tests/r30h15_primary_stop_retry_test.cpp -o /tmp/r30h16_stop_host
/tmp/r30h16_stop_host
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

**Verified code-commit host evidence:** Remote host [#45133874907](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45133874907) **PASS**, Boat host [#45133874972](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45133874972) **PASS**, source inventory [#45133874893](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45133874893) **PASS**. Primary [PlatformIO diagnostic build #45133874886](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45133874886) and Remote Arduino [reference #45133874948](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45133874948) must be checked for final verdict before claiming successful target compilation for this specific revision.

## Remaining acceptance requirements

- R30H-10 original 18 physical gates remain **12 NOT_RUN and 6 BLOCKED**.
- R30H-12 twelve bridge/STOP physical measurements remain **7 NOT_RUN and 5 BLOCKED**.
- No physical STOP current-zero detection, no independent motor/solenoid cutoff, no verified Remote CE reset LOW, no RF analyzer timing, no measured I²C/Wire blocking limit or actual nRF/LoRa interoperability. Neither a source-only 150 ms retry interval nor a target PASS is a measured STOP latency.
- STOP retry timing is subject to real I²C/SPI/OS latency; physical STOP must be validated across busy RF24, SD, UI and E22 traffic, real input loss and board resets with hardware interlocks before motors/solenoids can be enabled.
- **Boat VER014 / Remote VER028 remain frozen**, diagnostic build only. Development ZIPs may auto-package, but no official release promotion and no upload/actuator enable.

**Next engineering hinge:** review actual STOP retry/ACK semantics on coupled physical radios with common-clock capture; a confirmed hardwired current-zero stop and reset-safe CE are still engineering blockers. Sonar remains deferred pending module selection.
