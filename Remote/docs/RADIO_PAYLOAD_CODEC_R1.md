# Radio payload codec R2

Portable eight-byte payload validators were added to both endpoint source trees:
- `Remote/source/radio_payload_codec.h/.cpp`
- `Boat/source/radio_payload_codec.h/.cpp`

Functions decode HEARTBEAT, ACK subtype 0, and TELEMETRY subtypes 1–6. They reject unknown subtypes, enum/range errors, and nonzero reserved bits/bytes. Decoder output is cleared on failure.

Host tests: `Remote/tests/radio_payload_codec_test.cpp`. The same test is compiled against the Remote and Boat copies with GCC C++11, `-Wall -Wextra -Werror`.

**Integration update:** Remote inbound frame dispatch now uses the decoders after HMAC/session validation. Telemetry payloads are applied to per-group state with freshness and primary-link provenance. ACK payloads feed the command tracker. Boat-side telemetry/ACK generation remains unimplemented. These decoders do not authorize commands or actuate hardware.
