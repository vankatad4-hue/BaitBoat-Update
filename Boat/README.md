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

## R30C single-bus STOP and physical feedback

[The R30C feedback contract](docs/BOAT_R30C_ONE_BUS_SAFETY_FEEDBACK_CONTRACT.md) documents how a future **single shared 3S actuator bus** could be supervised: one qualified common DC-rated disconnection feedback, hardwired STOP, independent watchdog, independently measured motor/coil current zero and externally bounded one-hot coil pulses. No module is chosen, no contacts exist and the code flag `COMMON_BUS_HARDWARE_COMMISSIONED=false` prevents authorization. Historic dual-contactor tests do **not** count as evidence for the owner's one-pack design. `Flytec_Boat.ino` still has no commissioned actuator driver. Both releases frozen.

## R30C verified CI — source and documentation (2026-11-22)

- [**Boat host Actions #45044409605**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044409605), commit `1d7fef011214277941dbfa6804dbc193fde98fe9`: **SUCCESS**. The dedicated R30C C++17 synthetic STOP/watchdog/common-disconnect/current-zero fault suite printed **"R30C common-bus STOP/watchdog/contact/current-zero model PASS; physical output permanently locked"**; accompanying static source/physical-gate check printed **PASS**.
- [**Latest R30C documentation-triggered Boat host Actions #45044485081**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044485081), commit `e7d7e1588e89ecc9463ce3f87fd6f638406ee0ea`: **SUCCESS**, including R29, R30A, R30B and R30C portable host checks.
- [**Boat ESP32-S3 target Actions #45044281786**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044281786), executable-source commit `c49f83cf8debcf7ce40776aed9e1338e65598d5b`: **SUCCESS**, both non-actuating main Boat diagnostic and default-locked INA231 bench compiled; all artifact uploads completed. Later R30C files updated tests/tools/docs/workflow, not executable Boat source.
- These are **software-only results**, not physical DC cutoff or feedback. `COMMON_BUS_HARDWARE_COMMISSIONED=false`, INA231 physical commissioning false, legacy dual-contactor driver unattached, shared 3S wiring uncommissioned, all 24 R30A physical gates OPEN and 81 older physical checks NOT_RUN. No hardware chosen, no motor/coil load energized or safety claims made; Boat VER014 / Remote VER028 frozen.

## R30D read-only common-bus observer

[Read-only feedback specification and simulated faults](docs/BOAT_R30D_READONLY_FEEDBACK_SIMULATION.md). The Boat compiles a transport-neutral input-only interface for seven potential local safety feedback signals, rejecting stale, duplicate-source and missing observations without generating motor/solenoid commands. Both source and hardware commissioning flags remain **false**, and no reader or load disconnect was installed. One common 3S4P battery supplies the planned MDD20A VM and EKM014 VH parallel loads; not separately switched. No physical safety acceptance or version promotion.

## R30D verified results — 2026-11-22

- **ESP32-S3 target CI** [run 45044906755](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044906755) completed **SUCCESS** on Boat firmware-source commit `b8340cb74ebd78261bca474b9285c7abea1219ca`. It compiled the selected non-actuating diagnostic application including the new R30D read-only header/static commissioning guards, the historical locked ADS1115 bench, and selected locked INA231 bench. No reader constructed or actuator GPIO configured.
- **Boat host CI** [run 45045107306](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45045107306) completed **SUCCESS** on source-auditor correction `829bede975471a529103a41222e44a7da846f422`. The R30D simulated seven-source input test **PASS**, source-level no-write/unsigned gate audit **PASS**, and all existing Boat portable host tests **PASS**.
- Earlier intermediate runs [45044974018](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45044974018) and [45045063567](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45045063567) failed **only** the static Python scanner: it mistook an explanatory comment `// Deliberately no write()` for an actual write API. The C++ fault test itself passed; the scanner now strips `//` comments before inspecting the concrete `ReadOnlySource` method declarations.
- **No physical signoff:** `ACTUAL_READER_COMMISSIONED=false`, `COMMON_BUS_HARDWARE_COMMISSIONED=false`, `INA231_PHYSICAL_COMMISSIONED=false`, and shared-bus physical acceptance remain false. Both real motor and solenoid power paths are unwired and the actual Boat actuator HAL remains unattached. R30A 24 OPEN gates and prior 81 NOT_RUN checks unchanged. No release changes (Boat VER014, Remote VER028).

## R30E — local fault diagnosis without actuator power

[Boat R30E report](docs/BOAT_R30E_LOCAL_FAULT_TELEMETRY.md). Non-actuating `Flytec_Boat.ino` now reports **NO_READER / UNVERIFIED / outputs=LOCKED** in a two-second USB Serial diagnostic, plus a versioned local 8-byte hex record. Actual R30D hardware safety inputs are unselected, so the code cannot claim measured STOP/watchdog/current-zero evidence. No Remote RF protocol or hardware module changed. Physical commissioning flags remain false and Boat VER014 / Remote VER028 unchanged.

## R30E CI — independently observed PASS (2026-11-22)

- [Boat host run **45047481082**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45047481082), code/workflow commit `bed9ce336beb6bf32f4ccf84ab6e1f463de90fad`: **SUCCESS**. The new C++17 `boat_r30e_fault_status_test.cpp` passed (all fault-code priorities, local 8-byte envelope, event tracking and permanently locked physical driver). The static `verify_r30e_fault_status.py` audit and all other Boat regression steps passed.
- [Boat ESP32-S3 target run **45047413922**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45047413922), latest executable-source commit `7f21a574a7bb21c3b8b277022d3915ecff104697`: **SUCCESS**. The non-actuating `Flytec_Boat.ino` including R30E local Serial telemetry compiled; the locked INA231 and historical locked ADS1115 bench sketches compiled; artifact steps completed.
- [Post-documentation Boat host run **45047573473**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45047573473), documentation commit `d129e71b4c90221671e15c1a0b2d8e0317693308`: **SUCCESS**. Confirms documentation-triggered host suite still passed. No Boat executable source changed after `7f21a574`.
- R30E's actual on-target expected diagnostic line is **`NO_READER / UNVERIFIED / outputs=LOCKED`** because the physical reader was not selected or attached. The tests do **NOT** certify real safety feedback or live Remote telemetry. The D5 diagnostic envelope remains USB local-only and not an authenticated radio command/payload. The 24 physical design gates remain OPEN, 81 older physical checks NOT_RUN; single common 3S VM/VH architecture and Boat VER014 / Remote VER028 unchanged.

## R30F — optional authenticated unverified safety telemetry to Remote

[Boat↔Remote R30F wire protocol](docs/BOAT_R30F_DIAGNOSTIC_WIRE_PROTOCOL.md): an optional, HMAC/session-authenticated primary-nRF telemetry subtype 8 for `NO_READER`, STOP/watchdog/disconnect/current-zero faults and physical commissioning **LOCKED**. The Boat keeps USB R30E diagnostics and adds the RF packet only when the secure primary session is active. No physical R30D input reader exists, so it **cannot** report a genuinely healthy contact. The Remote displays diagnostics on a separate Bulgarian line and never treats them as STOP ACK, battery charge or control link freshness. Legacy subtypes 1–7 unchanged, no physically separate motor/coil rails, all 24 physical gates still OPEN.
