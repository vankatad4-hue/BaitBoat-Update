# Boat host-side tests

These tests use a host C++ compiler and small Arduino/frame stubs. They do **not** validate cryptography or radio hardware.

## Payload codec/encoder

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror Boat/source/radio_payload_encoder.cpp Boat/source/radio_payload_codec.cpp Boat/tests/radio_payload_encoder_test.cpp -o /tmp/boat_payload_test
/tmp/boat_payload_test
```

## Command deduplication and failsafe decision logic

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/boat_radio_safety.cpp Boat/tests/boat_radio_safety_test.cpp -o /tmp/boat_safety_test
/tmp/boat_safety_test
```

## Command codec and backup-link policy

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/radio_command_codec.cpp Boat/tests/radio_command_codec_test.cpp -o /tmp/boat_codec_test
/tmp/boat_codec_test
```

## Sequence guard

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/radio_command_codec.cpp Boat/tests/radio_sequence_guard_test.cpp -o /tmp/boat_sequence_test
/tmp/boat_sequence_test
```

Command-codec and sequence-guard tests supply stub frame-authentication/freshness functions. They validate policy and sequence logic only, not HMAC/CRC or physical links. Tests cover backup restrictions, throttle clamp, command-ID dedupe scope/cache behavior, cross-radio replay classification, wraparound and stale/out-of-order sequence rejection, and link-failsafe decision logic. They do not test the BoatDualRadioPort session/queue integration or actuators.

## Bounded command queue

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/boat_command_queue.cpp Boat/tests/boat_command_queue_test.cpp -o /tmp/boat_queue_test
/tmp/boat_queue_test
```
