**SUPERSEDED:** R3 selects matching EBYTE E22-900T22S modules at both endpoints. The Tracker integrated SX1262 is not used for the initial backup pair. See `REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md`.

# Dual-radio redundancy architecture R2 — updated hardware pairing

## Pairing
- **Remote primary:** nRF24L01+ PA/LNA module at 2.4GHz.
- **Remote backup:** EBYTE E22-900T22S UART SX1262 module candidate, configured for EU868.
- **Boat primary:** nRF24L01+ PA/LNA module at 2.4GHz.
- **Boat backup:** use the Heltec Wireless Tracker candidate's **integrated SX1262** if exact board revision/band is confirmed. Do not add a separate LoRa radio on boat by default.
- Remote RIGHT antenna = LoRa EU868. Remote LEFT = nRF24 2.4GHz. Boat requires separate corresponding antennas.
- Wi-Fi/BLE on remote controller uses its onboard antenna.

## LoRa interoperability risk
E22's UART module uses its own air-packet framing/configuration. Boat's integrated SX1262 is controlled at radio PHY level. The two are only interoperable after matching frequency, bandwidth, SF, coding rate, preamble, sync word, CRC, IQ polarity, packet length and modulation configuration. UART transparent data from E22 needs an application frame with framing, command IDs, acknowledgements, and authentication. Bench test this before assuming the backup link works.

## Link safety
nRF primary carries normal responsive control. LoRa backup carries low-rate heartbeat, telemetry, STOP/RETURN and capped manual control. No sonar stream; mission start and bait/hook release disabled on backup by default. Boat autonomy and link watchdog must be local.

## Antenna and RF
Use frequency-specific 50-ohm antenna systems. Keep antenna feed/ground clear of batteries, controller PCB, metal and motor wiring. Check enclosure detuning and RF coexistence with Wi-Fi/BLE. Legal output power includes antenna gain/cable loss; verify EU/Bulgaria channel and duty-cycle/sub-band constraints.

## See
- `Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R1.md`
- `Boat/docs/DUAL_RADIO_BACKUP_LINK_R1.md`
- `Remote/New_Handheld_Remote/PCB/REMOTE_HARDWARE_PIN_ASSIGNMENTS_R1.md`
