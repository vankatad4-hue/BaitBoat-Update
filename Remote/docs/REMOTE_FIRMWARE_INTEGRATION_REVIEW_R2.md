# Remote firmware integration review R2 — 2026-11-20

## Implemented on the Remote source
- Strict payload validators for HEARTBEAT, ACK subtype 0, and TELEMETRY subtypes 1–6.
- Authenticated receive dispatcher in `boat_radio.cpp`: envelope/HMAC, IDs, flags, session epoch and reserved payload fields validated before dispatch.
- ACK lifecycle tracker wired into non-manual command sends and inbound ACK frames; matching 16-bit ID/type/session, link restriction, ACCEPTED→EXECUTED, rejection and timeout. Query API is exposed for UI integration.
- Partial telemetry application wired to inbound telemetry; local receive timestamps, coordinate sample pairing, per-group expiry, primary-link provenance and release readiness gates.
- Heartbeat payload sender fields corrected (availability bits/build-major; STOP bit only from confirmed state).
- Command action IDs use nonzero 16-bit sequence, separate from transport sequence.
- Per-radio sequence guards plus a cross-radio guard; authenticated cross-radio duplicates are not applied twice.
- Main loop applies telemetry with local millis and continues to fail closed when status/primary/interlock freshness is missing.

## Host validation performed
GCC C++11/C++17 warnings-as-errors + AddressSanitizer/UndefinedBehaviorSanitizer passed for:
- payload codec tests;
- ACK tracker tests;
- partial telemetry/freshness/safety tests;
- release guard tests;
- physical button scanner syntax check with hardware stubs.
OpenSCAD R6.6 front/rear/button meshes from a reconstructed local source mirror rendered; front/rear watertight and the button plate has 16 separate caps. Internal-preview export timed out. This is not byte-for-byte repository source verification.

## Remaining blockers
1. Boat firmware still does not emit telemetry or ACK frames and does not implement the command execution/deduplication lifecycle.
2. The UI does not yet show per-command ACK state, failsafe/low-battery/heading-valid warnings or detailed telemetry group freshness.
3. STOP clear, mission and release execution are not implemented boat-side.
4. Session recovery after a boat reboot within an existing remote session is intentionally fail-closed; recovery protocol needs design/test.
5. Full ESP32 firmware compile, actual Waveshare pin crosswalk, radio/antenna/bridge bring-up, key provisioning, I2C/SPI concurrency, and hardware safety tests remain unverified.

**No actuator command is operationally safe yet.** Boat-side firmware and hardware are not complete.
