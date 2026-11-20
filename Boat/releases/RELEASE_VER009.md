# Boat ver009 — development snapshot notes

- Added portable 8-byte HEARTBEAT, ACK and TELEMETRY payload encoders matching the existing decoder contract.
- Added round-trip and invalid-range host tests for status, battery/speed, GNSS coordinates, navigation, sonar summary, heartbeat and ACK.
- The encoder is not yet connected to the boat radio receive queue, ACK lifecycle, application telemetry source, or command execution.
- No actuator, GNSS, sonar, radio GPIO or hardware readiness changes. No full ESP32 compile or hardware test.
