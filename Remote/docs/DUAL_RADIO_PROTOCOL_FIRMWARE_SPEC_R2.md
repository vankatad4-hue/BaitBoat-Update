**SUPERSEDED:** Current radio pairing, timing and implementation status are in `DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R3.md`. R2's raw Tracker SX1262 pairing assumptions and status descriptions are historical.

# Dual-radio protocol and implementation specification R2

## System radio pairing
- Remote nRF24L01+ PA/LNA is primary; boat has matching nRF24 module.
- Remote LoRa backup uses EBYTE E22-900T22S UART/SX1262 module.
- Boat backup uses Heltec Wireless Tracker candidate integrated SX1262, if exact board/revision and EU band are confirmed. Do not add a second boat LoRa module by default.
- E22 transparent UART and boat raw SX1262 packets are not inherently compatible; set identical LoRa PHY and encapsulation parameters, then test interoperability.

## Wire frame
Fixed 32-byte frame, common for nRF payload and LoRa application packet. Protocol implementation in Remote/source/radio_protocol.*.
- magic 2B; version; message type; source/destination IDs; sequence16; commandId16; timestamp16ms; flags; payload8; CRC16-CCITT; truncated HMAC-SHA256 tag8; reserved byte.
- CRC checks accidental corruption. HMAC authentication prevents unauthenticated commands only when a unique secret key is provisioned and verified.
- Key is not embedded as a usable default. Firmware must remain command-disabled until provisioning/secure storage is implemented. HMAC tag is 64 bits; security review should decide tag length and key provisioning before release.
- 16-bit sequence/command ID wrap; receiver duplicate cache and freshness limits are designed for short windows only. Long-running session/reconnect needs session nonce/epoch before unattended operation.

## Payload encoding
ACTION/CONTROL payload:
- byte0 command type
- byte1 mode
- bytes2–3 left motor signed int16 LE
- bytes4–5 right motor signed int16 LE
- byte6 activate/action flags
- byte7 reserved, zero
Motor command normalized -1000..1000.
Frame commandId is low 16 bits of command's logical ID. Reconnect/session epoch must be added before final safety release.
Telemetry and ACK payload formats still need detailed allocation; current 8-byte payload is insufficient for all telemetry. Split messages or compact values; do not send unvalidated structures.

## LinkManager scaffold (Remote/source)
- Implemented state/health API and routing policy in dual_radio_manager.*.
- Primary stale after 250ms (provisional), backup stale after 3s (provisional).
- Failback requires eight authenticated primary packets and ≥1.5s holdoff (provisional).
- Backup manual motor commands limited to 25% and at most 1Hz; STOP/RETURN allowed. Mission start, clear-stop, bait/hook release and other action commands blocked on backup.
- No normal commands during degraded/recovery state.
- These thresholds must be measured, and backup duty cycle enforced by actual RF transport.

## Transport status
- boat_radio.* now integrates manager status/hooks and routes commands through it.
- Actual nRF driver, E22/SC16IS750 UART bridge driver, Heltec SX1262 driver, wire serialization integration, receive validation, ACKs and packet scheduling remain not implemented.
- CE through MCP23017 is prototype-only. Direct GPIO preferred.
- No authenticated packet can currently be reported by a driver; therefore both links remain offline and commands are rejected.

## Safety requirements for boat firmware
1. Authenticated packet + correct IDs + version + CRC + age + sequence + command policy required.
2. Duplicate action cache (at least 32 command IDs per sender) prevents repeat bait/hook release across retries/radio handover.
3. Action ACK is returned only after boat-side state confirms action. Release retries never re-act.
4. STOP always accepted if packet authentic, regardless active link; backup RETURN is accepted only if navigation/system health validates.
5. If both links expire, boat local watchdog stops motors unless an autonomous mission is active and GNSS/heading/battery/geofence state is valid, in which case return-home may continue.
6. Solenoids default OFF on reset, no link, invalid packet, or watchdog.
7. On reconnect, synchronize state; never replay queued stale steering.
8. Provision per-device/project HMAC key, prevent debug dumps, define replacement/recovery process.

## EU airtime profile
Candidate band for backup safety/control: 869.4–869.65MHz, harmonized conditions indicate up to 500mW ERP with 10% duty-cycle or polite spectrum access, but national implementation, device category, bandwidth and EIRP/ERP calculation must be confirmed in Bulgaria. E22 output max is 22dBm, not a target setting.
- Initial power must be conservative and compliant with antenna gain/feed loss.
- Configure the exact selected channel and LoRa PHY on both devices. Keep packet airtime short.
- Use channel18 868.125MHz (1% duty band). At 25ms conservative frame airtime, one frame per 3s per transmitter = 0.83% duty; enforce 36s/hour max and track each endpoint separately. This is too slow for normal joystick steering; LoRa backup is STOP/RETURN and limited control only if tested, compliant airtime budget permits.
- Duty-cycle counters persist across resets or reset only after the legal interval; design a robust airtime limiter.
- Avoid 868.0–868.6MHz 1% subband for 2Hz control unless LBT/AFA compliant access is implemented and verified.

## Test plan
- codec golden vectors: CRC, HMAC, invalid tag, malformed, wrong IDs, stale timestamp, sequence rollover.
- cross-link duplicates and action cache.
- nRF packet delivery/retries/latency.
- E22 ↔ native Heltec SX1262 interoperability at identical PHY settings; UART transparency, AUX state, packet framing, ACK.
- primary loss, backup acquisition, failback hysteresis, both lost failsafe.
- no solenoid actuation in backup, duplicate or stale frames.
- measure airtime and enforce duty cycle; legal EIRP with actual antennas.


## Clock and session safety correction
- Sender and receiver millis() clocks are independent. The 16-bit sender tick is not a cross-device freshness clock; do not compare it to receiver millis(). The current command decoder uses local receipt time and sequence checks instead. Receiver watchdogs measure time since locally received authenticated frames.
- Session epoch is now inside the authenticated frame header. However, session negotiation/epoch reset and replay protection across remote/boat reboot are still not implemented; session epoch currently remains a placeholder constant in sender scaffolds. Do not treat replay protection as complete.


## LoRa transmit-slot multiplexing
- E22 limit is one frame per endpoint per3s (25ms estimated airtime,0.83% duty). Remote scheduler queues the latest backup command and suppresses its heartbeat until the command is transmitted; it never intentionally sends a heartbeat and control frame in the same slot. Boat sends backup heartbeat every3s. The duty limiter remains RAM-only and not field-ready.
