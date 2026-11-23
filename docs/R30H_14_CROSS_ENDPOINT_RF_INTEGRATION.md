# R30H-14 — Boat/Remote nRF24 interoperability profile and bounded RX polling

**Date:** 2026-11-23. **Scope:** non-actuating source integration and portable RF mocks only. **Primary IDE: VS Code + PlatformIO**. Official Boat **VER014** / Remote **VER028** remain unchanged. This document adds to [R30H-13 PlatformIO source migration](R30H_13_PLATFORMIO_SOURCE_MIGRATION.md).

## Evidence and correction

At the beginning of R30H-14, both Boat and Remote primary PlatformIO builds and both Arduino reference builds were independently **PASS** for R30H-13 source, [PlatformIO #45129353902](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45129353902). Both host suites and the source inventory also passed. That was a compiler/source baseline, **not** RF, GPIO, STOP or current-zero proof.

During the subsequent source review, `Boat/source/nrf24_link.cpp` had a polling duplication: `BoatDualRadioPort::update()` called `nrf_.update()`, then every `nrf_.receiveFrame()` call invoked `update()` again. The fix makes `NRF24Link::update()` the **only hardware poll**. It reads **at most three** packets per call, matching nRF24's three-level RX FIFO. `receiveFrame()` only pops the in-RAM four-slot queue and **never issues extra SPI**. Frames with invalid magic or application CRC are discarded. RX remains deferred while TX is awaiting completion; no stale frame may be taken when the radio has faulted. R30H-13's **120 ms software TX-pending timeout and fail-closed primary revocation** remain in force.

Portable `Boat/tests/r30h13_boat_nrf_timeout_test.cpp` now checks boot lock, valid ACK, MAX_RT, 120 ms stuck TX, millis rollover, **RX FIFO burst including a bad CRC**, no I/O during queue dequeue and deferral while TX is pending. Its enhanced mocked RF24 also records live configuration arguments, read/write pipe addresses, auto-ACK, data rate, retries, CRC and frame length.

## Cross-endpoint PHY *configuration* contract (software simulation only)

| Profile item | Boat RF24 library source | Remote SC18IS606 register source |
|---|---|---|
| nRF channel | `BOAT_NRF_CHANNEL=76` | `REMOTE_NRF_CHANNEL=76` |
| Data rate | `RF24_250KBPS` | `RF_SETUP=0x22` |
| Initial PA | `RF24_PA_LOW` | `RF_SETUP=0x22` |
| RF auto-ACK | enabled on active pipes | `EN_AA=0x03` (P0/P1) |
| Auto retries | delay=5, count=15 | `SETUP_RETR=0x5F` |
| CRC length | `RF24_CRC_16` | `CONFIG=0x0F` in RX |
| Dynamic payload | disabled | `DYNPD=0`, `FEATURE=0` |
| Payload length | 32 fixed bytes | `RX_PW_P0/P1=32` |
| TX address | `RMTE1` (Remote's receive address) | `BOAT1` (Boat's receive address) |
| RX pipe 1 address | `BOAT1` | `RMTE1` |
| Pipe 0 ACK address | managed by RF24 `openWritingPipe()` | same as Remote TX, `BOAT1` |

The mirrored over-air protocol retains 32-byte frames, application CRC and authenticated session/command verification. **No claim of RF interoperability:** tests inspect source configuration and emulated register writes, not a logic-analyzer capture or paired radio modules. They do not prove antenna polarity, board SPI routing, CE idle levels, SNR, RF power compliance or link STOP latency.

`Remote/tests/sc18is606_bridge_nrf_test.cpp` now asserts all three 5-byte address registers, retransmission settings, P0/P1 ACK and payload configuration, receive-mode CRC, and channel/data rate. `Boat/tools/verify_platformio_source_layout_r30h13.py` explicitly forbids drift in channel, data-rate/CRC/retry code and fixed-payload settings. `Boat/tests/test_platformio_source_layout_r30h13.py` injects mismatched channel and retry settings to verify the audit fails closed.

## Source-only repeatable commands

```sh
python3 Boat/tools/verify_platformio_source_layout_r30h13.py
python3 -m unittest discover -s Boat/tests -p test_platformio_source_layout_r30h13.py -v
python3 Boat/tools/verify_r30h10_physical_lock.py
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IBoat/tests/r30h13_nrf_stubs -IBoat/source Boat/source/nrf24_link.cpp Boat/tests/r30h13_boat_nrf_timeout_test.cpp -o /tmp/boat_nrf_timeout
/tmp/boat_nrf_timeout
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/tests/bridge_stubs -IRemote/source Remote/source/sc18is606_port.cpp Remote/source/mcp23017_ce_port.cpp Remote/source/sc18is606_nrf_link.cpp Remote/tests/sc18is606_bridge_nrf_test.cpp -o /tmp/remote_bridge_nrf
/tmp/remote_bridge_nrf
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

These commands **compile and simulate only**; do not use `pio upload`. Both projects intentionally deny upload. Host regression results for this code include [Boat portable tests #45131342414](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45131342414) **PASS**, [Remote portable tests #45131342373](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45131342373) **PASS**, and [source inventory #45131342360](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45131342360) **PASS**. The [latest R30H-14 primary PlatformIO build #45131671825](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45131671825) is **PASS for Boat and Remote**, after the code changes. This is compile-only evidence, not a verified live RF or STOP test.

## Verified target CI (2026-11-23)

**Latest R30H-14 target compilation (2026-11-23):** [Primary PlatformIO #45131671825](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45131671825) **Boat PASS / Remote PASS** (compile-only, no hardware flash). [Boat Arduino reference #45131671920](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45131671920) **PASS**. Remote Arduino reference [#45131671831](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45131671831) **still processing at checkpoint write**, so its result is not claimed. Boat and Remote source/host tests PASS. The physical CE reset, shared-I2C latency, real RF packets and independent STOP/current-zero remain **NOT_RUN/BLOCKED**. Official Boat VER014 / Remote VER028 are unchanged.

## Next R30H-10 physical handoff

With real modules available, start with visually verified **Waveshare Standard** board, **SC18IS606-EVB** and **MCP23017 0x21** at 3.3 V. Keep actuator power disconnected. Use one logic-analyzer clock to validate the two separate SPI domains (Remote TF EXIO4 vs bridge SS0), MCP CE active-high transitions, nRF STATUS/ACK and 32-byte frames, I²C BUSY NACK/latency, and recovery under SD/ADC/UI concurrency. In particular **MCP GPA0 CE may remain HIGH across MCU reset or I²C loss**, so an independent reset-safe hardware interlock is still a **BLOCKER**; no software timeout solves this. Measure STOP press to authenticated reception and separately physical current-zero/solenoid output disable using dummy loads only after safe module-based interlocks are engineered.

The original 18 R30H-10 gates remain **12 NOT_RUN / 6 BLOCKED** and the 12 R30H-12 measurements remain **7 NOT_RUN / 5 BLOCKED**, no physical approval. GNSS/PPS remains a separate physical gate. Do not unlock radio audit flags, actuator authorization, flashing or release markers as a consequence of this software work.
