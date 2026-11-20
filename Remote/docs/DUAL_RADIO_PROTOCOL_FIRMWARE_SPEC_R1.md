# Dual-radio application protocol and firmware integration R1

Status: architecture specification only. Not yet integrated into the existing firmware source or compiled.

## Design goals
- Keep UI, joystick sampling, navigation, sonar, and radio transport non-blocking.
- nRF24 is the primary fast link. LoRa is standby/fallback with a strict low airtime budget.
- One logical command identity regardless of transport, so failover retransmission cannot duplicate solenoid/release actions.
- Boat firmware—not remote UI—owns the link-loss safety decision.

## Suggested software components
- RadioTransport interface: begin, send, poll/receive, link metrics, reset/reconfigure.
- Nrf24Transport: primary driver; existing packet settings and 250kbps baseline retained until measured.
- LoRaUartTransport: E22 module serial interface, AUX-ready and M0/M1 mode handling; exact library/configuration depends on selected module revision.
- LinkManager: state machine, selection, failover/failback, hysteresis, telemetry and UI state.
- PacketCodec: versioned packet, validation, sequence freshness, duplicate cache, CRC/authentication.
- CommandRouter: validate mode/limits and apply only once; output action ACKs.
- SafetySupervisor: local boat watchdog and failsafe independent of the remote.

## Packet envelope (wire format proposal)
Fields in serialized order:
1. protocol_version (u8)
2. sender_id (u16)
3. receiver_id (u16)
4. message_type (u8)
5. link_epoch (u8; changes on reconnect/session reset)
6. sequence (u16)
7. command_id (u32 for action/command messages; zero for pure telemetry)
8. timestamp_ms (u32)
9. payload_length (u8; bounded)
10. payload (bounded, radio-specific MTU)
11. CRC16 (transport/application error detection)
12. authentication tag (truncated AES-CCM tag, length fixed by protocol revision)

Payloads:
- HEARTBEAT: mode, battery/health flags, active-link identifier, last command sequence acknowledged.
- CONTROL: joystick axes normalized signed values, motor mode, throttle cap, deadman/failsafe status.
- TELEMETRY: boat battery, link metrics, GNSS validity, heading validity, mission state.
- ACTION: STOP, RETURN_HOME, mission action. Actions carry unique command_id and must be idempotent.
- ACK: original message type, sequence and command_id, status/result.

CRC is not authentication. Before autonomous operation, provision a per-device/project key and implement cryptographic authentication/replay protection. Key management and secure provisioning are not yet designed. Reject wrong IDs, invalid length, stale timestamp, invalid sequence, malformed/authentication-failed packets and unsupported modes.

## LinkManager states
- PRIMARY: nRF is selected for control; LoRa carries sparse heartbeat/link-check traffic.
- DEGRADED: primary misses threshold; display warning and disallow risky mission/release actions.
- BACKUP: valid LoRa ACK/heartbeat confirms link; restricted profile.
- NO_LINK: both paths stale. Boat local watchdog takes over; remote shows disconnected.
- RECOVERY: primary has recovered for N consecutive validated packets; handover at command boundary after holdoff.

State change must not clear action IDs or cause a queued action to execute twice. Keep a short rolling duplicate-action cache at the boat. Command completion must be ACKed from boat-side state, not merely radio TX success.

## Initial timing values (all provisional)
- Primary control/heartbeat target: 20–50Hz, chosen after latency and airtime tests.
- Primary warning: 3 missed heartbeats or 150–250ms stale, whichever configured; calibrate to actual packet period.
- LoRa standby link probe: 1–2Hz initial.
- LoRa fallback control: 2–5Hz initially, with a reduced motor output cap.
- Primary recovery: 5–10 consecutive successful fresh ACKs plus 1–3s holdoff (initial values to test).
- LoRa transmit airtime: track per regulatory channel and enforce applicable duty-cycle or spectrum-access rule. If duty budget prevents TX, suppress non-safety telemetry and indicate status.

## Backup-mode command policy
Allowed initially: STOP, RETURN_HOME, low-rate manual steering with speed cap, heartbeat and telemetry.
Disabled initially: new mission start, waypoint edits, bait/hook release, firmware update, calibration, sonar streaming.
No command should be accepted unless the boat's current mode, safety checks and packet authentication permit it.

## Fail-safe independent of link
Boat-side watchdog tracks the freshness of valid control packets from either link. If both links expire:
- If mission is already autonomous AND GNSS/heading/battery/geofence and system health are valid, continue the configured return-home procedure.
- Otherwise stop propulsion and enter safe waiting state.
- Solenoid outputs must remain OFF; do not repeat a release action.
- On reconnection, require an explicit state synchronization before resuming manual commands.

## Non-blocking code rules
- Do not use long delays in radio/UI tasks.
- Do not block LVGL or joystick ADC acquisition waiting for UART AUX or packet ACK.
- Use fixed-size packet buffers; no heap allocation in high-frequency control/radio paths.
- Use monotonic timestamps and bounded queues.
- Radio driver tasks own their hardware; LinkManager communicates through queues/events.
- Radio TX queue: safety messages highest priority; periodic telemetry lowest. Drop stale control frames rather than transmitting them late.
- Use a radio bus mutex only for shared SPI (nRF/SD); LoRa UART is independent. Never hold SPI lock while waiting for radio ACK.
- Keep logs rate-limited and avoid logging raw packets continuously.

## Pseudocode (integration target)
```cpp
void linkManagerTick(uint32_t now) {
    updateMetrics(nrf, lora);
    switch (state) {
      case PRIMARY:
        if (nrfFresh()) sendControl(NRF, latestControl);
        if (nrfMissThresholdExceeded()) state = DEGRADED;
        break;
      case DEGRADED:
        showLinkWarning();
        if (loraFreshAndAuthenticated()) state = BACKUP;
        else if (nrfFresh()) state = PRIMARY;
        else if (bothLinksStale()) state = NO_LINK;
        break;
      case BACKUP:
        sendRestrictedControlIfDue(LORA, latestControl);
        if (bothLinksStale()) state = NO_LINK;
        if (nrfRecoveryStreakReached()) state = RECOVERY;
        break;
      case NO_LINK:
        showNoLink();
        // Boat performs local failsafe; do not issue a delayed stale command.
        if (loraFreshAndAuthenticated()) state = BACKUP;
        else if (nrfFresh()) state = RECOVERY;
        break;
      case RECOVERY:
        synchronizeBoatState();
        if (handoverReady()) state = PRIMARY;
        else if (bothLinksStale()) state = NO_LINK;
        break;
    }
}
```
This is illustrative pseudocode—not compilable firmware. Exact state timing and packet structures must be synchronized between remote and boat.

## Verification matrix
- nRF primary healthy; LoRa standby reachable and airtime-limited.
- nRF disabled/interference: transition to LoRa with UI warning, low-speed control and ACKs.
- LoRa lost while nRF primary healthy: primary continues; no unnecessary mode change.
- Both links lost: boat local failsafe; no delayed queued steering or solenoid action.
- Primary restored: stable recovery and state sync; no oscillation.
- Packet duplicates across links, stale packet, wrong device ID, invalid CRC/auth, sequence rollover.
- Button/action held during failover; one release/action only, action state ACK visible.
- Duty-cycle exhaustion and module AUX busy; safety queue remains bounded.
