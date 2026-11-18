# Remote Firmware

Remote touchscreen firmware, UI, configuration and release archives.

- Source: `source/`
- Hardware, UI, map and project documentation: `docs/`
- Versioned releases: `releases/`

Target hardware is the Waveshare ESP32-S3-Touch-LCD-4.3 standard board with LVGL 9.6.0. The repository is the canonical source of truth. Check it before editing and update relevant docs alongside source changes. Do not overwrite existing release archives.

## Alternative handheld remote concept

A new game-controller-style handheld remote concept is under exploration, separate from the current Waveshare-based remote. See [`docs/NEW_HANDHELD_REMOTE_CONCEPT.md`](docs/NEW_HANDHELD_REMOTE_CONCEPT.md). This concept is not yet approved as a replacement firmware/hardware baseline.

## New handheld remote hardware development
The new handheld hardware concept is now an active engineering workspace, separate from existing touchscreen firmware work. Hardware documents live in `New_Handheld_Remote/`.

Power baseline: 1S4P protected battery (charging circuit excluded), TPS61088 5 V display boost candidate, TPS63070 3.3 V dedicated nRF radio buck-boost. Calculated radio-rail baseline and current estimates are documented in the hardware README and PCB design files. Native KiCad capture and validation remain pending.
