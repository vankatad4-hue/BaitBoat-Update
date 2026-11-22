# R30H-7 — GNSS UTC validity, physical PPS verification and STOP-timer isolation

**Status: source correction and logic-only tests implemented; real GNSS / PPS / RF clock measurements NOT_RUN.** This phase does not change the Boat 3S4P common bus, actuators, nRF/E22 authentication, STOP or 10-second RTH policy.

## Current UTC synchronization already implemented (R30H-5/6)

- Boat HGLRC M100 PRO → `TrackerSensorPort` GNSS parser: either an inspected 9600-baud NMEA stream (RMC UTC date plus GGA/GSA fix) or an inspected 115200-baud UBX NAV-PVT stream with valid time and 3D position. The actual module's revision/output mode and PPS pin remain uninspected. Firmware does **not** auto-detect both simultaneously.
- `Boat/source/firmware/Flytec_Boat.ino` disciplines the Boat ESP32-S3 **CLOCK_REALTIME** from two coherent accepted UTC samples with `settimeofday()`, only if a physically reviewed sensor configuration exists.
- Authenticated nRF24 TELEMETRY subtype 9 (8 bytes: type/version/unix seconds/milliseconds) sends Boat GPS UTC as low-priority best-effort traffic. Remote verifies HMAC, session epoch, sequence and primary origin, qualifies two coherent readings and updates its ESP32-S3 **CLOCK_REALTIME**. E22 STOP priority/airtime unchanged.
- `millis()` and all monotonic link leases, safety watchdogs, STOP events, and conditional last-authenticated-Boat-RX + 10,000 ms RTH remain independent of the UTC wall clock. A successful `settimeofday()` does **not** align two unrelated local microsecond counters.

## R30H-7 actual corrections

1. In `Boat/source/tracker_gnss.h`, `utcSnapshot()` rejects NMEA UTC when the last valid GGA position timestamp (`positionUtc_`) is from a *different GNSS second* than the valid RMC date/time (`rmcUtc_`). Previously a stale-but-under-2s GGA could qualify an RMC date for the next second; a 1-ms-sampling test could miss this. A new regression rejects mismatched epoch and confirms recovery only when a matching GGA sentence arrives.
2. In `Remote/source/boat_radio.cpp`, public `boatRadio_utcNow()` now returns false if the authenticated primary link is stale, even when the ESP RTC retained a previously written time. Its quality API likewise reports INVALID. This affects the informational clock only and does not change radio freshness or motor authority.
3. `Boat/tools/verify_r30h_radio_preflight.py` and its negative-mutation tests prevent regression of the GNSS epoch guard, loss of authenticated-primary freshness on Remote UTC, UTC codec divergence, or manufactured PPS/current-zero claims.
4. [12 real-hardware GNSS/PPS/clock-offset gates](BOAT_R30H_7_GNSS_PPS_RF_CLOCK_SIGNOFF.csv) are **12 NOT_RUN**, with blank observations and reviewer signoff. No PPS input pin has been guessed or connected. Physical RF/STOP timing still requires independent common-clock evidence.

## Proposed physical verification, when actual components are available

- Verify M100 PRO SKU/board marking, GNSS UART voltage level and actual 9600-NMEA or 115200-UBX output before setting the UART profile. Capture RMC/GGA/GSA or UBX NAV-PVT with checksum and sample date, fix validity and accuracy. Do not sign off GNSS time on inferred baud alone.
- Identify whether a physical 1PPS/timepulse pin exists **on the purchased module and carrier**. If not present, do not invent one. If present, verify voltage, pin conflicts and PPS-to-UTC epoch relationship using the **same real external logic-analyzer clock** at the Boat.
- On the actual paired radios, capture source GNSS time arrival, the transmission trigger, Remote receive and observed CLK_REALTIME update while keeping MDD20A VM and EKM014 VH physically disconnected. The one-way beacon has variable GNSS UART and RF latency and cannot prove precise sub-millisecond synchronization. Record maximum/median clock offset only against a true shared physical reference and include oscillator drift/temperature.
- Deliberately interrupt GNSS, primary nRF24 and both radios, then restore them. Verify UTC becomes unavailable/stale without granting STOP release, lease extension, fresh telemetry, mission permissions or RTH re-arming. Verify STOP/RTH intervals use independent monotonic timers on **real** ESP32 silicon.
- No external diagnostic GPIO/test marker may be connected without the unsigned R30H-4 and R30H-6 seven prerequisite hardware conditions. There is no currently commissioned authenticated STOP, motor-current-zero or RTH marker.

## What remains blocked

The R30H-7 12 physical time gates, R30H-4 10 analyzer probes, R30H-3 16 STOP acceptance gates, R30H-1 22 RF module gates and original R30H 22 RF cases remain unverified; R30A independent STOP and solenoid current-zero hardware gating remain OPEN. Current E22 STOP pacing (3s) and backup freshness (6s) remain **unmeasured physical latency risks**. Boat VER014 / Remote VER028 remain frozen. Sonar selection deferred.

**Next engineering phase R30H-8:** obtain real module/antenna/logic-voltage/PPS provenance, perform RF-only common-clock characterization and use verified observations to decide whether a physical PPS/clock-offset calibration module is warranted. Do not claim actual synchronization accuracy or safe physical STOP latency from software simulations.

## R30H-7 Remote UI feedback (verified in host tests)

The existing authenticated-only `boatRadio_utcQuality()` is now consumed by `Remote/source/boat_ui.cpp`. A read-only line in the handheld header shows `GPS UTC: hh:mm:ss ~` while the Boat-origin time source is qualified, and `GPS UTC: --:--:--` when unavailable, stale, unpaired or primary-radio quality is lost. The trailing `~` explicitly means **coarse / uncalibrated transport latency**, not a synchronized physical stopwatch. The visible time is UTC, not Bulgarian local EET/EEST, so no daylight-saving assumption is hidden in the telemetry protocol.

A portable, allocation-free formatting helper `Remote/source/r30h7_utc_clock_ui.h` and `Remote/tests/r30h7_utc_clock_ui_test.cpp` reject invalid quality, stale samples, impossible UNIX timestamps, incorrectly asserted physical-certification flags, and undersized label buffers. [Remote host #45056813182](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056813182) **SUCCESS** with the new R30H-7 label test. [Boat host #45056804415](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056804415) **SUCCESS** with R30H-7 NMEA matching-epoch and physical-gate checks.

The Remote ESP32-S3 full target compile of the newly changed UI must be confirmed separately; a host formatter test alone cannot prove LVGL layout and actual display visibility. No GNSS, RF or physical clock-offset test has been performed, and every one of the twelve R30H-7 physical signoffs remains `NOT_RUN`.
