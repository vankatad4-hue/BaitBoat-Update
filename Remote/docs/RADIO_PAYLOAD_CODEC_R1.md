# Radio payload codec R1

Portable eight-byte payload validators were added to both endpoint source trees:
- `Remote/source/radio_payload_codec.h/.cpp`
- `Boat/source/radio_payload_codec.h/.cpp`

Functions decode HEARTBEAT, ACK subtype 0, and TELEMETRY subtypes 1–6. They reject unknown subtypes, enum/range errors, and nonzero reserved bits/bytes. Decoder output is cleared on failure.

Host tests: `Remote/tests/radio_payload_codec_test.cpp`. The same test is compiled against the Remote and Boat copies with GCC C++11, `-Wall -Wextra -Werror`.

**Not yet integrated:** inbound frame dispatch, command ACK correlation/lifecycle, telemetry group freshness/coordinate pairing, boat telemetry transmission and UI state updates. These files are parsing primitives only; they do not authorize any command or actuate hardware.
