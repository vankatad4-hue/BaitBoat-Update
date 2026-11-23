# R30H-19 — STOP ACK → authoritative STATUS priority and host integration

**2026-11-23 · VS Code + PlatformIO · non-actuating development only**

## Finding

R30H-18 reliably queues a Boat STOP ACK when the nRF24 transmitter is occupied. But the remote R30H-17 confirmation state requires **two** pieces of evidence:
1. An authenticated, session/command-ID-matched STOP ACCEPTED/EXECUTED ACK.
2. A subsequent authenticated, replay-checked **primary Boat STATUS** reporting `stopState=1`.

Previously the Boat application attempted STATUS telemetry immediately after `radio.update()`; the radio scheduler might already have admitted the next 100 ms heartbeat. `sendFrame()` then returned false, and the 200 ms periodic STATUS attempt could repeatedly miss available TX slots. Boat outputs could be safely stopped yet Remote would remain latched with no STATUS. An earlier status must NEVER be promoted into new STOP evidence.

## R30H-19 changes

- Add `Boat/source/boat_r30h19_status_outbox.h`: **one replaceable snapshot** of the caller's latest `RADIO_PAYLOAD_STATUS`; no synthetic STOP and no direct motor/solenoid access.
- `BoatDualRadioPort::sendHardwareTelemetry()` schedules STATUS instead of attempting immediate best-effort transmission after heartbeat. The existing 200 ms real application sampling remains unchanged. STATUS from an absent/invalid physical-feedback reader stays **unknown**; this change cannot manufacture `stopState=1`.
- In `BoatDualRadioPort::update()` the priority order becomes **nRF TX completion → validated RX / STOP acceptance → STOP ACK → latest STATUS → heartbeat → optional diagnostic → UTC**. Pending ACK blocks STATUS and ordinary primary transmissions; pending STATUS blocks ordinary traffic until its one scheduled opportunity is attempted.
- STATUS snapshots expire after **300 ms**, are invalidated by each new authenticated STOP and on handshake/session reset or primary PHY failure. The authenticated wire format, HMAC, 32-byte frame, sequence guard, and backup radio rules are unchanged.
- `Boat/source/boat_r30h18_stop_ack_outbox.h` retains its 8-transmission / 75 ms / 2500 ms window. Remote retains its separate 8-admission / 150 ms / 2500 ms retry, correlated STOP ACK and primary stopped-STATUS requirements. **RF buffer admission is NOT receipt at Remote**.
- The status offered by the Boat application continues to originate from `boatHardware_status()`: STOP status requires valid recent safety and output feedback, watchdog readiness, stationary motors and idle releases. If physical feedback is absent, the status remains unknown.

## Host integration (simulated, no physical radio)

`Boat/tests/boat_r30h19_stop_ack_status_integration_test.cpp` composes the **actual portable Boat R30H-18 ACK outbox, new STATUS outbox and Remote R30H-17 confirmation gate**. It simulates a TX-busy packet, a failed ACK, a successful RF ACK dropped before the Remote application, command-ID-preserving STOP reattempt, an unknown STOP STATUS, a fresh primary stopped STATUS, new STOP supersession, expired and invalid STATUS, session/transport failure and `millis()` rollover.

The test exercises state-machine *composition*; it does NOT instantiate physical RF24, Waveshare I²C→SPI bridge, actual actuator driver or prove end-to-end radio delivery.

Both `.github/workflows/boat-host-tests.yml` and the Boat job in `.github/workflows/platformio-migration-build.yml` compile and run this integration test. The source/mutation audit checks ACK-before-STATUS-before-heartbeat order, cleanup of pre-STOP status and active priority interlocks. The existing R30G audit was updated to keep optional diagnostics below pending status.

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IBoat/source -IRemote/source \
 Boat/tests/boat_r30h19_stop_ack_status_integration_test.cpp -o /tmp/r30h19 && /tmp/r30h19
python Boat/tools/verify_platformio_source_layout_r30h13.py
python -m unittest discover -s Boat/tests -p test_platformio_source_layout_r30h13.py -v
python Boat/tools/verify_r30g_optional_diag_priority.py
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

**CI evidence note:** check the GitHub Actions result on the latest code commit; earlier runs may be cancelled by concurrency or may have failed before the R30G/source-audit adjustments. Do not count such attempts as current PASS.

## Still blocked

**R30H-10:** 12 NOT_RUN / 6 BLOCKED. **R30H-12:** 7 NOT_RUN / 5 BLOCKED. GPIO, radio/bridge, physical independent STOP/current-zero, output disconnect, CE reset-level, SPI/I²C latency and actual on-air STOP/ACK/STATUS delays have not been accepted. No flashing or actuator power is authorized. Boat **VER014** / Remote **VER028** remain unchanged.

## Next engineering priority

After passing latest host and PlatformIO CI, instrument non-actuating dual-endpoint integration for three observable events: Boat STOP command admission (exact ID), Boat ACK TX and Remote ACK receipt, then independently sourced Boat stopped-STATUS TX and Remote receipt. Record common-clock measurements **only on real assembled bench hardware**; never turn source-only timestamps into physical evidence. If an ACK or STATUS is absent, keep Remote STOP confirmation asserted and movement/release denied.
