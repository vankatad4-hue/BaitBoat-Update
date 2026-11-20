# Boat ver012 — queue extraction and ACK implementation scaffold

- Extracted the fixed four-entry decoded-command FIFO into portable `BoatCommandQueue`, preserving link, duplicate marker and session epoch per entry.
- The BoatDualRadioPort now uses the standalone queue for push/pop, pending-command lookup and clearing on session reset.
- Added host tests for FIFO order, empty/full behavior, no-overwrite on full, wraparound reuse, pending lookup scoped by epoch and clearing.
- Implemented the previously declared but missing `commandPending()` and `sendCommandAck()` definitions. ACK generation uses payload validation, authenticated frame sealing, command type/ID correlation and sends on the receiving link.
- Previous cross-radio replay classification remains: stale sequence with a new command ID is rejected; exact pending/executed duplicate may be ACKed.
- Host tests are committed but not executed in this environment; no compile claim.
- No command executor, STOP/interlock policy, telemetry, actuator output, secure session handshake, GPIO enablement or hardware validation.
