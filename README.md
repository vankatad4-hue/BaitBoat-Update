# Flytec V080 Bait Boat Retrofit

**This GitHub repository is the canonical source of truth for the project.**

## Project layout
- Boat firmware, configuration, documentation and releases: Boat/
- Remote firmware, UI, documentation and releases: Remote/
- Shared architecture, decisions, safety and validation: docs/

## Workflow
Check relevant repository files first; implement approved changes; update documentation; coordinate shared interfaces; review and verify; increment releases without overwriting. GitHub is authoritative.

## Baseline
- Retain Flytec V080 hull, 12V 550 brushed motors, battery indicator and main switch; replace control electronics.
- Boat controller: Heltec Wireless Tracker V1.2 (ESP32-S3 + UC6580 GNSS) candidate.
- Remote: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6.0, Arduino IDE.
- Radio: dual link — primary nRF24L01+ PA/LNA 2.4GHz, backup LoRa EU868 (SX1262/E22 UART module candidate) on both remote and boat; 500m remains an unvalidated range goal.
- Motor driver: Cytron MDD20A candidate, current validation required.
- Sonar: CHIRP echogram/fish-arch module; selection/integration pending.

## Safety
Motor/solenoid current, battery/BMS, wiring/fuses, connectors and pinouts require validation. Outputs default OFF on reset/invalid control. Return-home on link loss only if navigation/system health are valid.

## Release names
Flytec_Boat_verXXX.zip and Flytec_Remote_verXXX.zip. Never overwrite prior releases.

## Remote power progress (2026-11-20)
- Protected 1S4P Li-ion pack; charging is not a custom circuit in this phase.
- **Controller/display 5V:** revised baseline uses Waveshare board's onboard CS8501 single-cell battery boost through PH2.0 battery input. Manufacturer describes single-cell-to-5V boost and lists board load 5V/450mA. Validate onboard converter current, transients and thermal. External TPS61023 is fallback only, not to be paralleled with onboard boost.
- **Radio 3.3V:** TPS63070 buck-boost; 1.5µH candidate; 470k/150k feedback, nominal 3.3067V; effective capacitance and radio bursts need validation.
- Battery protection: BMS, 4A fast-acting fuse candidate, FDS6575 reverse P-MOS candidate (Drain pack-side, Source protected-output), ADC sense and NTC. Exact coordination and tests pending.
- At 3V loaded, estimated controller input ≈0.83A (5V/450mA at assumed 90% onboard efficiency) plus radio ≈0.37A (3.3V/300mA at assumed 90%); combined ≈1.20A. Assumptions only.
- Native KiCad schematic, ERC, PCB, review and hardware bring-up are incomplete.

## Current power documents
- Remote/New_Handheld_Remote/PCB/5V_CONTROLLER_POWER_ARCHITECTURE_REVIEW_R1.md
- Remote/New_Handheld_Remote/PCB/BOOST_CONVERTER_SELECTION_REVIEW_R3.md
- Remote/New_Handheld_Remote/PCB/BATTERY_INPUT_PROTECTION_DESIGN_R4.md
- Remote/New_Handheld_Remote/PCB/POWER_SHEET_CAPTURE_SPEC_R4.md
- Remote/New_Handheld_Remote/PCB/REMOTE_POWER_BOM_CANDIDATES_R4.md
- Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R6.md
- Remote/New_Handheld_Remote/PCB/POWER_DISTRIBUTION_DESIGN_R4.md
- Remote/New_Handheld_Remote/PCB/REMOTE_POWER_AND_CONTROL_BLOCK_DIAGRAM_R1.md
- Remote/New_Handheld_Remote/PCB/REMOTE_PERIPHERAL_INTERFACE_PINOUT_DRAFT_R1.md
- Remote/New_Handheld_Remote/PCB/WAVESHARE_GPIO_RESOURCE_AUDIT_R1.md
- Remote/New_Handheld_Remote/PCB/TPS63070_PIN_NET_MAP_R3.md
- Remote/New_Handheld_Remote/PCB/TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md
- Remote/New_Handheld_Remote/PCB/TPS61023_COMPONENT_CALCULATIONS_R1.md (fallback study)

## Dual-radio redundancy update (2026-11-20)
- Remote RIGHT antenna: EU868 LoRa; LEFT antenna: nRF24 2.4GHz. ESP32 Wi-Fi/Bluetooth uses onboard antenna.
- Boat requires matching LoRa radio/antenna. LoRa backup limited to safety/low-rate traffic, not sonar.
- Failover protocol, power budget and EU/Bulgaria regulatory constraints are documented in `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.

## Firmware integration status — dual radio
- The remote radio source remains a non-operational stub; its API now describes primary nRF24 + backup LoRa and exposes link-state diagnostics.
- Command scaffolding stamps a stable ID and timestamp for future cross-link duplicate suppression.
- Protocol and failover code specification: `Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R1.md`; hardware/radio/antenna architecture: `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.
- No radio driver, authenticated packet codec, failover supervisor, or boat failsafe has been implemented or compiled. Do not operate the boat.
