# Remote Project

Remote touchscreen firmware, UI, configuration, hardware development and versioned releases.

## Existing touchscreen remote
- Source: `source/`
- Documentation: `docs/`
- Releases: `releases/`
- Target board: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, LVGL 9.6.0.

## New handheld hardware
The new game-controller-style remote is an active hardware-development workspace under `New_Handheld_Remote/`, separate from the existing remote firmware.
- Concept/mechanical/CAD: preliminary, physical fit not yet verified.
- Power design: TPS61088 5 V display boost candidate and TPS63070 3.3 V nRF radio buck-boost.
- Radio: nRF24L01+ PA/LNA external antenna; ESP32 uses the Waveshare board onboard antenna.
- Battery: 1S4P 18650 pack; cell model and protection verification pending.
- Charging schematic is excluded from current power-design scope.

See `New_Handheld_Remote/README.md` and `docs/NEW_HANDHELD_REMOTE_CONCEPT.md` for baseline, calculations and remaining validation.
