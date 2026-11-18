# Flytec V080 — nRF24L01+ PA/LNA radio link decision

**Decision recorded: 2026-11-16**  
**Status: selected architecture; hardware driver and pinout not yet implemented or verified.**

## Selected hardware

Use one nRF24L01+ PA/LNA transceiver module in the handheld remote and one matching nRF24L01+ PA/LNA module in the boat. Both ends must use compatible nRF24L01+ silicon/protocol settings. This replaces the earlier SX1278 433 MHz / SX1262 868 MHz LoRa proposals for the primary remote-to-boat command and telemetry link.

- Band: 2.4 GHz ISM.
- Initial data-rate setting for range testing: **250 kbps**. The nRF24L01+ also supports 1 Mbps and 2 Mbps; only increase after measured link testing.
- Payload: nRF24L01+ Enhanced ShockBurst supports up to 32 bytes per packet.
- Use hardware auto-acknowledgement and bounded retransmission for appropriate unicast packets. Define separate message semantics for commands, acknowledgements and telemetry.
- Target range remains 500 m in mixed conditions; it is a test target, not a guaranteed range.
- Keep sonar echogram data local to the boat display. The radio carries control and compact telemetry, not the full CHIRP waterfall.

## Electrical and RF integration requirements

1. Power the PA/LNA module from a dedicated, low-noise regulated 3.3 V rail sized for the module's measured transmit-current peaks. Do not assume the ESP32 board's 3.3 V output can safely supply it. Add local bulk and high-frequency decoupling close to the module; follow the exact module vendor's requirements.
2. Logic is 3.3 V. Do not apply 5 V to nRF24 supply or logic pins.
3. Connect by SPI (SCK, MOSI, MISO) plus CE and CSN; IRQ is optional but may be used. Exact GPIO assignments remain intentionally unassigned until the final board pin map and all display/SD/joystick/peripheral conflicts are checked.
4. Use a suitable 2.4 GHz antenna with a proper 50-ohm RF connection. Keep it away from display cables, switching regulators, motor wiring and metalwork. Mount the boat antenna as high and clear of the water as practical; keep the remote antenna clear of the operator's hand and enclosure shielding.
5. PA/LNA boards vary. Verify the actual module schematic, RF front-end, supply current, antenna connector and output power before PCB design.
6. Configure channel, data rate, address, CRC, auto-ack and retry parameters identically at both ends. Select the channel after checking local 2.4 GHz interference.
7. Verify the final radiated EIRP and applicable Bulgarian/EU 2.4 GHz requirements. PA/LNA modules can exceed permitted power depending on board gain and antenna gain. Do not assume maximum module output is automatically legal.

## Protocol and safety requirements before operation

- Define a shared, versioned packet format no larger than 32 bytes for each nRF24 packet.
- Include message type, protocol version, sequence number and integrity validation. The nRF24 hardware CRC detects transmission errors but is **not authentication or encryption**.
- Design application-level authentication/replay protection before actuator commands are enabled. Do not describe CRC as security.
- Use bounded retries and explicit application acknowledgements for safety-critical commands (especially STOP, mode changes and release commands). Hardware auto-ack alone does not prove the application acted on a command.
- Include a heartbeat and boat-side stale-command watchdog. The boat must enforce STOP and actuator interlocks independently of the remote.
- On link loss in autopilot, the agreed preference is return-home only when GNSS, heading, home point and boat health are valid; otherwise enter the separately defined safe state. On manual link loss, propulsion must be stopped safely.
- Display connection state and link-quality / packet-loss statistics. Track sequence gaps, retries, round-trip latency and last valid packet age.
- Do not clear the remote's STOP-latched state until a valid boat status/STOP acknowledgement is received.

## Initial performance test profile

| Parameter | Initial test value |
|---|---|
| Air data rate | 250 kbps |
| Application payload | Up to 32 bytes per packet |
| Joystick update target | 10–20 Hz, only after measured packet loss and latency permit it |
| Test distances | 100 m, 250 m and 500 m |
| Test conditions | Bench, open line-of-sight, boat antenna close to water, boat moving, representative 2.4 GHz interference |
| Record | Packet delivery, retries, RSSI where available, round-trip latency, jitter, longest interruption, current draw and recovery after link loss |

The 10–20 Hz target is a design/test objective, not a guaranteed rate. Account for acknowledgement airtime, retries, packet collisions, interference and regulatory compliance. Do not rely on theoretical air data rate as application throughput.

## Current implementation boundary

The remote `boat_radio.*` module remains a non-operational stub. The boat project remains a non-actuating scaffold. This decision updates the architecture and documentation only; it does not mean either radio is wired, configured, paired, tested or operational. Pin assignments, exact PA/LNA module model, antenna, RF channel, RF24 library version, packet format, authentication, watchdog and failsafe implementation remain open engineering tasks.
