# Boat ver010 — development snapshot notes

- Boat dual-radio port now echoes the remote's session epoch and responds to session-start markers.
- Added per-radio sequence guards plus cross-radio duplicate filtering; CONTROL/ACTION frames use command-ID deduplication for failover.
- Pending commands are deduplicated across both links.
- Non-manual queued commands receive ACCEPTED, executed duplicates receive DUPLICATE, and full queue receives BUSY; ACKs are sent on the incoming link.
- ACCEPTED is not execution confirmation. Boat application/executor, EXECUTED ACK, telemetry source, interlocks and failsafe execution remain unimplemented.
- Source not compiled for ESP32; no hardware or actuator tests. Key provisioning remains fail-closed.
