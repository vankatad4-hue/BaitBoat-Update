# Remote command ACK tracker R2

A portable bounded tracker is added at `Remote/source/radio_ack_tracker.*`.

- Capacity: 8 records. Pending records are not evicted; completed/timeout slots may be reused.
- States: WAITING, ACCEPTED, EXECUTED, DUPLICATE, REJECTED, TIMEOUT.
- ACCEPTED is intermediate and remains pending until EXECUTED/rejection or timeout.
- ACK matching requires the same 16-bit command ID, command type, session epoch and permitted transport.
- Clear-STOP, non-RETURN-HOME mode changes and release commands require primary-link ACK. STOP, RETURN HOME and manual control may be acknowledged on backup.
- Timeout uses wrap-safe unsigned millis subtraction.
- The tracker is now wired to non-manual Remote command queue/send and inbound authenticated ACK dispatch. A UI-facing query API is exposed but the screen does not yet display lifecycle states. The boat-side ACK emitter/execution lifecycle is not implemented. A command is not considered executed by transport send success.

Host tests cover epoch/type mismatches, primary-only restrictions, ACCEPTED→EXECUTED, rejection, timeout, millis rollover and pending-ID reuse rules.
