# Flytec R6.5 — Electrical Netlist & Archive Cross-Review

**Date:** 2026-11-20  
**Status:** engineering review; candidate design only, not schematic/ERC/PCB validated.

## Source baseline reviewed
- Boat ver001–005 and Remote ver001–005 archives, including the verified ver005 snapshots.
- Remote development archive ver009.
- Remote R6.3 internal layout package and R6.4 concept-front review package.
- Current branch R4 capture sheets, R4 connector CSV, R5 button/pin allocation, R8 power net draft, R6 BOM, converter calculations, Boat R2 electrical design, Boat R3 connector CSV, and controller resource audit.

## Preserved decisions
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, LVGL 9.6; physical crosswalk still provisional.
- Two discrete MCP23017s: 0x20 for sixteen keys, 0x21 for radio/status plus independent OK.
- Two ADS1115s: 0x4A for four stick axes, 0x4B for battery/NTC/optional rail.
- SC16IS750 @0x48 bridges I2C to E22 UART; no direct E22 wiring to ESP32 RS485/USB UART pins.
- nRF24 SPI GPIO11/12/13 shared with TF-card SPI; serialized bus access required. CE GPIO6 and CSN GPIO15 are candidates only after their Sensor-AD/RS485 board paths are isolated and verified.
- Controller/display from Waveshare single-cell battery input/onboard boost; nRF from TPS63070 3.3V rail; E22 from separate TPS61023 5V rail. Never tie converter outputs together.
- User-approved layout: one power key, two sticks, 2×4 key bank each side, four independent release keys, separate OK, USB-C, dual antenna exits. No duplicate power key.

## Cross-review findings / required actions

### Electrical blockers
1. **Pack/charger mismatch:** the R6.3 power note records a Waveshare battery-capacity recommendation below 2000mAh, while the proposed 1S4P pack using 2500mAh cells would be about 10000mAh. The onboard charger/current, thermals, charge termination and power-path behavior must be confirmed. Do not connect a 1S4P pack to the board battery connector or charge it through USB until this is resolved.
2. **Radio rail load:** E22 TX current and PA/LNA nRF burst current must be measured from the exact purchased modules. The 150mA E22 figure and 300mA nRF budget are provisional; converter, fuse, battery, wiring and thermal calculations are not frozen.
3. **GPIO6/GPIO15:** hardware path isolation/continuity and reset levels required before use.
4. **I2C loading/address scan:** existing GT911, CH422G and RTC plus five proposed peripherals share the bus. Verify address straps, clock limits, cumulative pullups and cable capacitance. Do not blindly add pullups.
5. **SC16 bridge:** verify address straps, crystal frequency/load caps, IRQ polarity, GPIO reset defaults, E22 M0/M1/AUX and UART FIFO servicing.
6. **Joystick ADC:** verify 3.3V pot endpoints, ADS1115 PGA, filters, physical axis orientation, calibration persistence and input-fault detection.
7. **Battery monitor:** divider 220k/68k yields about 0.99V at 4.2V pack; confirm ADS1115 PGA/range, calibration and open/short detection. NTC topology and thresholds are not frozen.
8. **Radio duty-cycle:** E22 profile is only a bench candidate; actual airtime, EU868 channel/EIRP rules and persistent duty accounting must be verified.
9. **Connector freeze:** connector families, pin numbering/keying, current ratings, footprints and physical PORT1 crosswalk remain open. No cable/PCB manufacture from this draft.
10. **Mechanical revision mismatch:** R6.1/R6.3 package estimates shell depth around 87mm with grips/pads, while older R5/R6 status notes use ~69.5–70mm. Use the current CAD source and measure the final printed shell; the 2D front concept is not dimensional evidence.

### Netlist policy
- Keep the R6.4 consolidated netlist as the logical mapping reference, but annotate every unverified connection as candidate.
- RADIO_EN (MCP23017 0x21 GPA3) has no finalized destination/enable logic. Leave it reserved/NC in capture until the controlled rail(s), default state and failure behavior are explicitly defined.
- Do not create actual connector pin numbers or footprints until connector selection and board crosswalk review.
- Power sheet must distinguish BAT_PROT, controller battery input, RADIO_3V3 and LORA_5V; converter outputs are separate.
- Motor/solenoid high-current returns must not pass through remote logic ground; boat-side power design remains separate and incomplete.

## Next gate
1. Update the netlist with these unresolved items and an explicit verification column.
2. Create native schematic sheets with connector placeholders and candidate labels.
3. Run ERC only after symbols/footprints and electrical types are populated.
4. Hardware bring-up order: controller/display; I2C scan; buttons; ADC; radio power/load; nRF; E22 bridge/module; simultaneous load/thermal.
5. No motor or solenoid testing until boat power/driver/coil design and firmware safety gates are reviewed.

