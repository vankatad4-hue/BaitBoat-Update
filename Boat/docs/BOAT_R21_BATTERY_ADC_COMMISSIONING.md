# R21 — Boat battery ADC low-voltage commissioning

**Status:** electrical design and software tests prepared; **NOT physically commissioned**. Date: 2026-11-22.

This document **supersedes only the R20/R79 proposed 100kΩ / 33kΩ divider**. All other R20 selected modules, R79 low-battery thresholds and the unsigned R20 GPIO/header map remain current.

## Correction and reason

At a full 3S Li-ion pack voltage (12.6V), the old 100kΩ/33kΩ divider would produce 3.126V at ADS1115 AIN0. The production `carrierBatteryMv()` converter rejects inputs above 3.000V; a healthy full pack could have appeared **invalid**. It also left little transient margin on a 3.3V-powered ADC.

**New R21 proposed connection (still uncommissioned)**

| Element | Connection / value |
|---|---|
| ADC | Premade ADS1115 module, 3.3V VDD, I²C 0x48 |
| R_TOP | 150kΩ ±1%, protected/master-switched 3S supply to ADS1115 AIN0 |
| R_BOTTOM | 33kΩ ±1%, AIN0 to sensor ground |
| Filter | 100nF ceramic, AIN0 to sensor ground near ADC |
| Nominal ratio | (150+33)/33 = **5.5454545:1** |
| ADS1115 mode | Single-shot AIN0-GND, ±4.096V PGA, 128SPS, 0.125mV/count |
| Profile | `c.battery={false,BOAT_BATTERY_DIVIDER_RATIO,1.0,0.0,9000,13000}` |

`commissioned=false` and the ESP32-S3 `pinAuditSigned=false` flag **must remain set until actual hardware acceptance**.

### Proposed wiring — sensor-only bench

```text
3S pack through BMS
  |
  +-- MASTER SWITCH / protected distribution
       |                              |
       |                              +-- D24V10F3 #2 --> 3.3V --> ADS1115 VDD
       +-- switched voltage sense --> R_TOP 150k -->+-- AIN0
                                                    |
                                                    +-- 33k --> SENSOR GND
                                                    |
                                                    +-- 100nF --> SENSOR GND

ADS1115 SDA --> ESP32-S3 GPIO8 [R20 candidate; unsigned]
ADS1115 SCL --> ESP32-S3 GPIO7 [R20 candidate; unsigned]
ADS1115 ADDR --> GND (0x48)
ADS1115 ALERT/RDY --> not connected
ADS1115 GND --> sensor GND, not high-current motor return

MDD20A motor power and EKM014 VH: **PHYSICALLY DISCONNECTED**.
```

**Absolute input safety:** TI specifies analog input within GND−0.3V to VDD+0.3V. The ±4.096V gain setting does **not** permit 4.096V at AIN0 if ADS1115 VDD is only 3.3V. A 150kΩ top resistor is **not** proof against back-powering the unpowered ADC. Measure AIN0 and VDD **simultaneously during startup, shutdown and switching**; if that input limit is exceeded, keep the sensor disconnected until a manufacturer-compliant sequencing/protection solution is selected.

### Nominal voltages (not measurements)

| 3S pack | ADS1115 AIN0 |
|---:|---:|
| 9.0V | 1.623V |
| 9.9V | 1.785V |
| 10.2V | 1.839V |
| 10.8V (RTH threshold) | 1.948V |
| 11.4V (yellow boundary) | 2.056V |
| 12.0V | 2.164V |
| 12.6V full | **2.272V** |
| 13.0V test upper | 2.344V |
| 16.0V transient calculation only | 2.885V |

At 16.0V with top −1% and bottom +1%, AIN0 would be approximately 2.933V. This does not validate any real transient, ADC rail sag or off-state exposure. The actual pack should never be continuously operated at 16V.

## Commissioning stages

1. **Unpowered review:** verify exact Espressif DevKitC-1 N32R16V revision, ADC identification/address straps, 150kΩ and 33kΩ resistance, filter, switch, battery polarity, cable grounds, and continuity. No motor or solenoid loads.
2. **3.3V-only low-voltage rail:** current-limit the supply. Verify regulator voltage/ripple, ADS1115 3.3V supply and I²C pull-up voltages. Confirm DevKitC USB 5V is not backfeeding external supplies.
3. **I²C scan:** verify address 0x48 on GPIO7/8 **only after** those pins have been matched to the purchased board schematic. If other I²C modules are installed, check unique addresses and common ground.
4. **Simulated pack voltage:** increase the protected/switched source through 9.0, 9.9, 10.2, 10.8, 11.4, 12.0, 12.6V. Measure both AIN0 and ADS1115 VDD with a DMM, and scope their rise/fall sequencing.
5. **Calibrate ADC:** record actual battery voltage and AIN0 voltage, signed ADS1115 raw code, firmware calculated mV and gain/offset residuals at all setpoints in the [blank R21 worksheet](BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv). Do not use nominal numbers as evidence.
6. **Inject non-damaging faults:** disconnect ADC/I²C, simulate a zero/invalid ADC code, interrupt the sense wire. Every invalid **measurement** must trigger **immediate STOP**. A valid reading that merely ages beyond 1000ms must also STOP.
7. **Low-battery state machine with dummy outputs:** verify 10.8V red/RTH timing, release deenergization + independent feedback, 10.2V confirmed critical STOP, 9.9V immediate critical STOP and dual-radio loss.
8. **Release gate:** verify independent physical STOP/watchdog and release-current-zero interlocks. If those modules/signals have not been selected and commissioned, the real motor/solenoid driver must remain disconnected and the application must stay in diagnostic mode.

## Code and evidence

- `Boat/source/boat_battery_divider.h` — single nominal ratio source.
- `Boat/source/tracker_sensor_profile.example.h` — unsigned example referencing that ratio.
- `Boat/tests/tracker_sensors_test.cpp` — full-charge ADC regression.
- `Boat/source/boat_battery_policy.h` — immediate invalid-reading fail closed; 1-second age limit as separate additional protection.
- `Boat/tests/boat_battery_policy_test.cpp` — invalid/critical regressions.
- `Boat/docs/BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv` — **blank** physical-evidence form.

Sources: [TI ADS1115](https://www.ti.com/product/ADS1115), [Espressif DevKitC-1 v1.1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html), [Electrokit EKM014](https://docs.electrokit.com/en/modules/EKM014/).

No physical DMM/scope reading, dummy-load test or water test has been performed. **Boat VER014 and Remote VER028 remain frozen.**

## R22 companion diagnostics (subsequent software work)

The [R22 isolated ADS1115 serial bench sketch](BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md) can produce actual raw ADC codes **only after the physical GPIO and voltage-sequencing checks**. Its default build performs no I²C initialization; it is not integrated into the Boat's output-authorization path. Store the physical DMM and scope measurements in the R21 worksheet; a plausible uncalibrated serial value is never sufficient to commission the sensor or propulsion.

## R25 clarification — do not energize the sense line before sequencing validation

**Safety clarification (2026-11-22):** The 150kΩ/33kΩ divider values solve the previous full-charge software scaling conflict only. They **do not** prove protection against input back-powering while ADS1115 VDD is absent or rising/falling. At 12.6V, a disconnected/unclamped divider predicts ~2.272V AIN0; the ADS1115 absolute maximum is VDD+0.3V. The original R21 ramp/check procedure must **not** be interpreted as permission to apply a knowingly over-limit source to an unpowered IC. Keep the 3S pack sense feed disconnected until a compliant power sequencing/protection design and safe verification method have been approved for the exact premade ADS1115 carrier. See [R25 preflight](BOAT_R25_POWER_LEVEL_PREFLIGHT.md) and [unsigned evidence worksheet](BOAT_R25_POWER_LEVEL_EVIDENCE.csv). No physical calibration has been performed.

## R27 — R21 battery divider retired (2026-11-22)

**Historical only:** R27 changes the selected Boat sensor from ADS1115 to **Ardboard INA231 product 1491** (BUS-voltage-only). Do not build the R21 150kΩ/33kΩ divider or apply R21 ADC AIN0 test procedure to the new INA231. See [selected INA231 wiring, R100 shunt exclusion, and 21 physical gates](BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md). The original ADS1115 full-charge software calibration regression remains available as archived historical code evidence; it is not runtime authorization.
