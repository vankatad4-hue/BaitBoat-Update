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
- Radio: dual link — primary nRF24L01+ PA/LNA 2.4GHz; backup matching E22-900T22S UART LoRa modules on both endpoints. 500m range unvalidated.
- Motor driver: Cytron MDD20A candidate, current validation required.
- Sonar: CHIRP echogram/fish-arch module; selection/integration pending.

## Safety
Motor/solenoid current, battery/BMS, wiring/fuses, connectors and pinouts require validation. Outputs default OFF on reset/invalid control. Return-home on link loss only if navigation/system health are valid.

## Release names
Flytec_Boat_verXXX.zip and Flytec_Remote_verXXX.zip. Never overwrite prior releases.

## Remote power progress (2026-11-20)
- Protected 1S4P Li-ion pack; charging is not a custom circuit in this phase.
- **Controller/display 5V:** revised baseline uses Waveshare board's onboard CS8501 single-cell battery boost through PH2.0 battery input. Manufacturer describes single-cell-to-5V boost and lists board load 5V/450mA. Validate onboard converter current, transients and thermal. External TPS61023 is fallback only, not to be paralleled with onboard boost.
- **nRF24 radio 3.3V:** TPS63070 buck-boost; 1.5µH candidate; 470k/150k feedback, nominal 3.3067V; effective capacitance and RF bursts need validation.
- **LoRa backup:** E22 module supply from protected 1S battery rail (2.7–5.5V range) with legal reduced power configuration; 5V needed for guaranteed maximum output. Local bulk decoupling and TX droop test.
- Battery protection: BMS, 4A fast-acting fuse candidate, FDS6575 reverse P-MOS candidate (Drain pack-side, Source protected-output), ADC sense and NTC. Exact coordination and tests pending.
- Previous 1.20A controller+nRF estimate was based on assumed loads/efficiency and did not include LoRa. Recalculate and measure total load with LoRa enabled before selecting converters, fuse and battery/BMS.
- Native KiCad schematic, ERC, PCB, review and hardware bring-up are incomplete.

## Current power documents
- Remote/New_Handheld_Remote/PCB/5V_CONTROLLER_POWER_ARCHITECTURE_REVIEW_R1.md
- Remote/New_Handheld_Remote/PCB/BOOST_CONVERTER_SELECTION_REVIEW_R3.md
- Remote/New_Handheld_Remote/PCB/BATTERY_INPUT_PROTECTION_DESIGN_R4.md
- Remote/New_Handheld_Remote/PCB/POWER_SHEET_CAPTURE_SPEC_R5.md
- Remote/New_Handheld_Remote/PCB/REMOTE_POWER_BOM_CANDIDATES_R5.md
- Remote/New_Handheld_Remote/PCB/POWER_SCHEMATIC_NET_DRAFT_R7.md
- Remote/New_Handheld_Remote/PCB/POWER_DISTRIBUTION_DESIGN_R4.md
- Remote/New_Handheld_Remote/PCB/REMOTE_POWER_AND_CONTROL_BLOCK_DIAGRAM_R1.md
- Remote/New_Handheld_Remote/PCB/REMOTE_PERIPHERAL_INTERFACE_PINOUT_DRAFT_R1.md
- Remote/New_Handheld_Remote/PCB/WAVESHARE_GPIO_RESOURCE_AUDIT_R1.md
- Remote/New_Handheld_Remote/PCB/TPS63070_PIN_NET_MAP_R3.md
- Remote/New_Handheld_Remote/PCB/TPS63070_COMPONENT_CALCULATIONS_REVIEW_R2.md
- Remote/New_Handheld_Remote/PCB/TPS61023_COMPONENT_CALCULATIONS_R1.md (fallback study)

## Dual-radio redundancy update (2026-11-20)
- Remote RIGHT antenna: 868MHz LoRa; LEFT antenna: nRF24 2.4GHz. ESP32 Wi-Fi/Bluetooth uses onboard antenna. Boat uses separate 2.4GHz and 868MHz antennas.
- Matching E22 modules are used at both endpoints. LoRa backup is duty-limited low-rate safety traffic, not sonar.
- Failover protocol, power budget and EU/Bulgaria regulatory constraints are documented in `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.

## Firmware integration status — dual radio
- Remote source includes nRF24/E22 transports, SC16 bridge, CRC/HMAC codec and failover manager. Pin configuration remains provisional; key provisioning fails closed.
- Command scaffolding stamps a stable ID and timestamp for future cross-link duplicate suppression.
- Protocol and failover code specification: `Remote/docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R1.md`; hardware/radio/antenna architecture: `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md`.
- Boat source has dual-radio receive/dedupe/failsafe scaffolds but no actuator integration. Neither side has secure key provisioning, app ACK/session negotiation or verified hardware build. Do not operate the boat.

- Remote/New_Handheld_Remote/PCB/SCHEMATIC_CAPTURE_SHEETS_R1.md
- Remote/New_Handheld_Remote/PCB/SCHEMATIC_NETLIST_BLOCKS_R1.md
- Remote/New_Handheld_Remote/PCB/SCHEMATIC_CAPTURE_CHECKLIST_R1.md

## Hardware/software work branch
A dedicated implementation branch `hardware-ready-dual-radio-r1` contains R2 test-build electrical specs, pin connections, revised enclosure envelope, packet codec and dual-link supervisor scaffold. Boat uses matching E22 UART module; Tracker integrated SX1262 is unused in initial prototype. Hardware remains non-fabrication-ready until GPIO crosswalk, KiCad/ERC/DRC, motor current, solenoid current and RF compliance/interoperability gates pass.

- Dual-radio R3 pairing: identical E22 UART modules on both endpoints; Tracker's integrated SX1262 is unused in this initial test configuration. See `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md`.

## Current implementation snapshot
- Remote software now includes HMAC/CRC packet codec, command decoder, failover manager, E22 driver with SC16IS750 I2C-UART bridge, and nRF24 transport class. E22 configuration/readback is integrated at startup; nRF pin candidate CE6/CSN15, SPI11/12/13.
- Secure key provisioning is deliberately not implemented; no authenticated link is accepted and command TX fails closed. E22 airtime accounting is RAM-only, so it is lab-only. Boat receive port and dedupe/failsafe helpers are scaffolds, not actuator integration.
- Detailed test-build specs and dedicated E22 bench sketches are on development branch `hardware-ready-dual-radio-r1`; no release ZIP or native KiCad schematic has been produced.

## Test-build artifacts
- Remote electrical capture sheets, pin map, pin-connection CSV, BOM and mechanical envelope under `Remote/New_Handheld_Remote/PCB/` and `MECHANICAL/`.
- Boat electrical schematic specification, logical connector CSVs, motor-driver fit review and BOM under `Boat/docs/`.
- Separate radio bench sketches under `Remote/hardware_test/` and `Boat/hardware_test/`.
- Development branch `hardware-ready-dual-radio-r1`; native KiCad/ERC/DRC and release ZIPs not yet created.
