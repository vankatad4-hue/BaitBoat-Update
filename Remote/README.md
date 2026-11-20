# Remote Project

Remote touchscreen firmware, UI, configuration, hardware development and versioned releases.

## Existing touchscreen remote
- Source: `source/`
- Documentation: `docs/`
- Releases: `releases/`
- Target board: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, LVGL 9.6.0.

## New handheld hardware
The new game-controller-style remote is an active hardware-development workspace under `New_Handheld_Remote/`, separate from existing remote firmware.
- Power baseline: Waveshare onboard CS8501 1-cell battery-to-5V boost for display/controller; external TPS61023 is fallback only. Dedicated TPS63070 3.3V rail for nRF24.
- Primary radio: nRF24L01+ PA/LNA, 2.4GHz, LEFT antenna.
- Redundant backup radio: matching EBYTE E22-900T22S UART LoRa modules on remote and boat; EU868; RIGHT antenna.
- ESP32 Wi-Fi/Bluetooth uses onboard antenna; no external Wi-Fi antenna connector planned.
- Both radios are paired across remote/boat. LoRa is safety/low-rate backup only, no sonar.
- Battery: 1S4P 18650; cells/BMS and protection coordination pending.

See `New_Handheld_Remote/README.md`, `docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md`, `docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R2.md`, and `New_Handheld_Remote/PCB/REMOTE_HARDWARE_TEST_BUILD_R2.md`.

## Development branch R4
The current dual-radio implementation and electrical test documents are on `hardware-ready-dual-radio-r1`. nRF pins CE6/CSN15 and SPI11/12/13 are provisional; E22 uses SC16IS750 I2C bridge @0x48. Key provisioning is not implemented, and hardware tests have not been compiled/run. No release ZIP has been produced.
