# Boat ACK/session integration scaffold R1

**Status: source scaffold only. No ESP32 compile or radio/hardware validation.**

## Session behavior
- Boat now waits for the remote's authenticated SESSION_START; the remote's session epoch is the epoch used for boat replies.
- Boat replies with SESSION_START on configured links. Both radio paths may carry the same session marker.
- Repeated marker on the same link is ignored; a first marker on the second link seeds that link's receive guard without restarting the session.
- A new authenticated epoch clears the queued commands and receive guards. This is not strong anti-replay protection: the current protocol lacks a challenge/response handshake and persistent monotonic session counter.

## Sequence/failover behavior
- Primary and backup links have independent sequence guards.
- A cross-radio guard suppresses duplicate non-command frames from being applied twice.
- CONTROL/ACTION frames are allowed through the cross-link guard because command-ID deduplication, not radio sequence alone, determines duplicate command handling.
- Pending queue entries are keyed by session epoch and 16-bit command ID. A duplicate pending command is not enqueued a second time. Executed duplicates use the existing action cache.

## ACK behavior
- Non-manual command successfully queued → ACCEPTED.
- Already-executed command ID → DUPLICATE.
- Queue full → BUSY.
- ACK is returned on the radio link that carried the command, with matching command type and 16-bit ID.
- Manual motor updates do not receive application ACKs to avoid saturating the responsive control link.
- ACCEPTED means only **received and placed in the software queue**. It is not execution confirmation.
- The boat application does not consume the queue, perform command safety checks, or send EXECUTED ACKs yet. STOP/interlock fields are deliberately unknown/zero.

## Safety limitations
- Boat key provisioning still returns false; authenticated communication is disabled by default.
- GPIO assignments remain unset.
- The queue has no connected command executor.
- ACK transmission may fail, especially on the LoRa duty-limited backup.
- No motor, solenoid, mission, telemetry-source, GNSS, watchdog, or failsafe actuator integration.
- No ESP32 compile, unit test of the port, or hardware test has been performed.

## Next gate
1. Host-test queue and duplicate handling using fake links, including same command arriving on both radios and queue-full behavior.
2. Add a boat application state model and telemetry source contract; keep outputs disabled.
3. Integrate the queue into a fail-closed application loop with STOP and link-watchdog priority.
4. Implement ACCEPTED→EXECUTED/REJECTED lifecycle only after command policy/interlocks are independently tested.
5. Design secure session challenge/response and replay protection before key provisioning.
