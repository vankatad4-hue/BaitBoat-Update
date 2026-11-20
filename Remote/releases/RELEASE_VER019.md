# Remote ver019 — development snapshot notes

- Authenticated receive dispatcher decodes telemetry/ACK/heartbeat frames after session/envelope validation.
- Per-link and cross-radio sequence guards added; duplicate session markers do not reset telemetry/ACK state.
- Remote ACK tracker connected to action send and inbound ACK; telemetry partial updates with group freshness and primary-link safety provenance.
- Nonzero 16-bit action IDs and heartbeat payload fields corrected.
- Host tests pass for decoder, ACK tracker, telemetry freshness/safety, release guard and scanner syntax.
- Boat-side telemetry/ACK generation, command execution/dedupe, full firmware compile and hardware validation remain pending.
