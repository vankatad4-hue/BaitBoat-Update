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

## R30A observed full CI verification — 2026-11-22

- [**Boat host Actions #45042597229**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042597229), head `308fafa10a1d506e4ae42179e2c563608a51ae38`: **SUCCESS**. The R29 shared-3S-fault C++17 integration, R30A 73-effective-net/24-gate package validator and its seven hostile-mutation cases, and all downstream Boat host steps completed successfully. The successful R30A package checker confirms **17 software source references**, **64 unwired + 7 NC + 2 aliases**, and **81 physical NOT_RUN** lines; no real wire or safety component was tested.
- [**Boat ESP32-S3 target Actions #45042370480**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042370480), head `36b5a8afe7343a744584a62f1e3c2a9441c51c2d`: **SUCCESS**. Compiled diagnostic Boat firmware **408681 bytes program / 21160 bytes global RAM**; historical locked R22 ADS bench **302617 bytes / 18432 bytes**; new **selected locked R30A INA231 bench 302553 bytes / 18440 bytes**. Firmware binaries and build logs uploaded as GitHub Actions artifacts.
- Both runs are **source compilation and document/fault simulation evidence only**. The new INA231-only binary has `INA231_PHYSICAL_COMMISSIONED=false`, does not configure I2C pins in default build, and has no motor/release API. Hardware application still has no `BoatPhysicalDriver` attached.
- This verifies the R30A design handoff package **for unpowered/inspection-only prototype preparation and future supervised logic-only bench**, not for motor/solenoid high-current connection or water use. All 24 R30A physical gates OPEN, old 81 physical checks NOT_RUN. No new regulator, breaker/contactor, BMS or fuse SKU selected and no physical electrical acceptance performed. Boat VER014 / Remote VER028 unchanged.

## R30B application-level common 3S brownout fault protection

[Engineering report](docs/BOAT_R30B_SHARED_SUPPLY_FAULT_SIMULATION.md). The actual `BoatApplicationPort` now vetoes critical, stale, invalid or electrically untrusted INA231 readings before mission/actuator arbitration; the R79 radio-loss and low-battery tests remain in place. [R30B full host suite](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043490071) **PASS**. This code does not wire or certify an independent STOP, motor driver, coil limiter or current measurement. Boat diagnostic default still has no physical actuator HAL; single shared 3S4P power architecture and all 24 physical gate statuses unchanged.

## R30B final verified CI (2026-11-22)

- [Boat ESP32-S3 target #45043490121](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043490121), source commit `7f5e2e11222c2b859113b5a4b8e610a5cd8ac016`: **SUCCESS**. Full non-actuating `Flytec_Boat.ino` diagnostic, historical locked ADS1115 bench and selected locked R30A INA231 bench all compiled; upload-artifact steps successful. No live motor/solenoid outputs.
- [Boat host #45043597807](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043597807), documentation commit `eb461d416db71fb2ca1d258553cf4adc7747791d`: **SUCCESS**, including R29 shared-bus fault simulation, R30A 73-net/24-readiness-gate consistency and R30B brownout/stale voltage tests + source-guard assertions.
- No Boat executable source changed after `7f5e2e` before these reviews. Firmware physical commissioning remains **DISABLED**, R30A 24 hardware gates OPEN and R23/R25/R27 physical tests **81 NOT_RUN**. These successes establish reproducible **software/locked-target readiness**, not proof of a safely energizable Boat or a working RF/autopilot/actuator assembly.

