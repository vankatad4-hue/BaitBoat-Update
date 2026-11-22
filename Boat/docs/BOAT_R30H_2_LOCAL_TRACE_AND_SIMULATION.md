# R30H-2 — Boat-local RF/STOP/RTH trace, with non-actuating timing simulations

**Date:** 2026-11-22. **Status:** Engineering implementation and host tests completed; physical radio, STOP current and RTH navigation evidence **NOT_RUN**. Target-compile evidence is tracked in GitHub Actions. Boat VER014 / Remote VER028 unchanged.

## What was added

- `Boat/source/boat_r30h2_local_trace.h` — fixed 32-record event ring with bounded, O(1) insertion and retrieval; overflow counted (never overwrites oldest safety event). No allocations, no radio payloads, no keys, no packet authentication modifications, no actuator output.
- `Boat/source/boat_dual_radio_port.cpp/.h` — records after successful handshake and accepted payload authentication: sampled primary (every ~1 s) and backup heartbeats, `BOTH_LINKS_STALE`, `AUTH_LINK_RESTORED`, `STOP_AUTH_RECEIVED`, `STOP_LOGICAL_INTENT`, and `RTH_LOCAL_ENTRY`. Trace bookkeeping cannot change freshness, mission eligibility, STOP latch or outputs.
- `Boat/source/firmware/Flytec_Boat.ino` — **only in the non-actuating diagnostic target**, drains at most one event per application loop over local USB serial and only if at least 120 bytes are available for nonblocking reporting. It does not print inside receive/watchdog/STOP paths. Corrected actual newline delimiters for existing R30E serial rows as well as R30H2 rows.
- `Boat/source/boat_radio_hw_config.h` — `BOAT_R30H_LOCAL_TRACE_ENABLED=0` by default; optional source variant build sets it to 1 **only in its isolated temporary CI directory**. Signed Boat nRF and E22 RF flags remain 0, so it cannot perform actual RF transmission without additional independent electrical signoff.
- `Boat/tests/boat_r30h2_local_trace_test.cpp` — real C++11 host tests for ring overflow/drop, event order, millis 250/6000/10000 boundaries, Home invalid, and recovery resetting the timer. Pure software, not radio/actuator acceptance.
- `Boat/tools/analyze_r30h2_boat_local_trace.py` with `Boat/tests/test_r30h2_boat_local_trace.py` — validates the USB-only CSV marker `INTENT_ONLY`, rejects bad/overflowed records and RTH occurring before 10,000 ms from last accepted authenticated packet. Reports only local STOP-auth-to-intent and local stale-to-RTH intervals; never asserts physical STOP/RF E2E timing.
- `Boat/tools/verify_r30h_radio_preflight.py` and mutation tests — verify signoff remains locked, trace is disabled in ordinary build, no Serial I/O in the radio hot path and simulator does not attach a physical driver.
- CI: Boat host workflow runs trace C++ and Python tests, and Boat ESP32-S3 target workflow additionally compiles an **isolated, RF-locked nonactuating optional USB trace variant**; it never promotes it to a release binary.

## Event format / how to use when actual logic modules exist

```text
R30H2,BOAT_LOCAL,timestamp_ms,last_accepted_rx_ms,event,link,fresh_mask,command_id,overflows,INTENT_ONLY
```

Events `SESSION_ESTABLISHED`, `AUTH_PRIMARY_SAMPLE`, `AUTH_BACKUP_SAMPLE`, `BOTH_LINKS_STALE`, `AUTH_LINK_RESTORED`, `STOP_AUTH_RECEIVED`, `STOP_LOGICAL_INTENT`, `RTH_LOCAL_ENTRY` are *BOAT-LOCAL observed firmware events*. Link is 0/1/2 (none/nRF/E22). Fresh mask bit 0=nRF, bit 1=E22. `last_accepted_rx_ms` is the Boat's last **accepted and authenticated** radio-frame timestamp, not the Remote transmit time.

After collecting a USB text log **with actual purchased, reviewed, provisioned radio modules**, run:

```bash
python3 Boat/tools/analyze_r30h2_boat_local_trace.py boat_usb_serial.txt
```

A blank file gives `NOT_RUN` (exit 3); overflow, mismatched STOP, early RTH, forged physical label or inconsistent timing gives a nonzero result. `BOAT_LOCAL_OBSERVATIONS_ONLY` means purely local observations and **not** approved latency. The original [R30H 22-case matrix](BOAT_R30H_RF_TEST_MATRIX.csv) remains all `NOT_RUN`. To compare Remote STOP-button time to Boat physical current-zero, use a **shared external hardware clock/logical analyzer**, not a subtraction of unrelated USB logs.

A firmware-level USB time stamp cannot reveal the actual RF-block instant; record `RF_BLOCKED` externally. `STOP_LOGICAL_INTENT` is not a confirmed MDD20A output transition, solenoid current-zero or independent safety switch. In the uncommissioned sketch, no physical driver is attached, and no outputs are energized.

## Software boundary result, not a measured RF number

The code uses `primaryFresh = age <= 250ms` and `backupFresh = age <= 6000ms`. Therefore a physical RF outage immediately after a legitimate E22 frame can leave the **backup link fresh until the first application tick after 6000ms**. This is an engineering **worst-case timing exposure from software constants**, not a measured latency. The RTH eligibility helper separately requires `age >= 10000ms`, both links stale, an eligible previously started mission, matching CRC-valid Boat Home, valid navigation and physical/sensor safety interlocks. There must be **no RTH at 9,999ms** from last accepted authenticated packet.

With a healthy, authenticated E22-only connection, backup authority cannot issue manual motor commands or releases. Actual backup STOP delivery may be delayed by 3000ms pacing and E22 UART TX; no hardware measurements or signoff exist. Do not automatically shrink E22 freshness or change STOP arbitration until the E22's true airtime/PHY, packet loss and safety response are characterized. **The ~6s software window and ~3s pacing remain open safety-review issues** before live propulsion.

## Physical readiness and next stage

- [R30H-1 22-item physical module signoff](BOAT_R30H_1_RF_MODULE_SIGNOFF.csv): **all NOT_RUN**.
- [R30H 22-case latency/STOP/RTH matrix](BOAT_R30H_RF_TEST_MATRIX.csv): **all NOT_RUN**.
- R30A common-3S STOP, watchdog and one-hot release/current-zero external safety, plus physical level/antenna/logic gating, remain **OPEN**. Boat INA231 BUS-only voltage sensing and R79 low-battery behavior remain unchanged. No additional PCB or modules selected.
- **Next R30H-3**: use purchased and documented nRF24/E22 hardware only after successful voltage/I/O/antenna/profile audit; run isolated RF-only authenticated pairing, capture USB-local events and external shared-clock reference, determine actual E22 worst-case STOP delay, then revise unsafe polling/freshness/pacing as needed. Do not connect motors or releases for this stage.

**Release/CI rule:** CI verifies source consistency, simulated timing policy and target compilation — not real RF airtime, mechanical motion, water readiness, physical current-zero, or a safe independent hardware cutoff.
