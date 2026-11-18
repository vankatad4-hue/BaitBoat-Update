# Actual Component Selection & Dimension Integration — R1

**Status:** Engineering shortlist / CAD integration basis. Not a purchasing BOM. Verify exact revisions and footprints before fabrication.

## 1. Battery (fixed architecture)
- Configuration: 1S4P, four matched 18650 Li-ion cells in parallel.
- Target cell capacity: 2500 mAh each (nominal pack 10 Ah).
- Pack voltage: nominal 3.6–3.7 V; maximum charge 4.2 V.
- CAD reserve: 104 × 82 × 24 mm; cell pack target envelope about 74 × 65 × 20 mm, subject to cell model, insulation, spot-welded interconnect, pack protection and connector.
- Arrangement: four cells side-by-side, axes along the 65 mm length. Keep the pack removable and restrained; do not put standoffs or screws in the pack volume.
- Battery assembly: use a properly spot-welded pack, matched cells, insulation, pack protection, fuse strategy and temperature sensor. Do not solder directly to cell terminals.

## 2. Display + MCU candidate
**Candidate:** Waveshare ESP32-S3-Touch-LCD-4.3 (touch version)
- Display: 4.3 in, 800 × 480, capacitive touch.
- Board/display envelope: 106.1 × 67.8 mm; board depth reported around 8.8 mm.
- MCU: ESP32-S3, 16 MB Flash, 8 MB PSRAM.
- Use as the main UI/firmware development platform.
- Important mismatch: onboard antenna. It does not satisfy the previously requested ESP32 external antenna. Final production hardware may use a separate ESP32-S3 module with U.FL/IPEX and a display-only module, or a verified external-antenna variant.
- Do not connect the large 1S4P pack to the board's onboard battery charger as the final charging system. Power the board from a validated 5 V system rail and isolate/avoid unintended charge paths.

## 3. Joysticks (preferred candidate)
**C&K / Littelfuse THB001P**, two units
- Analog 2-axis, 10 kΩ, center-select push switch.
- Approximate footprint envelope: 17 × 21 mm; depth around 19.45 mm (check full drawing, shaft and PCB terminal clearance).
- Toggle life: 2 million cycles; center switch life: 1 million cycles.
- Mount as mirrored upper-left and upper-right controls.
- CAD reserve: each joystick well approx. 24 × 26 mm plus shaft travel/knob clearance; confirm from exact datasheet before hole design.

## 4. Radio
**Candidate retained:** nRF24L01+ PA/LNA module with SMA antenna.
- Boat command/telemetry radio.
- Reserve a provisional module envelope of 45 × 16 × 12 mm; actual vendor module varies and must be measured.
- Place near antenna exit, with local low-noise 3.3 V regulation and decoupling.
- Keep RF/antenna keep-out and distance from display/battery/copper.
- 2.4 GHz coexistence with ESP32 Wi-Fi/Bluetooth must be tested; independent antennas alone do not guarantee coexistence.

## 5. Buttons
- 16 button caps, 2 × 4 per side.
- Cap target: 15 × 12 mm; pitch target: 21 mm.
- Mechanical cap + PCB tactile switch or low-profile switch assembly TBD. Do not finalize cap stem, PCB footprint or hole until selected switch is physically measured.
- White printed legends, matte black caps; no decorative LED lighting.

## 6. Power architecture
- USB-C 5 V input with correct CC sink configuration, ESD and protection.
- Charger/power-path candidate: TI BQ25895 (single-cell charger with power-path, I2C, battery-temperature monitoring and boost capability).
- Initial charge-current design target: 2 A, only after exact cell charge ratings, pack construction, USB input budget and thermal design are confirmed.
- Display/controller rail: regulated 5 V sized for board load and transient margin.
- Radio rail: dedicated low-noise 3.3 V regulator sized for PA/LNA burst current, with local bulk + ceramic capacitors.
- Add battery fuel gauge or calibrated voltage/current measurement. Voltage-only state-of-charge estimate on a Li-ion pack is approximate.
- USB charging and firmware-programming paths must be designed to avoid backfeed and unsafe simultaneous-source conditions.

## 7. Mechanical placement basis
- Screen board centered; target board/display envelope 106.1 × 67.8 mm.
- Joysticks at upper left/right, clear of display bezel and button caps.
- Button clusters remain 2 × 4 on both sides of the display.
- Battery reserve is lower-central, target 104 × 82 × 24 mm.
- Power/charging PCB is lower side near USB-C port, but outside the cell reserve.
- Radio module placed toward one antenna exit. ESP32 RF path/antenna zone is opposite; antenna separation and layout require RF validation.
- Provisional enclosure depth remains 70 mm; actual stack-up must include front panel, joystick height/body, display PCB, rear cover, connectors, cables, cell pack, insulation and retention.

## 8. Unresolved before purchase/fabrication
1. Exact display/controller version and external-antenna strategy.
2. Exact 18650 cell manufacturer/model, dimensions, datasheet and current ratings.
3. Battery pack protection, fuse, temperature sensor and connector details.
4. Exact nRF PA/LNA module dimensions and SMA/U.FL connector orientation.
5. Charger schematic, USB-C CC handling, charge-current setting and thermal analysis.
6. 5 V boost and 3.3 V radio regulator component selection and load testing.
7. Exact button switch/cap assembly and PCB footprint.
8. Joystick CAD footprint/shaft clearance from manufacturer's full drawing.
9. Fit check against front/rear shell and printed prototype.

## Sources / references
- Waveshare ESP32-S3-Touch-LCD-4.3 official documentation and mechanical drawing.
- C&K THB001P datasheet and DigiKey listing.
- TI BQ25895 datasheet/product information.
- Dimensions shown in CAD are envelopes for layout only; source drawings and exact vendor variants must be checked at schematic/PCB stage.


## User-confirmed selections and power recommendations — R2 (2026-11-18)
- Waveshare ESP32-S3-Touch-LCD-4.3 selected; onboard antenna accepted, external ESP32 antenna requirement waived for this board.
- Battery remains 1S4P, four 18650 cells in parallel, side-by-side; exact cells TBD. Pack assumed to include protection/BMS; charging schematic intentionally out of current scope.
- nRF radio recommendation: conventional nRF24L01+ PA/LNA module with SMA antenna, reserve 45 × 20 mm; use a dedicated 3.3 V regulator capable of at least 200 mA transient budget, with local 100 nF + 10–47 µF decoupling. Exact module vendor/PCB footprint TBD.
- 5 V boost recommendation: TPS61088-based synchronous boost stage, 5.0 V output, target 2 A continuous capability and short peak headroom to 3 A only if the selected implementation is thermally/electrically validated. TI reference design documents 5 V/3 A capability and short-circuit/overload protection, but this is not automatic for every module or custom layout.
- Battery protection: rely on pack protection/BMS for cell-level overcharge, over-discharge, over-current and short-circuit protections; verify its ratings and wiring. Add a separate downstream main fuse for wire/connector protection.
- Main fuse: 3 A time-delay provisional; verify load profile, pack protection, wire and connector ratings before final selection.
- Thermal: power PCB near lower USB/power zone, not beneath grip or against battery; compact switching node, short high-current loops, thermal copper and vias under converter thermal pad per datasheet. Prototype hotspot target <70 °C under sustained test; measure under full brightness and RF/Wi-Fi activity. Not a component rating.
- Waveshare board is documented at 5 V / 450 mA (approx. 2.25 W). This load is the baseline for power budget; total remote load must be measured.
- These are engineering recommendations, not a fabrication-ready schematic or safety certification.

References:
- Waveshare board documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3
- nRF PA/LNA supply notes: https://shillehtek.com/blogs/shillehtek-product-manuals/nrf24l01-pa-lna-wireless-transceiver-module-antenna-1100m-range-manual
- nRF PA/LNA current and decoupling discussion: https://circuitpython-nrf24l01.readthedocs.io/en/1.3.0/greetings.html
- TI TPS61088 boost and reference design: https://www.ti.com/tool/PMP9779
- TI TPS61088 product: https://www.ti.com/product/TPS61088-Q1


## Power distribution PCB — R1 (2026-11-18)
- Added `PCB/POWER_DISTRIBUTION_DESIGN_R1.md` with input/fuse, 5 V boost, dedicated 3.3 V radio rail, placement/grounding, thermal, test points, connector groups and prototype test sequence.
- Confirmed display board onboard antenna accepted; no ESP32 antenna requirement.
- Recommended 5 V boost candidate: TPS61088; TI reference designs cover 5 V/3 A and low-EMI layout/short-circuit protection concepts. 3 A capability is not assumed for our PCB until designed and validated.
- Recommended radio 3.3 V buck-boost: TPS63070 (input 2 V–16 V; 3.3 V output design TBD), chosen because battery voltage crosses 3.3 V.
- Fuse recommendation revised to 3 A time-delay provisional due to 1S input current and peak loads. Final fuse must be verified against load profile, wiring/connectors and pack protection; previous 2 A idea superseded.
- Battery pack protection assumed onboard; verify pack BMS specification. Charging design remains out of scope.
