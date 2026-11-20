# Remote command ACK tracker R1

A portable bounded tracker is added at `Remote/source/radio_ack_tracker.*`.

- Capacity: 8 records. Pending records are not evicted; completed/timeout slots may be reused.
- States: WAITING, ACCEPTED, EXECUTED, DUPLICATE, REJECTED, TIMEOUT.
- ACCEPTED is intermediate and remains pending until EXECUTED/rejection or timeout.
- ACK matching requires the same 16-bit command ID, command type, session epoch and permitted transport.
- Clear-STOP, non-RETURN-HOME mode changes and release commands require primary-link ACK. STOP, RETURN HOME and manual control may be acknowledged on backup.
- Timeout uses wrap-safe unsigned millis subtraction.
- This is a utility only; it is not yet wired to command send/receive or UI. A command is not considered confirmed by transport send success.

Host tests cover epoch/type mismatches, primary-only restrictions, ACCEPTED→EXECUTED, rejection, timeout, millis rollover and pending-ID reuse rules.
