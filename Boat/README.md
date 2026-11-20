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


## Hardware-ready development branch R4
- Backup radio: matching E22-900T22S UART modules on remote and boat; Tracker integrated SX1262 unused for first prototype.
- Boat electrical interface specs and logical pin tables are in `docs/BOAT_ELECTRICAL_SCHEMATIC_DESIGN_R2.md`, `docs/BOAT_HARDWARE_PIN_CONNECTIONS_R1.csv`, `docs/BOAT_E22_UART_PIN_CONNECTIONS_R1.csv`, `docs/BOAT_RADIO_PIN_ASSIGNMENTS_R1.md`.
- Candidate MDD20A module is larger than 77×79mm carrier footprint; placement/compartment must be revised or driver moved outside. See fit review.
- Firmware source has dual radio transport/command validation/dedupe/failsafe scaffold, but pins unset, key provisioning false, and actuators are not integrated.
