# Physical secure provisioning — deliberately disabled by default

The development source implements a primary-link mutual nonce handshake with domain-separated HMAC-SHA256 proofs and derived 256-bit session key. Both sides authenticate handshake envelopes with a unique shared root key. Legacy session-start frames cannot install a key. A single derived session supports both radios; fresh session establishment requires nRF24, not LoRa. Sequence counters are bounded below 30000, never wrap, and only reset after a new handshake. The peers invalidate queue/mission/telemetry state on session loss/rekey. The existing 64-bit frame authentication tag is unchanged; the session is authenticated, not encrypted in transit.

## Physical acceptance, before any opt-in
- Verify exact ESP32-S3 module variants and protected production bootloader/partition configuration; use vendor-approved secure boot and flash/NVS encryption provisioning guidance for that configuration. Enabling eFuses can be irreversible. Do not burn eFuses as part of this development build.
- Verify true random entropy during pairing/rekey with the actual RF/ADC configuration. The ESP hardware RNG API alone does not establish this when RF is off. If necessary commission a validated entropy source and DRBG.
- Generate a unique 32-byte root key on an offline trusted machine and provision the same key into **encrypted NVS** on the two owned endpoints (namespace flytec_radio, binary blob pair_key, exactly 32 bytes). Never use a default key, source code, CI secret, logs, chat, or SD map storage for the root key.
- The guarded reader requires build opt-in FLYTEC_ENABLE_SECURE_PROVISIONED_RADIO=1, CONFIG_NVS_ENCRYPTION, secure boot and flash encryption runtime checks, and successful read-only nonzero key retrieval. No write/provision command or fallback to plaintext exists. Normal CI compiles the disabled path; the production opt-in branch must be built and verified with the exact secure SDK configuration.
- Pair with high-current actuator supply disconnected. Verify wrong-key refusal, lost confirmation recovery, new-session replay rejection, session exhaustion, reboot, link loss, STOP priority, and real RF range. Rotate/revoke by securely replacing both device keys and verifying old keys cannot authenticate; no over-air pairing or key transfer exists.
- No actuator or autonomous start until Boat independent safety/watchdog, stationary feedback, current limits, and hardware checks pass.

## Current scope
The new target Boat sketch polls the radio and discards commands without physical execution. A queue-level ACK is not EXECUTED. Secure physical provisioning is a later hardware operation requiring explicit approval and verification. Do not use the diagnostic firmware in water.
