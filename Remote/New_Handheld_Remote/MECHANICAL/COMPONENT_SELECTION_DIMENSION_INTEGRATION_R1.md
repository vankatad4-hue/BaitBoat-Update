# Component Selection & Dimension Integration — R1

**Updated:** 2026-11-18  
**Status:** Candidate/component integration baseline; not a purchasing BOM. Verify exact revisions, datasheets, dimensions and footprints before fabrication.

## 1. Battery
- Architecture: protected 1S4P Li-ion pack, four matched 18650 cells in parallel.
- 2500 mAh per-cell target remains provisional; exact cell TBD.
- Pack nominal voltage 3.6–3.7 V, max charge 4.2 V; verify chemistry/pack documentation.
- Reserved volume: 104×82×24 mm; actual pack envelope and cell insulation/retention TBD.
- Use properly welded pack with compatible cells, protection, fuse, temperature sensing and mechanical restraint. Do not solder directly to cell terminals.
- Charging schematic excluded from current power-design work.

## 2. Display/controller
Selected direction: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 4.3-inch 800×480 capacitive display. Verify exact revision, board envelope, connectors, current and mechanical drawing. Onboard antenna accepted; no external ESP32 antenna path.

## 3. Joysticks
Candidate C&K/Littelfuse THB001P, two analog 2-axis 10 kΩ units with center push switch. Approximate envelope and shaft/PCB dimensions must be verified from exact mechanical drawing before CAD.

## 4. Radio
nRF24L01+ PA/LNA module with SMA antenna. Provisional envelope 45×20 mm; actual vendor module and connector orientation TBD. Dedicated low-noise 3.3 V rail, local 100 nF + 10–47 µF decoupling. 2.4 GHz coexistence with ESP32 Wi-Fi/Bluetooth must be tested.

## 5. Buttons
16 caps in 2×4 banks on both sides. Target cap 15×12 mm, pitch 21 mm; switch and cap stem remain TBD. Matte black caps, white functional legends; no decorative illumination.

## 6. Power architecture
- Battery input: protected 1S4P pack; charging circuit excluded.
- 5 V display rail: TPS61088 boost candidate. Output target 5.0 V; load estimate 450 mA is unverified. Converter sizing, inductor, compensation, current limit, thermal and transient design remain incomplete.
- 3.3 V radio rail: TPS63070 adjustable buck-boost.
  - R1 470 kΩ VOUT→FB, R2 150 kΩ FB→GND, 3.307 V calculated nominal.
  - L candidate Coilcraft XFL4020-152ME, 1.5 µH.
  - CIN: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603.
  - COUT: 3×22 µF 16 V X6S 0805 + 10 µF 0603, 76 µF nominal.
  - VAUX: 100 nF.
  - Configuration proposal FB2 GND, VSEL static LOW, PS/SYNC HIGH power-save, EN defined via 10 kΩ pull-up, optional PG 100 kΩ pull-up. Verify pin configuration against datasheet.
- Main fuse 3 A time-delay is provisional. Coordinate with pack BMS, wires, connectors, startup and fault current.

## 7. Placement
Display/controller centered. Joysticks upper left/right. Button banks on both sides. Battery reserve lower central. Radio near its antenna exit; ESP32 onboard antenna area kept clear. Power board near battery/USB-C but outside cell reserve. Switching nodes and converter loops kept away from RF feed.

## 8. Open items
1. Exact 18650 cell model and pack construction/protection.
2. Radio module exact model, dimensions, antenna and measured burst current.
3. Exact Waveshare board revision/connector/current.
4. TPS61088 component calculations and validation.
5. TPS63070 pin-state, symbol/footprint, capacitor bias and bench validation.
6. Power connector/fuse/reverse-polarity details and wire ratings.
7. Button switch/cap and joystick mechanical verification.
8. CAD stack-up and fit prototype.
9. Native schematics, ERC, PCB layout, BOM and bench tests.

