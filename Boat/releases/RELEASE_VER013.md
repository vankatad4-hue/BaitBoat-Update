# Boat ver013 — host-test build cleanup

- Removed duplicate heartbeat and ACK payload encoder definitions from `radio_payload_encoder.cpp`; the shared implementations remain in `radio_payload_codec.cpp`. This resolves duplicate-symbol risk in the documented host-test build command.
- Added a GitHub Actions workflow that compiles and runs the portable payload, safety/dedupe/cross-radio, command codec, sequence-guard and bounded command-queue host tests with `-Wall -Wextra -Wpedantic -Werror`.
- Tests have not yet been observed passing in Actions; compile status remains unconfirmed until the workflow run is reviewed.
- No BoatDualRadioPort fake-radio integration tests, secure challenge/response, application command executor, telemetry sender, output interlocks or hardware testing.
