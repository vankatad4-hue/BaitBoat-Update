# Flytec V080 Boat — active hardware and firmware

**Current assembly-design handoff:** [R30A prototype assembly, logic-only test and safety gates](docs/BOAT_R30A_PROTOTYPE_ASSEMBLY_HANDOFF.md).  
**Effective wiring authority:** [R30A 73-net consolidated design](docs/BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv) with [24 open gates](docs/BOAT_R30A_READINESS_GATES.csv) and [17-item firmware integration inventory](docs/BOAT_R30A_SOFTWARE_INTEGRATION_MATRIX.csv).  
**Historical BOM baseline:** [BOAT_HARDWARE_SELECTION_R20.md](docs/BOAT_HARDWARE_SELECTION_R20.md), superseded by R27 INA231 and R29 shared 3S bus. **Not a high-current wiring permit.**  
**R21 ADC commissioning:** [BOAT_R21_BATTERY_ADC_COMMISSIONING.md](docs/BOAT_R21_BATTERY_ADC_COMMISSIONING.md) (not physically tested).  
**GPIO/header map:** [BOAT_GPIO_HEADER_MAP_R20.md](docs/BOAT_GPIO_HEADER_MAP_R20.md) (candidate, unsigned).  
**Current failsafe:** [R79 low-battery failsafe](docs/BOAT_R79_LOW_BATTERY_FAILSAFE.md)  
**Bench procedure:** [R79 battery/radio-loss dummy-load test](docs/BOAT_R79_LOW_BATTERY_BENCH_TEST.md)

R20 is the active procurement baseline. Older Heltec Wireless Tracker/SX1262 and central-carrier PCB documents are historical; see [the archive index](../docs/archive/2026-11-22/README.md).

## Selected Boat modules

- ESP32-S3-DevKitC-1-N32R16V main controller.
- HGLRC M100 PRO GNSS with integrated QMC5883L compass.
- GY-91 (MPU-9250 + BMP280).
- nRF24L01+ PA/LNA primary radio and EBYTE E22-900T22S backup radio.
- MCP23017 I/O expansion and **Ardboard INA231 product 1491**, voltage-only BUS sense (I²C 0x40 candidate, hardware uncommissioned).
- Cytron MDD20A dual-channel motor driver.
- EKM014 four-channel bait/hook release driver.
- Olimex DCDC6-16-TO5 main 5V, TRACO TSR 2-2450 EKM014 logic 5V, 2× Pololu D24V10F3 #2830 3.3V rails.
- 3S4P 18650 pack, 11.1V nominal, ~10Ah, 20A BMS.

Ready-made modules and removable harnesses only; no custom electronic PCB.

## R79 battery policy

- Green above 11.4V; yellow 10.8–11.4V; red at or below 10.8V.
- Red holds outbound mission movement immediately. After 3 seconds, an eligible active mission is diverted to validated Boat-NVS Home.
- Critical STOP at or below 10.2V for 1 second; immediate STOP at or below 9.9V.
- Missing/stale ADC data (>1s) fails closed.
- Low-battery RTH requires valid Home, GNSS/heading/geofence and local STOP/release interlocks. All release outputs remain OFF.
- If there is no autonomous mission, the Remote warns and manual return is the operator's responsibility. New missions are blocked at red.

Critical battery is displayed separately as `КРИТ. БАТ. — STOP`. The selected **R27/R28 INA231** needs its actual BUS pad/shunt bypass and DMM calibration verified before enabling the sensor profile. R21 ADS1115 divider is *historical and NOT to be fitted*. Estimated percentage is voltage-based, not coulomb-counted.

## Current status

Software changes are under CI validation. Selected modules remain unpurchased/uncommissioned. Target compile and host-test success do not replace ADC calibration, dummy-load testing or physical acceptance. Keep actual motors and solenoids disconnected until the bench procedure passes.

## R22 first sensor-only startup

The original [R22 ADS1115 bench procedure](docs/BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md) is **HISTORICAL, NOT the selected measurement path**. Use the new [R30A INA231-only default-locked bench sketch](hardware_test/INA231_Battery_Bench/INA231_Battery_Bench.ino) for future sensor diagnostics, after actual PCB and logic-only physical signoff. The sketch is GPIO-audit locked and only handles the premade ADS1115 module through candidate SDA8/SCL7, if physically verified. Check ADC VDD-versus-AIN0 power sequencing before connecting pack sense voltage. MDD20A motor power and EKM014 VH must remain physically disconnected.

## R23 assigned GPIO and wiring

The **historical R23 pin and signal baseline** is [R23 GPIO pinout/harnesses](docs/BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md) and its [64-net wiring matrix](docs/BOAT_R23_MODULE_CONNECTION_MATRIX.csv); use the **R30A effective 73-net schedule** for current assembly planning, since it retires seven ADS1115 nets and supersedes the old two motor/coil feed nets. It maps **18** Espressif ESP32-S3 DevKitC-1 J1/J3 GPIOs and MCP23017 GPA0-3 (EKM014 inputs), GPB0-1 (MDD20A DIR), power and E22 manufacturer pads. The selected two Pololu 3.3V regulators are now assigned to nRF24 and sensors/I²C separately; E22 VCC uses main 5V. Original R20 pin definitions remain candidates until physical validation; R23 does **not** sign GPIO audit flags or permit outputs. Motors/solenoids disconnected until independent hardware safety and dummy-load acceptance.
