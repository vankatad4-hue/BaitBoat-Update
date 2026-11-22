# R30H-5/6 — GPS UTC on two ESP32-S3 endpoints; safe diagnostic marker readiness

**Engineering status (2026-11-22):** GPS-UTC software implemented and portable-host tested, physical GNSS/radio clock accuracy **NOT_MEASURED**. R30H-6 extends read-only quality reporting and an all-gates physical-marker allocation policy. Normal Boat/Remote RF initialization and actuator output remain locked pending actual hardware.

## End-to-end GPS wall-clock path

1. **Boat HGLRC M100 PRO GPS:** in `Boat/source/tracker_sensor_port.cpp`, inspected `tracker_commissioned_config.h` is needed before GNSS UART start. Firmware can read either verified NMEA at 9600 bps (GGA + GSA 3D + RMC with date) or UBX NAV-PVT at 115200 bps (checked validDate/time/fullyResolved, 3D fix and tAcc). The code does **not** automatically configure or probe both UART formats and does not assume the purchased module's output format. Both RTC and telemetry remain invalid if the GNSS is absent or the board profile unsigned.
2. **Time acquisition:** `Boat/source/gnss_utc_sync.h` validates calendar range 2020–2099, leap days, valid milliseconds, receiver validity, and a fresh 1500 ms timestamp. Source sample `localMs` is the Boat's **local monotonic reception tick**. Neither UART sentence-end arrival nor UBX packet completion is a GNSS PPS timing edge; end-to-end delay is not calibrated.
3. **Boat time discipline:** `flytec_time::Discipline` accepts two plausible consistent observations, rejects ±2 s outliers, invalid/stale time and sequence gaps, then updates **ESP32 CLOCK_REALTIME** using `settimeofday()` at most every 60 s. It never sets/steps `millis()` or motor/STOP/RTH watchdog clocks. In the default uncommissioned build `tracker_commissioned_config.h` is absent, so GNSS readout and wall-clock updates are not physically enabled.
4. **Authenticated radio UTC beacon:** `Boat/source/boat_dual_radio_port.cpp` sends a GPS UTC time estimate in versioned **TELEMETRY subtype 9**: `[0x09,0x01,unixSecondsLE32, millisecondsLE16]`. It is signed by the already-established application HMAC/session/epoch and sent only over nRF24 primary, best-effort at least 5 s apart, and **after** STOP, heartbeat, command traffic and R30G optional diagnostics. E22 airtime and emergency STOP priority are unchanged.
5. **Remote clock:** `Remote/source/boat_radio.cpp` accepts subtype-9 only after HMAC/session/sequence/primary-link validation. Two consistent estimates qualify UTC, then the Waveshare ESP32-S3 **CLOCK_REALTIME** is set via `settimeofday()` (up to once per minute). The `boatRadio_utcNow()` read-only API exposes UTC for a display or log. Subtype-9 **does not refresh** control-link freshness or STOP/mission/ACK state and does not unlock a radio that is already physically blocked.
6. **R30H-6 read-only quality:** `Remote/source/boat_radio.h` adds `boatRadio_utcQuality()` returning `ClockReport`: `INVALID` without fresh authenticated primary link, otherwise `AUTHENTICATED_REMOTE_COARSE` with UTC estimate and local sample age. Both endpoints have identical `r30h6_utc_marker_policy.h`; `traceReferenceCertified=false` and `rfEndToEndStopClock=false` by design. This information may label a displayed clock **„GPS UTC (приблизително)“**; never claim synchronized microsecond timestamps or certified STOP latency.

## What is actually synchronized

- The two microcontrollers' **civil UTC wall clocks** can converge to the same Boat GPS UTC source, when real modules, correct UART profile, GNSS fix and authenticated nRF24 are physically available.
- Their **monotonic MCU tick counters are NOT stepped, reset, or synchronized**. These are intentionally used for radio freshness (250 ms primary / 6000 ms backup), STOP leases and 10,000 ms conditional RTH since last authenticated Boat packet.
- Real remote-to-boat propagation time, PHY/SC16IS750 buffering, GNSS receiver timing/quantization, UART message delay, RTC oscillator drift and PPS phase are **unmeasured**. The present one-way authenticated UTC beacon alone cannot give independently verified submillisecond clock alignment or substitute for one external shared-oscillator logic analyzer.
- At loss of authenticated primary, Remote clock **quality is INVALID** even if a previously set RTC continues to tick. STOP failsafe cannot depend on the UTC wall clock. No fallback E22 UTC traffic is scheduled because E22 legal airtime headroom is unverified.

## Physical marker plan (R30H-6) — all unsigned

R30H-4 records ten candidate external analyzer probes; four Boat authenticated STOP/RTH state markers still have **no physical debug GPIO assigned**. `flytec_r30h6::MarkerReadiness` enforces seven necessary gates before a later hardware-probe design can be *considered*:

1. Actual ESP32-S3 and Waveshare board revisions inspected.
2. Chosen GPIO independently crosschecked against installed board harness/peripheral conflicts.
3. Marker is an output dedicated to **read-only diagnostic observation**, not motor/coil/STOP control.
4. Measured marker IO voltage and analyzer isolation are confirmed.
5. Boat's physical actuator HAL remains unattached and VM/VH disconnected.
6. Real common-clock external logic analyzer is present and tested.
7. Captured logs redact HMAC keys and other secrets.

No pin, ISR, GPIO write, electrical marker output, or actuator connection is created by the R30H-6 marker policy. **A true result from a host-only mocked MarkerReadiness is never physical acceptance.** R30H-4 signature requirements and all 22/16/10 physical RF/STOP gates are unchanged.

## Regression and next task

- Existing `Boat/tests/r30h5_gnss_utc_clock_test.cpp` exercises NMEA/UBX date and checksum, leap-year UTC, invalid fix, stale packets, subtype-9 encode/decode, drift/outlier and `millis()` rollover.
- Existing `Remote/tests/r30h5_utc_clock_test.cpp` tests paired beacon decode/discipline.
- New `Boat/tests/r30h6_utc_marker_policy_test.cpp` and `Remote/tests/r30h6_utc_marker_policy_test.cpp` test UTC quality, authenticated-primary gating, sample age, stale rejection, no physical latency authority and marker readiness refusing unsigned conditions. Host CI also compares the Boat/Remote wire UTC and marker policy headers byte-for-byte.

**Next R30H-7:** physically verify GPS UART profile and module revision, UTC PPS availability and two-radio RF-only source; if PPS is available, design and measure a disciplined timestamp/calibration path with an external common clock. Otherwise accept GPS UTC as coarse display/log time only. Finish actual STOP/current-zero acceptance before powering motor or solenoid outputs. No firmware official release promotion, no sonar implementation.
