# Remote R6.4 — Component shortlist and selection gates

**Status:** candidate shortlist only; no parts purchased and no footprints frozen.

## Joysticks — candidate KY-023 / HW-504 class

Two matching dual-axis analog thumbsticks are required. Candidate family: KY-023-compatible module with two 10 kΩ potentiometers, X/Y analog outputs and a push switch. Power both from 3.3 V; this keeps the analog outputs within the ESP32 ADC range. Calibrate center at boot (or store calibration), apply deadband, and map each axis independently. The integrated switch may be used as an optional input; it is not a substitute for the physical OK button.

**Mechanical gate:** supplier variants differ. Published sizes include approximately 34×39×26 mm and 40×26×32 mm. The existing 36×36 placeholder is therefore not reliable. Reserve at least 40×40 mm PCB envelope plus stick travel, mounting screw access and cable clearance; verify the exact purchased part before revising CAD. Avoid ordering based on “KY-023” alone.

References:
- https://www.espboards.dev/sensors/ky-023/
- https://cdn-reichelt.de/documents/datenblatt/A300/COM-KY023-DATASHEET.pdf

## Primary radio — nRF24L01+ PA/LNA candidate

Candidate common SMA PA/LNA module (not yet selected by supplier): approximately 41–46×15–17 mm PCB envelope; supply 3.0–3.6 V; peak TX current reported around 90–140 mA depending on front-end and board variant; RX roughly 20–45 mA. These variant-dependent figures are intentionally treated as a design range, not guaranteed limits.

**Electrical requirements for the carrier:**
- dedicated regulated 3.3 V rail, target capacity at least 300 mA for the primary radio branch pending measured board variant and transient tests;
- local decoupling at radio connector: preliminary 100 nF ceramic + 10–47 µF low-ESR capacitor, short return loop;
- SPI plus CE/CSN; IRQ optional;
- use low data rate (250 kbps) for range robustness; test at reduced power first and increase only as necessary;
- antenna connector must not be obstructed by enclosure or battery; keep RF feed short.

References:
- https://handsontec.com/dataspecs/module/NRF24L01%2BPA.pdf
- https://www.crcibernetica.com/2-4g-wireless-nrf24l01-module-with-pa-and-lna/

## Backup radio — Ebyte E22-900T22S (868 MHz variant)

Keep as the safety/backup link. Current provisional zone is upper-right. Confirm exact EU 868 MHz variant, antenna, UART logic and permitted RF settings. The module is 26×16 mm, but the footprint zone must include antenna feed, connector and keep-out.

Manufacturer manual:
- https://www.ebyte.com/Uploadfiles/Files/2024-4-12/2024412165254103.pdf

## Battery and charging — decision not frozen

Pack concept remains 1S4P 18650, matched cells, physically retained and insulated. Do not treat a generic BMS board as a confirmed selection. Requirements:
- 1S cell protection against over-charge, over-discharge, short circuit and over-current;
- pack-level fuse close to pack output;
- cell temperature sensing during charge (NTC) and thermal separation from boost converter;
- charge-current limit selected from cell datasheets and pack wiring;
- one charging architecture only after verifying Waveshare board's battery/charger/power-path schematic and charge current;
- no assumption that charging while operating is safe/supported.

## DC/DC converter selection gates

The display is a known 5 V load (manufacturer-listed 450 mA). E22 can peak at 150 mA TX at 5 V. The primary radio is a separate 3.3 V burst load. Choose converters only after confirming:
- minimum pack voltage under load;
- display-board battery input and onboard boost/charger behavior;
- radio transmit duty cycle and simultaneous operation;
- efficiency and thermal rise in closed enclosure;
- transient/brownout test at maximum brightness and both radios active.

**Preliminary branches:** display board's own documented power path (pending verification); 5 V branch for E22; separate 3.3 V low-noise branch for nRF PA/LNA and GPIO. The branches must not be tied together at their outputs.

## Required next bench checks
1. Measure exact joystick board + stick sweep envelope.
2. Measure nRF PA/LNA current in TX/RX at 3.3 V on the exact module.
3. Verify Waveshare board charge/power-path circuit from its official schematic.
4. Confirm E22 variant and antenna.
5. Build a temporary power bench, measure droop and converter temperatures.
6. Only then freeze connector pinouts, regulator part numbers, fuse, and PCB footprint.

