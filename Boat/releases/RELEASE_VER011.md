# Boat ver011 — cross-radio replay-policy and host-test snapshot

- Added cross-radio command sequence classification: new commands are accepted only on forward sequence; a sequence-rejected command may be acknowledged only when its exact command ID is already pending or executed; stale sequence/new ID is dropped.
- Extracted the cross-radio replay decision into a portable helper and added host assertions for all decision branches.
- Added host tests for executed-command cache scope/eviction, failsafe decision truth table, sequence wrap/replay rejection, and backup-link command restrictions/throttle clamping.
- Added host Arduino/frame stubs and expanded test instructions.
- **Tests are committed but not executed in this environment** (GitHub clone/network unavailable); compile status remains unverified.
- This is protocol-policy test scaffolding only. No fake-radio integration test of the BoatDualRadioPort, cryptographic validation, ESP32 compile, actuator application, telemetry, secure session, or hardware tests.
- Key provisioning remains fail-closed and all radio GPIO macros remain disabled/TBD.
