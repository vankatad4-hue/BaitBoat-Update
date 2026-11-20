# Boat host-side tests

Portable payload-encoder tests require only a host C++ compiler:

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror \
  Boat/source/radio_payload_encoder.cpp \
  Boat/source/radio_payload_codec.cpp \
  Boat/tests/radio_payload_encoder_test.cpp -o /tmp/boat_payload_test
/tmp/boat_payload_test
```

Optional sanitizer run:

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined \
  Boat/source/radio_payload_encoder.cpp \
  Boat/source/radio_payload_codec.cpp \
  Boat/tests/radio_payload_encoder_test.cpp -o /tmp/boat_payload_test_san
/tmp/boat_payload_test_san
```

This test validates wire-payload serialization and strict validation only. It does not test frame authentication, radio transport, command execution, actuator outputs, or hardware.
