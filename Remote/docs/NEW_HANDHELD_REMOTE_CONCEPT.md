# New Handheld Remote — Concept & Engineering Baseline

**Updated:** 2026-11-18  
**Status:** Design concept and component direction; not final CAD, PCB, firmware or purchased hardware.

## Physical/UI concept
- Ergonomic controller, dark graphite/matte black.
- Central 4.3-inch landscape touchscreen.
- Two analog joysticks above button banks.
- 16 buttons total: left and right banks, each 2 columns × 4 rows.
- No PlayStation symbols; no shoulder/rear buttons; no decorative blue LEDs/backlighting.
- Separate power ON/OFF button above screen; USB-C service port at lower edge.
- Textured lower grips and sculpted shell; reference styling is the design target.
- Provisional envelope: 310 × 190 × 70 mm; dimensions and fit remain unverified.

## Button map (preliminary)
Left: MENU, HOME, OPTIONS, PAUSE, STOP, WP SELECT, SAVE WAYPOINT, NAV.
Right: BAIT, HOOK, OK/CONFIRM, BACK, SONAR, MAP, CAM (reserved), LIGHT (reserved).
BAIT/HOOK require deliberate confirmation/hold behavior. STOP overrides propulsion and inhibits release; safety logic must be separately specified and tested.

## Display/controller
Selected direction: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480. Onboard ESP32 antenna is accepted; no external ESP32 antenna path is required.

## Radio
nRF24L01+ PA/LNA module with SMA antenna for boat command/telemetry. Initial radio test rate 250 kbps; 500 m range remains a goal, not validated.
- ANT_NRF: one external antenna exit for nRF radio.
- ESP32 RF: onboard antenna on Waveshare board.
- Both systems operate in 2.4 GHz band; coexistence, scheduling, antenna/layout and regulatory EIRP require testing.

## Battery
1S4P Li-ion pack, four matched 18650 cells in parallel. Earlier 2500 mAh/cell target is not yet a cell selection. Nominal capacity/energy are estimates until cell model is selected. Protection, fuse, pack temperature, mechanical retention and wiring need verification. Charging circuit is outside current power-schematic scope.

## Power electronics baseline (R1)
- Protected battery input; charging schematic excluded.
- 5 V display/controller rail: TPS61088 boost candidate; component calculations and thermal validation pending.
- 3.3 V radio rail: TPS63070 buck-boost, adjustable.
- TPS63070 feedback R1=470 kΩ, R2=150 kΩ; calculated nominal output 3.307 V.
- Inductor candidate: Coilcraft XFL4020-152ME, 1.5 µH.
- Input capacitors: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
- Output capacitors: 3×22 µF 16 V X6S 0805 + 10 µF 0603, 76 µF nominal.
- VAUX: 100 nF.
- Proposed configuration: FB2=GND, VSEL static LOW, PS/SYNC HIGH power-save, EN defined via 10 kΩ pull-up, optional PG pull-up. Verify exact pin-state wiring against datasheet before capture.
- 200 mA radio output load is a design budget, not measured. Estimated radio battery current at 3.0 V is ~245 mA assuming 90% efficiency.
- Whole remote preliminary estimate ~1.13 A at 3.0 V battery using display 450 mA at 5 V/85% and radio 200 mA/90%; assumptions, not measurements.

## Mechanical development
Shell CAD, component envelopes, joystick/button spacing, antenna clearance, battery fit and cable routing are still being refined. The reference image is stylistic only. Validate with physical component samples and printed fit prototype.

## Repository files
Hardware design workspace: `Remote/New_Handheld_Remote/`. See its README, component integration report, power design, schematic net draft and TPS63070 calculation review.
