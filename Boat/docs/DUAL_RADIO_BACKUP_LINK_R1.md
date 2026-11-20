# Boat-side LoRa redundancy implementation requirements R1

This file is the boat-side counterpart to the remote dual-radio decision. Boat hardware must include both radio modules and their antennas; remote-only LoRa is not a link.

## Hardware additions
- Primary nRF24L01+ PA/LNA, 2.4GHz.
- Backup LoRa UART/SX1262-family candidate EBYTE E22-900T22S-V2 configured for EU863–870 operation.
- Separate matched 50Ω antenna for each radio. Keep LoRa antenna/feed away from motor wiring, high-current switching, battery harness, metal and the nRF antenna.
- LoRa power branch/decoupling sized for peak module TX current and confirmed by transient/thermal testing. Exact boat supply voltage/converter depends on selected module variant and board rail design.
- UART TX/RX at 3.3V logic, AUX and M0/M1 mode pins wired to verified MCU GPIOs. Pin assignments are not yet frozen.

## Firmware requirements
- Same versioned packet/protocol and LinkManager state machine as remote; see Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R1.md.
- nRF primary; LoRa standby/fallback; low-rate only on LoRa.
- Duplicate command suppression shared across transports. Bait/hook release must use unique action ID, ACK, and state confirmation; never re-execute a duplicate/retry.
- Backup mode accepts STOP, RETURN and restricted low-speed manual steering only. New mission and release actions disabled by default on backup.
- Boat-side watchdog independent of remote: when both links expire, return home only with validated GNSS/heading/battery/geofence/system health; otherwise stop motors, keep solenoids OFF, and await recovery.
- Reconnection requires state sync and explicit mode-safe handover; never replay stale control frames.

## Regulatory and test blockers
- Confirm exact module regional variant, CE documentation, selected EU/Bulgaria channel, permitted output power/EIRP/ERP, antenna gain/loss, duty-cycle or LBT/AFA requirements.
- Test both links on bench and outdoors over water; record range, latency, packet error rate, antenna installation, coexistence, failsafe behavior and power-rail transients.
- This is a requirements record, not merged/compiled boat firmware or completed hardware.
