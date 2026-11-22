# R30H-8 — shared-clock GPS/PPS versus Boat/Remote UTC calibration readiness

**Date:** 2026-11-22. **Implemented:** read-only common-instrument time-analysis tool, hostile unit tests, empty evidence templates, unsigned physical gates, CI. **Actual PPS, GNSS, radio, MCU marker timing: NOT_RUN.**

## Current time synchronization remains unchanged

The Boat HGLRC M100 PRO GNSS, once correctly wired and independently commissioned, can supply checked NMEA UTC (RMC/GGA/GSA) or UBX NAV-PVT UTC. The Boat's existing `flytec_time::Discipline` writes **CLOCK_REALTIME** with `settimeofday()` after consistent samples. An optional low-priority HMAC-authenticated nRF24 telemetry subtype 9 shares that UTC with the Remote Waveshare ESP32-S3, which independently qualifies samples and updates its own CLOCK_REALTIME. Remote shows approximately synchronized UTC (`~`) only while the authenticated primary link is fresh.

Neither ESP32 `millis()` / monotonic hardware counter is set or aligned by this operation. STOP, session freshness, command leases, watchdog and conditional RTH are explicitly independent of wall-clock UTC. **No GPS/PPS, radio or hardware clock offset was actually measured** and the physical M100 module's PPS/timepulse output has not yet been identified.

## New R30H-8 evidence and analysis

- [PPS/ESP clock candidate analyzer](../tools/analyze_r30h8_shared_clock.py)
- [Empty instrument CSV template](BOAT_R30H_8_SHARED_CLOCK_CAPTURE_TEMPLATE.csv)
- [Blank UNSPECIFIED manifest](BOAT_R30H_8_SHARED_CLOCK_MANIFEST_TEMPLATE.json)
- [Eight R30H-8 physical signoffs](BOAT_R30H_8_CLOCK_ALIGNMENT_GATES.csv): **8 NOT_RUN**, observations/evidence/reviewer all empty
- [Adversarial host-only simulator tests](../tests/test_r30h8_shared_clock.py)
- Updated [Boat source/physical signoff preflight](../tools/verify_r30h_radio_preflight.py): rejects committed synthetic/physical data in the authoritative blank templates, forged PPS evidence and prematurely signed gates

### Acquisition model

Use **one actual external instrument** to timestamp three physically observed edges for each UTC second/epoch:

| Required event | Where it must be captured | Must be independently established |
|---|---|---|
| `GNSS_PPS_EDGE` | PPS/timepulse output on inspected real GNSS module | Whether pin physically exists, voltage/polarity, exact **UTC second** association, external trigger and pulse stability |
| `BOAT_RTC_SAMPLE_EDGE` | Safe, dedicated Boat diagnostic marker | The firmware's atomically sampled `CLOCK_REALTIME` value **for that exact marker**, bounded read-to-edge latency and GPIO voltage |
| `REMOTE_RTC_SAMPLE_EDGE` | Safe, dedicated Remote diagnostic marker | Same atomic UTC-to-edge association and delay evidence, with no Waveshare LCD/radio bootstrapping pin conflict |

**These three physical probes are NOT presently available**: there is no commissioned PPS pin or assigned Boat/Remote marker GPIO. A USB line from either ESP32 cannot be substituted for an edge, because serial buffering creates non-deterministic latency. The analyzer cannot detect forgery or substitute for independently verified probe signals.

CSV fields: `capture_id,trial_id,clock_domain,time_us,platform,event,utc_unix_ms,origin,notes`. The `time_us` value must be an integer microsecond reading **from the same independent sample clock**. `utc_unix_ms` on PPS is the known whole UTC second for the pulse; on Boat/Remote it is an actual UTC sample atomically associated with the respective marker. Use a single trial ID for the three edges within **one PPS second**. Mixed origins (`USB_LOCAL`, manually synchronized host USB timestamps) are invalid.

The analyzer computes candidate UTC offsets using **integer microsecond arithmetic**, conditioned on the PPS epoch and marker-to-RTC capture latency having been verified:

```text
boat_offset_us   = 1000*(boat_rtc_unix_ms - pps_unix_ms)
                   - (boat_edge_us - pps_edge_us)
remote_offset_us = 1000*(remote_rtc_unix_ms - pps_unix_ms)
                   - (remote_edge_us - pps_edge_us)
remote_minus_boat_us = remote_offset_us - boat_offset_us
drift_candidate_ppm = 1e6*(offset_last_us - offset_first_us)
                      / (pps_edge_last_us - pps_edge_first_us)
```

A positive offset means the **reported** device UTC runs ahead of the PPS-anchored time if all physical timing associations are correct. Results are *candidates*, not certified oscillator calibration. A packet's HMAC does not identify its flight time, correct PPS phase or timing of the ESP32 clock sample.

### Required hardware metadata

The JSON manifest must only be filled after actual instrument capture. It requires a raw CSV SHA-256, instrument model/asset ID/sample rate, Boat and Remote 40-hex firmware commits, inspected module revisions, physically verified PPS UTC-second relation and edge polarity, both marker GPIOs and their measured logic levels, calibrated read-to-edge latency, independently reviewed RF/antenna state, disconnected common-bus **MDD20A VM / EKM014 VH**, and a reviewer. Fake self-declared fields never create a PASS.

Usage on Linux or Windows where Python 3 is installed:

```bash
python3 -m unittest Boat/tests/test_r30h8_shared_clock.py -v
python3 Boat/tools/analyze_r30h8_shared_clock.py \
  Boat/docs/BOAT_R30H_8_SHARED_CLOCK_CAPTURE_TEMPLATE.csv \
  Boat/docs/BOAT_R30H_8_SHARED_CLOCK_MANIFEST_TEMPLATE.json
# Actual hardware: use your independently captured CSV and reviewed manifest instead.
```

**Exit code semantics (deliberately no exit 0):**

- `3` — `NOT_RUN`: empty template, no PPS/clock measurements
- `2` — `INVALID`: wrong/missing epoch or edge, duplicate sources, uncalibrated/missing provenance or incomplete trial
- `4` — `SIMULATED_NONCERTIFIABLE` or `EXTERNAL_CAPTURE_REVIEW_REQUIRED`, even with a superficially complete manifest

The physical-signoff worksheet is **never automatically edited by the analyzer**. The measured-offset numerical summary is only meaningful after independent manual waveform and calibration review.

## Safety and hardware blockers remain open

The 12 R30H-7 GPS/PPS physical gates, 10 R30H-4 probe mappings, 16 R30H-3 STOP acceptance gates, 22 R30H-1 radio module gates, 22 original R30H radio tests and R30A independent STOP/one-hot/zero-current gates are all unsigned/unrun. Source-only testing proves neither GPS PPS exists on the selected M100 carrier nor that valid antenna/legal EU868 configuration, STOP delivery, or motor current zero is assured.

No new board, custom PCB, radio, PPS GPIO, debug marker firmware, motor/solenoid power or GPS configuration override is introduced in R30H-8.

**Next R30H-9 (software-only until hardware arrives):** make non-actuating recorder requirements and reviewed GPIO allocations ready for the actual inspected board revision; after obtaining hardware, capture radio-only PPS/UTC offset and STOP events with the same external timebase before any motion or water acceptance. Boat VER014 / Remote VER028 remain frozen.
