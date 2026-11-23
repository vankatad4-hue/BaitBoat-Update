# R30H-18 — Boat STOP ACK under occupied nRF24 TX

**2026-11-23 | VS Code + PlatformIO primary | source-only development | Boat VER014 / Remote VER028 unchanged**

## Defect and safety consequence

Previously `BoatDualRadioPort::receive()` issued a STOP `RADIO_ACK_ACCEPTED` immediately. `sendCommandAck()` called `sendFrame()` once, and if nRF24's asynchronous transmitter was busy, the ACK was dropped. Boat's logical STOP intent was still asserted, but Remote R30H-17 could not satisfy its **matching STOP command-ID ACK, then fresh primary stopped-STATUS** gate. No hardware STOP/current-zero evidence is implied by that ACK.

## Change

- `Boat/source/boat_r30h18_stop_ack_outbox.h` is a portable single-slot **STOP ACK only** response queue, not a general command FIFO. It retains the 16-bit command ID, session epoch and ACCEPTED/EXECUTED result; a new STOP supersedes the old item. A repeated STOP with the same ID does not renew the deadline.
- Primary `sendCommandAck()` enqueues a STOP response rather than silently dropping it on TX-busy. The E22 backup path, non-STOP ACK behavior, authenticated frame format, session keys and replay guards are unchanged.
- In `BoatDualRadioPort::update()`: poll nRF TX completion, consume authenticated RX, **then attempt the pending STOP ACK before primary heartbeat, optional diagnostics or UTC**. All other primary TX is barred while a STOP ACK is pending. Sending starts with a fresh authenticated sequence number; the logical command ID remains stable.
- If RF24 reports an unsuccessful TX, the outbox can try again after **75 ms**, up to **8 radio TX admissions** and **2,500 ms of total local queue age**, whichever is reached first. A hardware-success indication retires the Boat queue, **not** the Remote confirmation requirement. The existing RF24 120 ms TX-pending watchdog revokes the radio on a stuck TX; session changes or radio faults discard the response.
- The optional diagnostic scheduler is denied while a STOP ACK is pending. R30G safety telemetry remains optional, replaceable and lower priority.
- Remote R30H-17 is unchanged: authenticated **matching command-ID ACCEPTED/EXECUTED ACK** must be followed by a separately decoded, fresh **primary** Boat `stopState=1` status. Local STOP stays latched on ACK loss, stale status or queue timeout. STOP ACK **alone is never evidence of physical current zero**.

## Software tests added

- `Boat/tests/boat_r30h18_stop_ack_outbox_test.cpp`: occupied TX, admission and incomplete TX, retry gap, TX success/failure, one-slot supersession, duplicate TTL, session/driver fault flush, 8-attempt bound, `millis()` wrap, invalid ID/epoch, result upgrade and expiration.
- Boat host and primary PlatformIO workflows run the portable test without any radio or GPIO output.
- Repository source audit checks STOP priority, mandatory primary TX gate and **poll → RX → pending STOP ACK → heartbeat → optional** ordering. Mutation tests reject removal of priority and heartbeat-before-ACK reordering.
- Test commands from repository root:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IBoat/source Boat/tests/boat_r30h18_stop_ack_outbox_test.cpp -o /tmp/boat_r30h18_stop_ack
/tmp/boat_r30h18_stop_ack
python Boat/tools/verify_r30g_optional_diag_priority.py
python Boat/tools/verify_platformio_source_layout_r30h13.py
python -m unittest discover -s Boat/tests -p test_platformio_source_layout_r30h13.py -v
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

**Evidence class:** portable simulated queue test passed locally; actual repo CI verdicts must be checked on the **last** code/document commit, not earlier or superseded runs. PlatformIO is compile-only and no device upload was made.

## Remaining safety and hardware boundaries

R30H-10 remains **12 NOT_RUN / 6 BLOCKED**, and R30H-12 remains **7 NOT_RUN / 5 BLOCKED**. There is no measured nRF air delivery, STOP-to-ACK timing, driver-current-zero, independent power cutoff, CE reset-safe idle, physical bus fault behavior or confirmation that the following stopped-STATUS actually arrives in RF conditions.

Boat's existing application reports stopped-STATUS only from its own supervised output-feedback status stream; **this patch does not invent a synthetic stopped-STATUS** to satisfy Remote. Missing physical feedback therefore leaves Remote safely latched, not released. The periodic STATUS may still be late/lost and needs physical common-clock capture with STOP button, CE/IRQ, command accept, ACK reception and measured output current. Follow R30H-10/R30H-12 acceptance procedures, test with electrically isolated/non-actuating loads and keep all radio/pin/flash/actuator interlocks unsigned until physical acceptance.

**Next:** verify latest Boat host + Boat/Remote PlatformIO CI to completion, then conduct non-actuating dual-endpoint busy-TX RF tests when hardware is available. Physical STOP and current-zero remain separate blockers. The sonar module decision remains deferred.
