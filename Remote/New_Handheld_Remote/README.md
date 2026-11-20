# New Handheld Remote Hardware Development

This folder contains the hardware design workspace for the new handheld bait-boat remote.

## Current concept
- 4.3-inch central touchscreen.
- Separate power button above screen; USB-C service connector.
- Two joysticks above two button banks; 16 buttons total, 2×4 per side.
- Ergonomic grips; no shoulder/rear buttons.
- **Primary:** nRF24L01+ PA/LNA 2.4GHz radio; LEFT external antenna.
- **Backup:** LoRa module (SX1262/E22 UART family candidate) in EU868; RIGHT external antenna. The boat must also carry a matching LoRa radio.
- Waveshare ESP32-S3 Wi-Fi/Bluetooth uses its onboard antenna; no external Wi-Fi antenna path.
- 1S4P Li-ion pack, four matched 18650 cells; cell model, BMS and protection validation pending.

## Workspace
- CAD: shell, front/rear, internal mockup; preliminary only.
- PCB: power design, connector plan, schematic drafts.
- MECHANICAL: dimensions, component envelopes, placement.
- WIRING: cable and connector interface documents.
- docs: design decisions, RF, bring-up.

## Power baseline (updated)
- Controller/display 5V: Waveshare board onboard CS8501 boost through single-cell battery interface; external boost is fallback only. Never parallel boost outputs.
- nRF24 rail: dedicated TPS63070 RADIO_3V3 buck-boost.
- LoRa module supply: provisional filtered 5V branch from CS8501 rail, subject to simultaneous display/backlight+nRF+LoRa current/thermal/transient test. Separate boost if required, never parallel.
- 1S4P pack, protection fuse and reverse-polarity MOS candidates pending validation.
- Charging circuit remains outside custom design scope; board USB/charging behavior still requires bench verification.

## Dual-radio redundancy
Primary nRF24 remains the high-rate responsive link. LoRa is backup for heartbeat, low-rate telemetry, capped low-rate steering, STOP and RETURN. Mission-start and bait/hook release are disabled on backup by default. Dual-link failover, handover hysteresis, duplicate suppression, authenticated/replay-safe packets, independent boat failsafe, and link-state UI are software requirements. See `../docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.

## Status / limitations
Documents are engineering planning/net-level design records, not fabrication-ready schematics or BOM. Native KiCad capture, pin/footprint audit, ERC, PCB layout, firmware integration and bench/field testing remain incomplete.
