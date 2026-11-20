# Release guard test
Run: `g++ -std=c++17 -Wall -Wextra -Werror -IRemote/tests/stubs -IRemote/source Remote/source/release_button_guard.cpp Remote/tests/release_guard_unit_test.cpp -o release_guard_test && ./release_guard_test`. Tests timing, confirmation, timeout and backup link rejection. Scanner syntax can be checked with `g++ -std=c++17 -Wall -Wextra -Werror -IRemote/tests/stubs -IRemote/source -fsyntax-only Remote/source/physical_button_scanner.cpp Remote/source/release_button_guard.cpp`. Hardware GPIO/board/interlocks are not covered.


## Payload codec tests
Run from the repository root: `g++ -std=c++11 -Wall -Wextra -Werror -IRemote/source Remote/source/radio_payload_codec.cpp Remote/tests/radio_payload_codec_test.cpp -o payload_codec_test && ./payload_codec_test`. Portable, no Arduino dependencies.

## ACK tracker tests
Run: `g++ -std=c++11 -Wall -Wextra -Werror -IRemote/source Remote/source/radio_payload_codec.cpp Remote/source/radio_ack_tracker.cpp Remote/tests/radio_ack_tracker_test.cpp -o ack_tracker_test && ./ack_tracker_test`. Covers session/type matching, primary-only restrictions, ACCEPTED→EXECUTED, rejection, timeout and millis rollover. Tracker is wired into the Remote send/receive path; Boat-side ACK generation and UI display remain pending.


## Telemetry freshness tests
Run: `g++ -std=c++11 -Wall -Wextra -Werror -IRemote/tests/stubs -IRemote/source Remote/source/boat_telemetry.cpp Remote/source/radio_payload_codec.cpp Remote/tests/boat_telemetry_test.cpp -o telemetry_test && ./telemetry_test`. Tests primary provenance, STOP/interlock readiness, coordinate sample pairing, GPS freshness, heading validity, battery and sonar summary. It does not test the hardware radio path.
