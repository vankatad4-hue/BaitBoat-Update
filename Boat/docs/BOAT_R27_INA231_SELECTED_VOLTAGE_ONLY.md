# R27 — Ardboard INA231 product 1491 selected for Boat battery voltage only

**Date:** 2026-11-22. **Selection: CONFIRMED FOR PROCUREMENT**; purchased=no; wired=no; measured=no; safety commissioned=NO.  
**Supersedes R20/R21/R25/R26 battery sensor selection only.** All other Boat hardware selections, R78 radio-loss and R79 low-battery logic remain unchanged.

## Selected physical module and evidence

- **Shop:** Ardboard Bulgaria, [INA231 module, product code 1491](https://www.ardboard.com/index.php?route=product/product&product_id=578). Listing showed €5.50 incl. VAT and 9 available on 2026-11-22. This is a shop listing, not a purchased physical unit or verified component authenticity.
- **IC:** Texas Instruments INA231 (A/B startup variant not specified by the Ardboard listing).
- **Voltage-only use:** 3S4P Li-ion pack nominal 11.1V, max 12.6V. TI documents BUS sense 0–28V, fixed bus-voltage register 0x02 with 1.25mV/LSB, VS 2.7–5.5V, 16 I²C address options. Power-off bus voltage is expressly allowed by TI because BUS/common-mode and VS are independent. [TI INA231 datasheet Rev D](https://www.ti.com/lit/ds/symlink/ina231.pdf), §§7 and 8.3.1.
- **Board has 0.1Ω shunt resistor R100.** At 20A it would dissipate P = 20² × 0.1 = **40W**, and INA231 ±81.92mV shunt range corresponds to only ±0.8192A through R100. **Do not connect main BMS/motor/solenoid current to the IN+ and IN− shunt pads.** This module is **not** selected as a current-monitoring or high-current safety device.
- Ardboard photo/other similar boards show a labeled **BUS** signal separate from the shunt pads. This is a provisional physical-interface interpretation; verify continuity on the purchased product 1491 PCB before connecting any battery voltage. No terminal cavity number, trace insulation rating or surge protection has been accepted from an online photograph.
- INA231 has **no manufacturer/die identification registers established by the TI Rev D register table**, unlike the previous INA226 R26 candidate. Do **not** use INA226 FE/FF identity-code checks on INA231. The on-board model identity and A/B variant must be verified from the exact supplied hardware/legitimate vendor evidence plus the documented configuration register behavior.
- TI also documents mode 0b110 (**bus-voltage continuous**) and 0b010 (bus-voltage triggered). The firmware must *explicitly configure* bus-only mode to handle both INA231A startup continuous-conversion and INA231B power-down-start behavior.

## Proposed low-current voltage-only connections (PINS NOT PHYSICALLY SIGNED)

| Function | Active logical net | Required gate |
|---|---|---|
| Module VDD/VS | Pololu D24V10F3 #2 regulated 3.3V sensor rail | Verify exact module labels, polarity, 3.3V pull-ups |
| Module GND | Boat low-current sensor ground | Do not use motor/solenoid high-current return wiring |
| SDA | ESP32-S3 GPIO8 (J1-12) | Verify R23 board header and I²C 3.3V high |
| SCL | ESP32-S3 GPIO7 (J1-7) | Same bus, module/power-off I²C behavior audited |
| BUS | Protected **LOW-CURRENT** switched 3S voltage sense branch | Check PCB label/continuity; sense wiring cannot carry traction current; suitable protection required |
| A1,A0 | Both GND -> theoretical 0x40 | Actual factory strap/address **NOT VERIFIED**; don't force unknown jumpers |
| ALE / ALERT | NOT CONNECTED | Not a physical cutoff or independent watchdog |
| IN+ and IN− shunt terminals | **NO MOTOR/BMS LOAD CONNECTION** | Existing R100 shunt bypassed entirely; physical unused-pin handling to be reviewed before applying voltage |

The board's distinct GND pads, BUS pad, A0/A1 configuration solder bridges and actual pin order must be checked from **the product 1491 module in hand**, not a visually similar shopping photograph. Full pack voltage at a protected BUS low-current sense connection is permitted by chip silicon, but the board and harness still need reverse-polarity, surge, corrosion, fuse and system ground review. No custom electronic PCB is required.

### Measurement-only software contract

- `Boat/source/ina231_voltage_contract_r27.h` — selected module=true, **physicalCommissioned=false**, 0x40 address only candidate, 0x4126 bus-continuous configuration for one sample at default 1.1ms VBUS conversion, and pure conversion of BUS register at 1.25mV/LSB. It deliberately provides **no manufacturer/die ID register check** and no Wire library.
- `Boat/tests/ina231_voltage_contract_r27_test.cpp` — host fixture tests voltage thresholds, bus-only mode, R79 critical-low handling, address mismatch, stale conversion, unverified PCB/shunt connections and permanently disabled safety read. Only synthetic data.
- `TrackerSensorPort::pollAdc` is intentionally locked against generating any ADS1115 voltage telemetry after R27 selection; the physical INA231 board is not commissioned or read from the Boat target yet. The battery application therefore remains fail-closed on missing valid measurements. **Do not claim live battery/RTH commissioning until real INA231 acquisition is implemented and measured.**
- Existing R26 INA226 alternative and R21 ADS1115 hardware guidance are historical, not approved purchase selections. R25 physical evidence checks remain NOT_RUN; do not use the old ADC calibration row to certify INA231.

### R27 acceptance required before activation

1. Purchase the **exact Ardboard 1491** unit and inspect front/back silkscreen, chip markings, module pull-ups, A0/A1 straps, continuity of the BUS pad to INA231 BUS pin and shunt bypass; record photos.
2. With no motor supply or EKM014 VH power connected, verify 3.3V VDD, sensor ground, 3.3V I²C, no bus conflict, boot-state and VS-off behavior safely.
3. Measure protected low-current BUS sense 9.0, 9.9, 10.2, 10.8, 11.4, 12.0, 12.6V against DMM; distinguish new-conversion-ready from cached register values. Unit remains uncommissioned until tests.
4. Verify raw-to-mV conversion; R79 immediate critical STOP 9.9V, critical 10.2V, low RTH 10.8V and failure on stale/invalid sample with mock actuator outputs and dummy loads.
5. Full physical MDD20A/EKM014 activation **still blocked** until independent rated STOP/watchdog/coil current-zero/pulse limiting and other R23/R25 gates are accepted.

**Official releases:** Boat VER014 / Remote VER028 frozen. No physical module energization, water trial or actual coil/motor test has taken place.

Sources: [Ardboard product 1491](https://www.ardboard.com/index.php?route=product/product&product_id=578), [TI INA231 Rev D datasheet](https://www.ti.com/lit/ds/symlink/ina231.pdf), [TI INA231EVM manufacturer guide](https://www.ti.com/lit/ug/sbou310/sbou310.pdf).

### R27 superseding harness matrix (2026-11-22)

The [R27 INA231 10-net battery-sense override](BOAT_R27_INA231_HARNESS_OVERRIDE.csv) replaces **only** the historical R23 `H03_ADS` nets. It leaves the rest of the 64-net R23 manufacturer pin map unchanged and marks every new INA231 connection **UNWIRED_PENDING** or **NO_WIRE**. The static [R27 selection checker](../tools/verify_r27_battery_selection.py) asserts exact Boat procurement model/quantity, GPIO7/8, 3.3V sensor rail, a protected low-current BUS sense, 0.1Ω shunt exclusion, permanent commissioning=false and 21 NOT_RUN evidence checks. These are software/design assertions, not inspected wire approvals.

## R28 supersession — triggered CVRF acquisition instead of R27 continuous fixture (2026-11-22)

R28 now integrates the selected INA231 into the dormant Boat sensor port via [real TwoWire register adapter, triggered BUS-only sampling and fault simulation](BOAT_R28_INA231_LOGIC_SIMULATION.md). The R27 `0x4126` continuous-mode fixture is **historical host evidence**, not the preferred production acquisition loop. R28 config `0x4122` requests a fresh BUS-only conversion and accepts it only with conversion-ready register evidence, no I²C error, plausible 3S voltage and actual timestamp. **Physical commissioning remains globally false, and not a single INA231 acceptance row is marked PASS.**
