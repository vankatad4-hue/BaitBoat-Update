# Remote Power Distribution PCB — R1

**Status:** Architecture and candidate parts only; no schematic or PCB layout is fabrication-ready.

## Confirmed project constraints
- Main board/display: Waveshare ESP32-S3-Touch-LCD-4.3; onboard antenna accepted.
- Battery: 1S4P 18650 pack, four cells side-by-side. Cell model remains TBD.
- Battery pack includes protection/BMS (must verify ratings and pack wiring).
- Charging circuit is outside the current remote PCB design scope.
- Radio: nRF24L01+ PA/LNA with SMA antenna.

## Functional architecture
```
Protected 1S4P pack output
   |
   +-- Pack connector / polarity protection / main fuse
   |
   +-- BAT_PROT
        |
        +-- TPS61088 5.0 V boost --> 5V_MAIN --> Waveshare ESP32-S3 touchscreen
        |
        +-- TPS63070 3.3 V buck-boost --> 3V3_RADIO --> nRF24L01+ PA/LNA
        |
        +-- battery voltage sense (divider to MCU ADC, switched/high impedance recommended)
        +-- pack temperature sense input (pack NTC if available)
        +-- common ground plane with power return zoning and controlled current paths
```

## 1. Main input and fuse
- Connector: keyed/locking 2-pin battery connector; current rating must exceed measured current and selected fuse.
- Fuse: provisional **3 A time-delay** starting point for evaluation, not final. The earlier 2 A proposal is reconsidered because 5 V load current translates to higher battery current at low VBAT. Select fuse after load/peak measurements and confirm it protects connector/wire; never select solely by converter rating.
- Reverse-polarity protection: use a low-loss MOSFET ideal-diode arrangement rather than a series silicon diode if board loss/voltage headroom is important. Circuit and MOSFET selection TBD.
- Pack BMS is not a substitute for downstream fuse and wire protection. Verify pack protection current and short-circuit behavior.
- Test points: BAT+, BAT−/GND, VBAT after fuse, 5V, 3V3_RADIO, EN, PG (where available).

## 2. 5 V display/controller rail
**Candidate IC: TI TPS61088 synchronous boost**
- Input range compatible with single-cell Li-ion operation (verify selected design and operating limits from datasheet).
- Target output: 5.0 V.
- Target capacity: 2 A continuous design target; not a guaranteed capability without component, thermal, PCB and test validation.
- TI reference PMP9778 documents 5 V/3 A output and low-radiated-EMI layout practices; PMP9779 demonstrates added output short-circuit/overload disconnect circuit.
- Use a dedicated enable signal. Default-off or defined startup behavior; enable after input is stable.
- Place input/output capacitors, inductor, IC and switching-node copper according to TI reference/datasheet. Keep the high di/dt loop very small. Include EMI filter footprint only if bench measurements show it is needed.
- The controller board itself is documented at about 5 V / 450 mA. Verify actual full-brightness, Wi-Fi/Bluetooth and startup current.

## 3. Radio 3.3 V rail
**Candidate IC: TI TPS63070 synchronous buck-boost**
- Input range 2 V–16 V; output programmable in its supported range.
- 3.3 V rail target.
- Datasheet product page specifies up to 2 A output in boost mode under stated conditions (VIN=4 V, VOUT=5 V). Actual 3.3 V current and thermal performance must be calculated from design conditions.
- Dedicated radio rail; don't feed PA/LNA from the ESP32 board's marginal 3.3 V rail.
- nRF PA/LNA current peaks depend on module revision and RF output setting; design budget ≥200 mA transient and validate at the module.
- Add local 100 nF ceramic plus 10–47 µF low-ESR bulk near module supply. Check PA/LNA module vendor guidance and RF behavior.
- Route radio supply/return independently back to the regulator/output-capacitor region; avoid sharing narrow high-current paths with the ESP32.

## 4. Battery sense / pack NTC
- Battery-voltage divider sized to keep ADC input within limits, using high values to reduce standby drain; include capacitor and filtering. Optional switched divider for low-power standby.
- NTC input connector or pad pair for pack temperature. If pack has no NTC, keep input unpopulated and define firmware fallback.
- Fuel-gauge IC is not selected in R1; ADC voltage alone is not a precise state-of-charge measurement for Li-ion.

## 5. Ground, placement and zones
- Use a continuous ground reference plane; don't split the ground plane under high-speed/RF signals. Keep noisy power loops physically compact and route high-current returns so they do not pass through sensitive radio/analog return paths.
- Board zones: battery/fuse at connector edge; TPS61088 + inductor + input/output caps in high-current power corner; TPS63070 + radio rail caps in RF corner; low-noise ADC/NTC circuitry away from switch nodes.
- Keep both switch nodes copper compact and away from RF antenna feed, display flex, joystick ADC traces and button matrix.
- Place bulk capacitors near each load connector. Consider ferrite/LC filter footprints on the radio supply only if measurement indicates noise issue.
- Four-layer PCB recommended for first layout: top components/short power loops, inner ground plane, inner power/signals, bottom routing/thermal copper. A two-layer layout is possible but likely harder to control EMI and return paths.
- Keep antenna keep-out and nRF module/connector location aligned to the enclosure antenna exit. Do not route copper or place battery/metalwork under the antenna region according to module/antenna guidance.

## 6. Thermal requirements
- TPS61088 thermal copper and vias must follow datasheet/reference layout; don't place directly under grip or adjacent to battery.
- TPS63070 thermal pad/via strategy according to package layout guidance.
- Prototype hotspot target <70 °C at a defined ambient during sustained worst-case load is a project test target, not an IC rating.
- Test with display maximum brightness, MCU CPU/radio activity, nRF transmitting, Wi-Fi/Bluetooth activity, and enclosed shell.
- Record input voltage/current, 5 V load, 3.3 V radio load, converter temperature and enclosure temperature. Re-test at low battery voltage because boost input current rises.

## 7. Proposed connector set (not yet pin-numbered)
- J1 BAT_IN: protected pack positive and ground.
- J2 DISPLAY_5V: 5 V, ground, enable/PG optional; board's own interface connectors remain separate.
- J3 RADIO_3V3: 3.3 V, ground, SPI signals, CE, CSN, IRQ.
- J4 NTC: pack temperature input/ground.
- TP1 BAT, TP2 GND, TP3 VBAT_FUSED, TP4 5V, TP5 3V3_RADIO, TP6 EN, TP7 PG.

Pin numbers, connector part numbers, wire gauges, fuse holder footprint and current ratings are deferred until selected harness/components.

## 8. Candidate parts summary
| Function | Candidate | Status |
|---|---|---|
| 5 V boost | TI TPS61088 | Recommended candidate; custom design needed |
| 3.3 V radio buck-boost | TI TPS63070 | Recommended candidate; custom design needed |
| nRF radio | nRF24L01+ PA/LNA module with SMA | Module selection TBD |
| Main fuse | 3 A time-delay provisional | Verify current profile/wiring |
| Reverse polarity | MOSFET ideal-diode circuit | Schematic TBD |
| Battery protection | Pack-integrated BMS/protection | Verify pack specification |
| Battery sense | ADC divider | Design TBD |
| Pack NTC | Optional/if pack includes it | TBD |

## 9. Build/test sequence
1. Create detailed schematic from this architecture and use the exact converter datasheet/reference design values.
2. Review converter loop/layout and current paths.
3. Verify fuse against measured load and wire/connectors.
4. Prototype power stages before connecting expensive display/radio.
5. Load-test 5 V rail with electronic load; verify startup, current limit, ripple and thermal rise.
6. Load-test 3.3 V rail and radio transmit bursts; verify no brownouts/reset or RF instability.
7. Run both rails and Wi-Fi/Bluetooth/radio concurrently; measure noise and temperatures.
8. Enclosed thermal test at low battery voltage.

## References
- TI TPS61088 product/datasheet: https://www.ti.com/product/TPS61088-Q1
- TI PMP9778 low-EMI boost reference design: https://www.ti.com/tool/PMP9778
- TI PMP9779 boost short-circuit/overload protection reference: https://www.ti.com/tool/PMP9779
- TI TPS63070 product/datasheet: https://www.ti.com/product/TPS63070
- nRF24 PA/LNA module common issues/power discussion: https://nrf24.github.io/RF24/md_COMMON__ISSUES.html

## TPS63070 radio rail — component calculation update R1 (2026-11-18)
- Proposed feedback: R1=470 kΩ (VOUT→FB), R2=150 kΩ (FB→GND), calculated nominal 3.307 V; divider current 5.33 µA. 1% candidates.
- Inductor candidate: Coilcraft XFL4020-152ME, 1.5 µH, manufacturer-rated current criteria to be checked at BOM selection.
- Input capacitance reference set: 2×10 µF 25 V X7S 0805 + 10 µF 25 V X5R 0603 local; VAUX 100 nF.
- Proposed output set: 4×22 µF 0805 + 10 µF 0603 local (98 µF nominal). Check DC-bias derating/effective capacitance against TI recommendation before lock.
- Calculated radio battery current at 200 mA output, 3.0 V battery, assumed 90% efficiency: ≈245 mA.
- Preliminary total remote battery draw ≈1.13 A at 3.0 V with 450 mA display load at 5 V, 85% boost efficiency, plus radio 200 mA at 3.3 V, 90% efficiency. This is a budget estimate only; confirm display current and efficiencies on bench.
- Full calculation record in TPS63070_COMPONENT_CALCULATIONS_REVIEW_R1.md. Mode-pin/enable design, exact capacitors, layout, stability and thermal remain unresolved.

## Datasheet correction and configuration review update (2026-11-18)
- TPS63070 adjustable single-rail configuration: FB2 grounded; VSEL held LOW (no dynamic voltage scaling); PS/SYNC HIGH to enable power-save PFM/PWM operation; EN pulled high through a 10 kΩ resistor to BAT_PROT for always-enabled rail. These are the design choices to implement, pending symbol/net review. If MCU shutdown control is added, it must safely override EN.
- PG is open-drain; 100 kΩ pull-up to 3V3_RADIO is a candidate for MCU status monitoring, not mandatory. Verify the intended MCU input and PG behavior before capture.
- Capacitor-bank proposal corrected: follow TI's adjustable 3.3 V typical application BOM (3×22 µF X6S 0805 + 10 µF 0603, nominal 76 µF), rather than the earlier 98 µF proposal. TI's filter table supports 1.5 µH with nominal 47/68/100 µF; the 76 µF typical-app bank is within the recommended combination table. Effective capacitance remains to be confirmed using manufacturer DC-bias/temperature data. Datasheet permits effective output capacitance 15–470 µF for nominal 1.5 µH.
- Selected 22 µF candidate: Murata GRM21BC81C226ME44L, 16 V X6S, 0805, ±20%, -55°C to +105°C. Exact DC-bias capacitance at 3.3 V is not stated in the basic part specification and must be verified from Murata's bias curves/approval data. Three units are specified by TI's reference BOM.
- The prior 98 µF nominal output proposal is superseded. See the calculation review and pin/net map for corrected baseline.
Sources: TI TPS63070 Rev. B datasheet, §§8.5.1, 9.2.2.1–9.2.2.3, Tables 2–4; Murata GRM21BC81C226ME44L reference sheet.
