# Dual-Radio Protocol and Firmware Specification R3

## Physical radios
- Primary: nRF24L01+ PA/LNA on both endpoints,2.4GHz,channel76,250kbps,fixed32-byte payload,CRC16,auto-ack/retry,PA_LOW initial.
- Backup: identical EBYTE E22-900T22S UART LoRa modules on both endpoints. Tracker integrated SX1262 is unused in initial test.
- Remote LEFT antenna=nRF24 2.4GHz; RIGHT=E22 EU868. ESP32 Wi-Fi/Bluetooth onboard antenna.
- Remote E22 UART via SC16IS750 I2C bridge@0x48; boat direct hardware UART with pin allocation TBD.
- E22 channel18=868.125MHz,air62.5kbps,subpacket32,10dBm,transparent,9600 8N1. 1% duty sub-band; no field operation until compliance/airtime confirmed.

## Wire frame (packed32 bytes)
Fields: magic2,protocolVersion1,type1,sourceId1,destinationId1,sequence16,commandId16,tick16(sender-local),flags1,sessionEpoch8,payload8,CRC16-CCITT, truncated HMAC-SHA256 tag8.
- Sender-local tick is not compared with receiver millis; clocks are unsynchronized.
- CRC protects accidental corruption. HMAC only authenticates if a per-device/project key is provisioned. Current key store returns false, fail-closed.
- Session epoch8 is inside authenticated header. Session-start type7 uses sequence0 and payload[0]=epoch. It allows sequence guard reset. Epoch is random8 in scaffolds; persistent anti-replay/session negotiation across reboot is incomplete and not safe for unattended operation.
- sequence/commandId are16-bit on air; logical commandId retained across transport retry, command sequence assigned per transmitted frame.
- Duplicate action cache at boat is required across links. Action execution and state ACK are not implemented.

## Message types and payload
- SESSION_START: authenticated new session epoch; sequence0.
- HEARTBEAT: link-health flags.
- CONTROL/COMMAND: type,mode,left/right int16 LE(-1000..1000), action flags, reserved.
- TELEMETRY/ACK: detailed layout still incomplete; payload8B only.
- Reject malformed frame, wrong IDs/version, bad CRC/tag, wrong session, sequence replay, unsupported mode/action.

## LinkManager
- PRIMARY: nRF24 selected.
- DEGRADED: primary misses; warning; no new risky mission/release.
- BACKUP: valid authenticated E22 session and fresh packets; restricted profile.
- NO_LINK: both stale; boat local failsafe.
- RECOVERY: stable primary packet streak then handover with holdoff.
- Primary heartbeat100ms, stale250ms provisional.
- Backup heartbeat/control slot3s, stale6s provisional.
- Recovery8 good primary frames +1.5s holdoff.
- No link sends no queued stale steering.

## Backup command policy
- Allowed: emergency STOP, RETURN_HOME (only if boat safety validation passes), restricted manual steering capped25%.
- Disabled: mission start, waypoint edits, bait/hook release, clear-stop, calibration, update, sonar streaming.
- LoRa backup transmit slot: one frame per endpoint per3s target. Remote command queue priority STOP>RETURN>manual; latest manual replaces older manual; lower priority cannot overwrite STOP/RETURN. Heartbeat suppressed while command queued. Boat sends heartbeat at3s.
- Estimated25ms/frame gives0.83% at one frame/3s. Actual airtime and duty-counter persistence must be tested; current limiter is RAM-only and not field-compliant.

## Boat requirements
- Boat DualRadioPort validates frame and policy, queues command, reports duplicate-action flag. Main application must not execute duplicate action and must ACK confirmed state.
- Local watchdog independent of remote. If both links expire: return home only if GNSS/heading/battery/geofence/system health valid and autonomous mission active; otherwise stop motors, solenoids OFF.
- Reconnection requires explicit state sync. No stale command replay.
- Hardware/actuator executor, app ACK, failsafe integration and secure key/session provisioning are not implemented.

## Firmware code inventory
Remote: nRF24/E22 transports, SC16IS750 bridge, frame codec, command codec, LinkManager, heartbeat scheduler and UI link state.
Boat: nRF/E22 transport, dual radio port, command queue, dedupe/failsafe helper.
**Neither firmware has been compiled or hardware-tested.** Key provisioning is disabled. Boat port is not called by main boat app.

## Test plan
1. Compile radio sketches and firmware with pinned Arduino/RF24 versions.
2. Verify session-start and sequence golden vectors; HMAC/CRC invalid vectors.
3. nRF packet loss/retry, shared SPI/SD and latency.
4. E22 config/readback, UART loopback, pair RX/TX, AUX, module power.
5. Failover/failback, duplicate action, STOP/RETURN/manual restrictions, stale packets, reboot/session.
6. Both links lost: local boat failsafe, motors/solenoids physically isolated.
7. Record packet loss/latency/airtime/power/RF. Verify Bulgarian/EU channel/EIRP/duty before field use.
