# Boat Firmware

Boat-side firmware, configuration, hardware documentation and release archives.

- Source: `source/`
- Hardware and project documentation: `docs/`
- Versioned releases: `releases/`

The repository is the canonical source of truth. Check it before editing and update relevant docs alongside firmware changes. Do not overwrite existing release archives.

## Radio architecture
- Primary: nRF24L01+ PA/LNA, 2.4GHz responsive control/telemetry.
- Backup: matching LoRa UART module (SX1262/E22 family candidate), EU868, with its own 50Ω antenna.
- Boat requires both radios for the dual-link remote system. LoRa carries safety/low-rate packets only; no sonar stream.
- Boat must independently enforce loss-of-link failsafe; return-home only if navigation and system health are valid, otherwise stop/hold safely.
- See `../Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md` for shared link protocol and regulatory design gates.
