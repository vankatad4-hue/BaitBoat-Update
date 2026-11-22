# R30H-1 — inspection, RF commissioning prerequisites and critical bridge fix

**Status:** Engineering code corrected and checklist prepared; **22/22 physical signoffs NOT_RUN**. This is not a request to connect motors, release coils, high-current VM/VH or to test on water. Boat VER014 and Remote VER028 remain unchanged.

## Exact verified software problem, fix and hardware boundary

**Problem:** Remote `SC16IS750Bridge::writeFifo()` treated NXP TXLVL register `0x08` as bytes occupied, computing `64 - txlvl`. In SC16IS750, TXLVL is **bytes FREE**: an empty 64-byte FIFO reads `0x40`, a full FIFO `0x00`. The old code calculated `0` available bytes for an empty FIFO, preventing ordinary writes to E22; `flush()` had the same inversion. `Remote/source/sc16is750_bridge.cpp` now uses `r30h_sc16::freeTxSlots(level)` and `r30h_sc16::txFifoEmpty(level)` from `Remote/source/sc16is750_fifo_budget_r30h.h`. Invalid register values fail closed. `Remote/tests/sc16is750_fifo_budget_r30h_test.cpp` exercises full/empty/partial/invalid cases. **This is source-level/host verification until the SC16IS750 carrier is physically tested.**

NXP TXLVL reference (register section 8.14): https://www.alldatasheet.jp/html-pdf/347835/NXP/SC16IS750/1978/32/SC16IS750.html

**Potential RF-budget mismatch to investigate before enabling E22:** Current firmware's explicit profile includes E22 `REG0=0x67` (9600 8N1 UART, air data rate **62.5 kbps** according to the legacy E22-900T22S register encoding), `REG1=0xC3` (sub-packet 32 bytes), and `channel=0x12` (channel 18, expected 868.125 MHz for original E22). The source assumes `EST_AIRTIME_MS=25` and `MAX_AIRTIME_PER_HOUR_MS=36000` (1% software budget); **25 ms is only an unverified estimate**, not a measured upper bound and not a legal declaration. RF preamble, FEC, packet airtime, antenna/EIRP and regional channel eligibility must be measured/checked against the **exact purchased suffix, including whether it is legacy or -V2**, rather than blindly trusting the hardcoded bytes. The backup's 3-second heartbeat and STOP arbitration may need a safe redesign after measurement.

Legacy E22 register reference: https://manuals.plus/m/dd2d8b19e8808eda853c8d50afa587ec2e4e5bf606dc9319e7ae1416213891e6

## What is locked at build-time

- **Boat nRF:** `BOAT_NRF_PIN_AUDIT_SIGNED=0` unchanged. **Boat E22:** existing pin/PHY/level gates 0 plus new `BOAT_E22_RF_AIRTIME_AUDIT_SIGNED=0`; UART radio initialization does not run under default firmware.
- **Remote nRF:** new `REMOTE_NRF_PIN_AUDIT_SIGNED=0` blocks provisional Waveshare GPIO6/15 physical radio start. **Remote E22:** `REMOTE_E22_BRIDGE_AUDIT_SIGNED=0`, `REMOTE_E22_RF_INTEROPERABILITY_VERIFIED=0` and `REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED=0` block SC16IS750/E22 hardware initialization until physical proof. This also prevents silent TX on an unverified 868MHz profile.
- **Actuator authority:** All previous R30A/R30C/R30D physical common-bus, coil-limiter, coil-current-zero and independent STOP gates remain false; no output-driving HAL is attached to `Boat/source/firmware/Flytec_Boat.ino`. Neither RF regression nor successful target compilation can sign these.
- **Waveshare peripheral collision:** Selected Standard Waveshare board carries GPIO6 Sensor AD and GPIO15 RS485 TXD; repurposing CSN or CE is permitted only after photographed board revision, electrical multiplexer audit and proof the original driver cannot drive the line. Do **not** assume the 4.3B/4.3C schematics match the selected Standard PCB.

## Bench plan, in strict order

1. **Unpowered inspection:** identify exact serial/SKU and revision of both ESP32 boards, two nRF24 PA/LNA boards and two E22 modules; verify nRF 2x4 orientation, E22 26-pad carrier pinout, connector polarities and usable antennas. Compare with R23/R30A.
2. **Logic-only power:** bench current-limited 3.3V and 5V rails, measure E22 5V-powered TXD/AUX and Remote SC16IS750 3.3V I2C pullups; prove no ESP32 sees 5V signals; isolate MDD20A VM and EKM014 VH completely.
3. **Remote bridge:** scan I2C for expected 0x48 with the **actual** oscillator/crystal. Check SC16IS750 TXLVL=0x40 at empty, FCR reset, 9600 8N1; send 32 bytes into TX FIFO and verify UART waveform/flush on oscilloscope or logic analyzer, with E22 RF disabled during this subtest. Record actual serial latency.
4. **E22:** with appropriate RF load/enclosure and local regulatory controls, verify both suffixes; compare 9-byte C1 readbacks and typed configuration command. Measure actual over-air frame airtime and conservative worst case; calculate duty-cycle for both endpoints over ≥1-hour worst-case schedule, including 3s heartbeats and STOP retries. Fix timing/pacing/airtime accounting if calculated/observed usage exceeds applicable band rules. **No signatures or transmitter activation from an estimated 25ms.**
5. **nRF24:** verify actual 3.3V peak current, CE/CSN/SPI, channel76, 250 kbps, PA_LOW for initial bench, 32-byte CRC16/ACK and ability to move from RX to TX safely under load.
6. **RF-only authenticated:** configure both modems only after #1–5 are signed. Use existing HMAC/session/sequence, not a mocked ACK. Collect 30 real trials per R30H matrix and timestamps from a single common time base, starting with nRF-only then E22-only, then both channels.
7. **Safety boundaries:** radio RX does not prove hardware power cutoff. Before any physical actuator/dummy-load work, complete independent physical STOP/watchdog/current-zero/one-hot release limiter, common DC-rated disconnect, shared-3S wire/fuse sizing, R30A 24 physical OPEN gates and legacy 81 NOT_RUN checks. Run existing R30H event analysis only on measured logs, with human signoff for unmodeled scenarios.

### Record physical evidence here

[Unsigned 22-item R30H-1 board and RF worksheet](BOAT_R30H_1_RF_MODULE_SIGNOFF.csv). Keep each result `NOT_RUN` until actual DMM/scope/photo/log evidence is available and independently reviewed. Data collection must never include root keys, HMAC keys or personal account secrets.

**Next executable work:** perform current baseline host+ESP32-S3 target compilation, correct any integration failures, then require the actual bought hardware/module photos and measurements before enabling RF GPIO. No new custom PCBs, sonar scope or firmware release markers.
