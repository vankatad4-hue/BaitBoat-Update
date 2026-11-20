# New Handheld Remote Hardware Development

This folder contains the hardware design workspace for the new handheld bait-boat remote.

## Current concept
- 4.3-inch central touchscreen.
- Separate power button above screen; USB-C service connector.
- Two joysticks above two button banks; 16 buttons total, 2×4 per side.
- Ergonomic grips; no shoulder/rear buttons.
- **Primary:** nRF24L01+ PA/LNA 2.4GHz radio; LEFT external antenna.
- **Backup:** EBYTE E22-900T22S UART/SX1262 module candidate in EU868; RIGHT external antenna. Boat will use an identical E22 UART module for first prototype. Tracker's integrated SX1262 remains unused unless compatibility is proven.
- Waveshare ESP32-S3 Wi-Fi/Bluetooth uses its onboard antenna; no external Wi-Fi antenna path.
- 1S4P Li-ion pack, four matched 18650 cells; cell model, BMS and protection validation pending.

## Workspace
- CAD R6 concept: reference-inspired ergonomic shell with button banks corrected to **4 columns × 2 rows per side**; OpenSCAD sources plus a separately generated and locally validated prototype STL/preview package. GitHub CAD-export commit is still pending workflow verification. Exact component fit is pending.
- Right-bank upper four physical keys now dedicated to independent hopper-left, hopper-right, hook-left, hook-right. Release requires two-step hold-confirm; backup LoRa prohibited.
- Release command IDs/codecs and native host test added; button scanning and boat actuator integration remain pending.
- PCB: power design, connector plan, schematic drafts; R3 BOM and R5 button/expander map added.
- MECHANICAL: dimensions, component envelopes, placement.
- WIRING: cable and connector interface documents.
- docs: design decisions, RF, bring-up.

## Power baseline (updated)
- Controller/display 5V: Waveshare board onboard CS8501 boost through single-cell battery interface; external boost is fallback only. Never parallel boost outputs.
- nRF24 rail: dedicated TPS63070 RADIO_3V3 buck-boost.
- LoRa E22 VCC: dedicated BAT_PROT→TPS61023→5V converter; CS8501 is reserved for controller/display. Module UART/M0/M1/AUX remain3.3V logic. Never parallel converter outputs.
- 1S4P pack, protection fuse and reverse-polarity MOS candidates pending validation.
- Charging circuit remains outside custom design scope; board USB/charging behavior still requires bench verification.

## Dual-radio redundancy
Primary nRF24 remains the high-rate responsive link. LoRa is backup for heartbeat, low-rate telemetry, capped low-rate steering, STOP and RETURN. Mission-start and bait/hook release are disabled on backup by default. Dual-link failover, handover hysteresis, duplicate suppression, authenticated/replay-safe packets, independent boat failsafe, and link-state UI are software requirements. See `../docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md`.

## Status / limitations
Documents are engineering planning/net-level design records, not fabrication-ready schematics or BOM. Native KiCad capture, pin/footprint audit, ERC, PCB layout, actual radio drivers, key provisioning, packet ACK/failover, firmware compile and bench/field tests remain incomplete.


## R2 test-build design
- Pin plan and bridge architecture: `PCB/REMOTE_HARDWARE_PIN_ASSIGNMENTS_R2.md`.
- Detailed circuit capture spec: `PCB/SCHEMATIC_CAPTURE_SHEETS_R2.md`.
- Pin wiring CSV: `PCB/REMOTE_ELECTRICAL_PIN_CONNECTIONS_R2.csv`.
- Test build and validation sequence: `PCB/REMOTE_HARDWARE_TEST_BUILD_R2.md`.
- BOM: `PCB/REMOTE_BOM_R2.md`.
- Mechanical envelope: `MECHANICAL/REMOTE_ENVELOPE_R2.md`.


## Radio pairing revision R3
Use the identical E22-900T22S UART module on both ends. This reduces protocol interoperability risk compared with mixing E22 transparent UART with a raw SX1262 driver. See `../docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md`.

## R5 preview and joystick input

- [R6 concept front preview SVG](CAD/REMOTE_CONCEPT_R6.svg)
- [R6 mechanical concept document](MECHANICAL/REMOTE_CONCEPT_R6.md)
- [R6 button map (4×2 per side)](MECHANICAL/REMOTE_BUTTON_MAP_R6.md)
- [Historical R5 layout preview](CAD/REMOTE_PREVIEW_R5.svg)
- [Joystick ADC integration draft](../../docs/JOYSTICK_ADC_INTEGRATION_R1.md)
- ADS1115 driver added to source; actual wiring/address remains provisional and hardware validation is pending.
