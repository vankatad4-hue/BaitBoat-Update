# R30H-33 — E22 LoRa redundant link audit (2026-11-23)

**Verdict: wired in design and protocol implemented, NOT ready for real redundant operation.** The two boards and their E22 carriers have not been assembled or RF-tested. This audit does not enable any radio gate, actuator, or release.

## End-to-end physical wiring

| Net | Boat ESP32-S3 DevKitC-1 | Remote Waveshare via SC16IS750 @0x48 | Status |
|---|---|---|---|
| UART TX → E22 RXD | GPIO17 TX | SC16IS750 TX output → E22 RXD | Design assigned; unverified physical wiring |
| UART RX ← E22 TXD | GPIO18 RX | SC16IS750 RX input ← E22 TXD | Design assigned; unverified physical wiring |
| M0 | GPIO15 | SC16 GPIO0 | Assigned |
| M1 | GPIO16 | SC16 GPIO1 | Assigned |
| AUX input | GPIO14 | SC16 GPIO2 | Assigned |
| NRST active-low | GPIO6 in pin map | SC16 GPIO3 | **Implemented in R30H-33**: Boat E22BoardPins includes NRST and drives GPIO6 through reset callback; physical levels unverified |
| E22 VCC | Regulated 5 V | Regulated 5 V boost rail | Selected design, voltage/ripple and available converter unverified |
| E22 GND | Common logic/RF return | Common logic/RF return | Assigned |
| Antenna | Dedicated compatible 868 MHz antenna | Dedicated right-side 868 MHz antenna | Verify actual module carrier, antenna, EU region settings |

Remote I²C ESP GPIO8 SDA / GPIO9 SCL -> SC16IS750. Boat direct UART does not share nRF SPI. Remote nRF24 uses SC18IS606-EVB SPI bridge and separate left antenna; TF remains on onboard SPI. **No missing ESP32 GPIO is required for the E22 data path.**

## Firmware and failover checks

- Both endpoints use `E22UartLink` with identical 12-byte configuration commands and fixed RadioFrame framing.
- Both use 9600 8N1 UART; Remote SC16IS750 uses a 1.8432 MHz crystal assumption and divisor 12. Confirm actual breakout crystal.
- Both use the same E22 configuration byte array, expected readback, CRC16 framing and application session/authentication checks.
- Remote `boat_radio.cpp` instantiates SC16IS750 @0x48 and E22 link; Boat `BoatDualRadioPort` instantiates direct UART E22.
- Backup is intentionally restricted: STOP and RETURN HOME; no hopper/hook release or unrestricted manual control on LoRa. The primary nRF is required for full control, session establishment and ordinary telemetry.
- Backup heartbeat and link freshness are present; after loss of **both** links, Boat has conditional RTH/STOP policy. Backup alone does not guarantee ability to establish a fresh session after restart.
- `BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED`, `BOAT_E22_UART_LOGIC_LEVELS_VERIFIED`, `BOAT_E22_UART_PIN_AUDIT_SIGNED`, `BOAT_E22_RF_AIRTIME_AUDIT_SIGNED`, `REMOTE_E22_BRIDGE_AUDIT_SIGNED`, `REMOTE_E22_RF_INTEROPERABILITY_VERIFIED`, `REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED` remain **0** by design. Do not set these to 1 based on diagrams or CI.
- **Resolved in source:** Boat `E22BoardPins` now includes GPIO6 NRST and an active-low reset callback, matching Remote SC16 GPIO3 reset behavior. No actual reset waveform has been measured.
- **Critical timing gap:** `EST_AIRTIME_MS=25` is explicitly an unverified estimate and the 1% duty bucket is provisional. Verify actual on-air duration and regional compliance; do not claim guaranteed STOP latency.
- **Current code quality gap:** Previous R30H-32 numeric radio macros referenced C++ namespace values inside `#if`; R30H-33 restored preprocessor-safe numeric macros with static assertions. PlatformIO builds/host CI have not yet passed on these commits.

## Completion criteria (normal functional integration, not hardware safety signoff)
1. Verify Boat GPIO6 NRST reset timing and E22 carrier pin numbering with the actual modules.
2. Confirm actual Remote SC16IS750 1.8432 MHz oscillator, I²C 0x48, 3.3 V logic and 5 V E22 power.
3. Verify matching E22 profile, UART frame exchange, authenticated backup heartbeat and STOP/RTH command acceptance with assembled radios.
4. Simulate nRF-only loss: STOP/RTH over backup; simulate both links lost >10s: conditional RTH else STOP; verify session recovery, duplicate rejection, LoRa airtime limits.
5. Re-run Boat/Remote PlatformIO and portable host CI; no PASS until observed.

**Do not use backup as a promise of unrestricted remote control.** In the current implementation it is a restricted safety/return communications channel, not a complete duplicate of the nRF primary link.
