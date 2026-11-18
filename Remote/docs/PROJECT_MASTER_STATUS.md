# Flytec V080 bait boat and remote — master project record

**Last consolidated:** 2026-11-18

## Archive versioning
Every distributed project ZIP uses `Flytec_Remote_verXXX.zip`. Start ver001; increment by one per changed project deliverable; do not overwrite prior release ZIPs.

## Project objective
Retrofit Flytec V080 bait boat while retaining hull, motors, battery indicator and main switch. Replace boat control electronics with central custom PCB/removable connectors. Remote hardware is being redesigned around a new handheld touchscreen concept while existing remote firmware work remains separate.

## Boat-side baseline
- Existing Flytec V080 hull and standard 550 brushed motors retained; 12 V motor target.
- Reported boat pack: 3S4P Li-ion, 2500 mAh cells, BMS 20 A (verify actual pack).
- Solenoids: FW0630 009 DC12V bait and FW0630 001 DC12V hook.
- Boat PCB envelope 77×79×14 mm; lid clearance about 30 mm; mounting holes, motor stall current and solenoid coil current not yet measured.
- Boat controller/GNSS: Heltec Wireless Tracker V1.2 candidate/selected direction; pin and antenna configuration must be verified.
- Motion sensor: GY-91 style MPU-9250 + BMP280 candidate; calibration and exact breakout verified before wiring.
- Radio: nRF24L01+ PA/LNA, 250 kbps initial test; 500 m range is unverified.
- Radio loss during autopilot: return-home preference, gated by GNSS/heading validity, safe home and watchdog.

## New handheld remote baseline
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, onboard antenna accepted.
- Screen central; joysticks upper left/right; 16 physical buttons in 2×4 banks; power button above screen; USB-C lower edge; grips.
- 1S4P pack, four 18650 parallel; cell model not selected.
- nRF24L01+ PA/LNA module with separate SMA antenna; 2.4 GHz coexistence with ESP32 Wi-Fi/Bluetooth must be tested.

## Remote power design
- Protected 1S4P pack input; charging circuit excluded.
- 5 V display/controller rail: TPS61088 candidate, component calculations incomplete.
- 3.3 V radio rail: TPS63070 adjustable buck-boost.
- TPS63070 calculated baseline: R1 470 kΩ VOUT→FB, R2 150 kΩ FB→GND, 3.307 V nominal; L 1.5 µH Coilcraft XFL4020-152ME candidate; input capacitors 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603; output 3×22 µF 16 V X6S 0805 + 10 µF 0603 (76 µF nominal); VAUX 100 nF.
- TPS63070 configuration proposal: FB2 GND; VSEL static LOW; PS/SYNC HIGH power-save; EN via 10 kΩ pull-up; optional PG 100 kΩ pull-up. Verify exact control-pin connections before capture.
- Radio design load 200 mA; estimated radio input current ≈245 mA at 3.0 V assuming 90% efficiency.
- Whole remote estimated ≈1.13 A at 3.0 V using display 450 mA/5 V and 85% boost efficiency plus radio 200 mA and 90% efficiency. Assumptions, not measurements.
- Main fuse 3 A time-delay provisional. Pack protection, wiring, connector, inrush, thermal and measured load must determine final value.

## Repository hardware workspace
`Remote/New_Handheld_Remote/` contains mechanical, CAD, PCB, wiring and documentation. Files remain preliminary. Native KiCad schematics, ERC, PCB layout, fabrication BOM and bench validation are not complete. Do not claim these deliverables exist until verified.

## Current engineering next steps
1. Verify TPS63070 package/pin numbers and control configuration against TI datasheet.
2. Verify exact capacitor parts' effective capacitance under DC bias and temperature.
3. Complete component-level TPS61088 5 V boost calculations.
4. Capture native schematic sheets and run ERC when KiCad tools are available.
5. Design battery protection/fuse/reverse-polarity and connectors around measured current.
6. PCB layout and bench test both rails at battery endpoints and load transients.

## Source links
- TI TPS63070: https://www.ti.com/lit/ds/symlink/tps63070.pdf
- TI TPS61088: https://www.ti.com/lit/ds/symlink/tps61088.pdf
