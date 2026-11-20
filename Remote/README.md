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
- Redundant backup radio: LoRa UART module candidate (EBYTE E22-900T22S-V2/SX1262 family), EU868; RIGHT antenna.
- ESP32 Wi-Fi/Bluetooth uses onboard antenna; no external Wi-Fi antenna connector planned.
- Both radio links require matching modules on the boat. LoRa is safety/low-rate backup, not sonar streaming.
- Battery: 1S4P 18650; cells/BMS and protection coordination pending.

See `New_Handheld_Remote/README.md`, `docs/NEW_HANDHELD_REMOTE_CONCEPT.md`, and `docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.
