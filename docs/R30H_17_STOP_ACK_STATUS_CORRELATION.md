# R30H-17 — correlated STOP acceptance and authoritative feedback

**Date:** 2026-11-23. **Development target:** VS Code + PlatformIO, source-only. **Official releases:** Boat VER014, Remote VER028 unchanged. **Physical test gates:** R30H-10 12 NOT_RUN / 6 BLOCKED; R30H-12 7 NOT_RUN / 5 BLOCKED.

## Confirmed source defect

In R30H-16 a Remote nRF24 STOP was retried until any authenticated primary Boat STATUS reported `stopState=1`. Such a STATUS could have been created **before the latest STOP request** and arrive later, falsely documenting that specific request as acknowledged. A separate source-only risk existed after the 8-transmission retry cap: heartbeat remained suppressed even though no further primary STOP admission would be attempted; this could starve Boat of its normal primary freshness and prevent another authoritative status from being sent.

## Source-only correction

**New file `Remote/source/r30h17_stop_confirmation_gate.h`:** a portable, hardware-free `StopConfirmationGate` keeps the last 16-bit STOP `commandId` and whether that exact request received an `ACCEPTED` or `EXECUTED` ACK through the existing authenticated, session/sequence-checked `RadioAckTracker`. Each new local STOP request resets the evidence, even if radio output is disabled. A wrong ID, rejected/BUSY ACK or old-session ACK cannot satisfy it.

**Integration `Remote/source/boat_radio.cpp`:** only an authenticated, sequence-accepted **matching STOP ACK** followed by a separate valid **primary Boat STATUS** with `stopState==1` can clear the pending-request latch and primary STOP retry queue. Existing baseline behavior with **no new local STOP request** still allows initial authenticated primary STOP STATUS to establish the remote's safe boot state. Session reset flushes both STOP retry and correlation evidence. Status `stopState=1` reflects the Boat's software-supervised *fresh local stopped/output-feedback status*, not independently measured physical current-zero; neither ACK ACCEPTED nor a TX admission alone means execution.

**`Remote/source/r30h15_primary_stop_retry.h`:** preserves the 8 transmissions, 150 ms minimum spacing and 2.5 s maximum local queue age from R30H-16. While eligible to retry, primary STOP outranks normal heartbeat. **After all 8 transmissions have been admitted**, periodic heartbeat can resume to maintain a healthy radio RX/control path and give Boat an opportunity to send STOP feedback; it does not clear the `g_stopMustConfirm` latch or re-authorize motors. The same source checks and `pio upload` denial remain in force.

## Repeatable host verification

- `Remote/tests/r30h17_stop_confirmation_gate_test.cpp`: baseline boot, primary-only confirmation, new STOP invalidating old status, wrong ID, rejected ACK, accepted matching ACK, status-after-ACK, new STOP superseding old evidence, session reset, 16-bit over-air command ID handling, command ID zero denial.
- `Remote/tests/r30h15_primary_stop_retry_test.cpp`: retry limits, backoff, no heartbeat during active retries, heartbeat after budget exhaustion, no heartbeat while TX busy, wrap, age and session reset.
- Host steps added to `.github/workflows/remote-host-tests.yml` and the main PlatformIO workflow. Shell continuation regression corrected. `Boat/tools/verify_platformio_source_layout_r30h13.py` audits source invariants, including STOP-before-heartbeat ordering and exact correlated ACK/STATUS gate; `Boat/tests/test_platformio_source_layout_r30h13.py` includes mutation regression. Main PlatformIO workflow now triggers when the source auditor or its mutation tests change.

Commands from repository root (host tests and **compile-only**, no RF hardware):

```bash
python Boat/tools/verify_platformio_source_layout_r30h13.py
python -m unittest discover -s Boat/tests -p test_platformio_source_layout_r30h13.py -v
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/source Remote/tests/r30h17_stop_confirmation_gate_test.cpp -o /tmp/r30h17_stop_gate
/tmp/r30h17_stop_gate
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/source Remote/tests/r30h15_primary_stop_retry_test.cpp -o /tmp/r30h17_stop_retry
/tmp/r30h17_stop_retry
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

**Evidence policy:** check Actions results for the **latest commit**, not an earlier run or an in-progress attempt. In particular initial R30H-17 CI attempts failed due to a double shell escape and a stale source-audit literal; both defects were addressed in subsequent commits. Do not count failed/obsolete workflow runs as passing evidence.

## Remaining physical blockers and tradeoffs

This confirmation policy is deliberately fail-closed if a valid matching STOP ACK is lost despite the Boat already having stopped. Boat currently emits an **admission ACK**, not a sensor-verified STOP execution ACK, so a separate post-ACK primary stopped-status is mandatory. A missing ACK may require another explicit STOP request; local motion remains denied. Hard independent current-zero / solenoid-off supervision, reset-safe CE idle LOW, actual RF24 + E22 interoperation, I²C blocking-time, real STOP timing and physical GPIO/power acceptance remain **NOT_RUN/BLOCKED**. The source-only 150 ms backoff is not an RF or actuator timing measurement.

**Next design review:** prioritize reliable Boat-side STOP acceptance feedback under nRF TX-busy, using a tightly bounded response queue and explicit output-feedback criteria. Only validate on non-actuating host simulations until interlock modules and physical evidence exist.
