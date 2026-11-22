# Flytec V080 Boat — R30A prototype assembly and physical-test handoff

**Authority:** 2026-11-22 · branch `hardware-ready-dual-radio-r1`.  
**Outcome:** **documented design and inspection-only/unpowered assembly plan; NOT yet approved for live actuator-power wiring or functional water trials.**
This is the active handoff package for the selected ESP32-S3 DevKitC-1 architecture. Older Heltec Tracker, PCB-carrier and screenless-V2 R29/R30/R31 documents are *historical alternatives* and MUST NOT override this package. Sonar remains deliberately deferred.

## Assembly authority, without contradictions

1. **Parts and procurement:** `docs/PROJECT_PURCHASE_REGISTER_R20.csv`, applying R27 INA231 replacement and R29 common-3S-power decision. Item status `NOT_PURCHASED`, candidate-only exact SKUs and open safety parts are not signed as received. Check availability anew before ordering.
2. **One effective connection list:** [BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv](BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv) — **73 logical rows: 64 UNWIRED_PENDING, 7 NO_WIRE, 2 REFERENCE_ONLY**. This merged plan retains R23 GPIO/signal connections but excludes **all 7 retired ADS1115/divider nets** and the **2 retired individually gated motor/coil power nets**; adds R27 INA231 and R29 one-common-3S-bus rows. Original R23, R27 and R29 CSVs are kept unedited as revision evidence.
3. **Board pin authority:** [R23 DevKitC-1 J1/J3 crosswalk](BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md). 18 proposed unique GPIOs; only manufacturer-header-supported logical pin **candidates**, not an inspected board/header release. nRF24 CE10/CSN9/SPI 11–13/IRQ21; E22 pins 6/14–18; UART M100 GPIO4/5; I2C SCL7/SDA8; PWM39/40; MCP23017 GPB0/1 MDD DIR, GPA0–3 EKM inputs.
4. **Sensor:** [R27 INA231 selection](BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md) and [R28 acquisition/fault logic](BOAT_R28_INA231_LOGIC_SIMULATION.md). INA231 BUS is **low-current voltage sensing only**; on-board R100 0.1-ohm shunt **NEVER in 20 A battery/motor/coil current path**. Actual BUS PCB trace and voltage input protection unverified.
5. **Actuator supply:** [R29 shared distribution](BOAT_R29_SHARED_BUS_FAULT_SIMULATION.md). Exactly **one** 3S4P pack common bus feeds MDD20A **VM** and EKM014 **VH** as parallel taps; NO motor-only or coil-only battery/independently switched domains. EKM014 **VL** receives separately *regulated* 5 V via TSR 2-2450. Other regulated 5 V/3.3 V logic rails all derive from the same battery. Do not confuse voltage rails with separate power sources.
6. **Actual firmware:** `Boat/source/firmware/Flytec_Boat.ino` is a **non-actuating diagnostic**. It does NOT attach `BoatPhysicalDriver`; default radio GPIO audits, INA231 physical commissioning and motor GPIO profiles remain OFF. Portable motor, release, mission, radio and battery modules exist but cannot be called a fully commissioned integrated vehicle controller.
7. **Physical blockers and signoff sheet:** [BOAT_R30A_READINESS_GATES.csv](BOAT_R30A_READINESS_GATES.csv), plus R23 32, R25 28, R27 21 `NOT_RUN` rows.
8. **Software implementation inventory:** [BOAT_R30A_SOFTWARE_INTEGRATION_MATRIX.csv](BOAT_R30A_SOFTWARE_INTEGRATION_MATRIX.csv) — 17 mapped source subsystems including default-disabled radio/physical HAL, simulated control and deferred sonar. A source file existing does not prove production runtime integration. Never convert a host-simulation PASS into a physical PASS.

## Selected module and logical power contract

| Module | Qty | Input/logic | Expected functions / remaining essential check |
|---|---:|---|---|
| ESP32-S3-DevKitC-1-N32R16V | 1 | Olimex DCDC6-16-TO5 5 V / USB entry | confirm exact PCB, J1/J3 labels, USB/5 V backfeed and memory SKU |
| HGLRC M100 PRO with QMC5883L | 1 | main regulated 5 V; 3.3 V-compatible *signals only after measurement* | UART4/5, I2C7/8, six-way cable cavity order |
| GY-91 MPU-9250 / BMP280 | 1 | Pololu D24V10F3 #2 3.3 V | authentic IMU ID and I2C voltages/address |
| Ardboard INA231 #1491 | 1 | Pololu #2 3.3 V | I2C 0x40 **candidate**, protected single low-current BUS sense |
| Waveshare MCP23017 15391 | 1 | Pololu #2 3.3 V | I2C 0x27 **candidate**, reset/address strap, GPA/GPB actual pins |
| nRF24L01+ PA/LNA | 1 | Pololu D24V10F3 #1 3.3 V | exact SKU/header orientation, local decoupling, radio identity |
| EBYTE E22-900T22S UART | 1 | regulated main 5 V, 3.3 V TTL only after level test | 26-pad factory carrier, AUX/TXD, matching EU868 antenna and radio pair |
| Cytron MDD20A | 1 | common 3S pack VM; 3.3 V PWM/DIR logic | two 550 brushed motors, no rudder, independent current/stall + boot PWM checks |
| Electrokit EKM014 | 1 | **common** 3S pack VH, regulated 5 V VL | four 12 V solenoids (two hoppers, two hooks), one-at-a-time logic, actual coil pulse/current |
| TRACO TSR 2-2450 | 1 | 3S in, 5 V VL out | premade secure SIP-3 socket, feed protection, thermal verification |
| Olimex DCDC6-16-TO5 | 1 selected | 3S in, 5 V main out | USB-A / barrel-input mating, return, ripple, real current budget |
| Pololu D24V10F3 | 2 | common 3S in, two **separate** 3.3 V regulated *logic* rails | independent output wires (never parallel converters), ripple/peak, input fusing |
| 3S4P 18650 + 20 A BMS | 1 | 9–12.6 V design sensing range | actual pack assembly, fuse/current/temperature/stall/short protection not qualified |

**Do not invent a fuse rating, common stop-contactor rating, wire gauge, solenoid pulse width or motor stall current.** The exact accessory/terminal/connector SKU and high-current safety architecture must be selected only from measured current, conductor length, insulation and environmental requirements. A "20 A" BMS label does not imply adequate peak starting current or protect every downstream branch.

## Assembly phases and permitted actions

### Phase A — receive and identify hardware, **no batteries connected**
Document photos of **both PCB sides**, manufacturer's BOM/order codes, physical dimensions, terminal naming, jumper settings, antenna connectors and cable cavity orientation. Mark `UNWIRED_PENDING` records as **not installed** until the *actual* bought revision is checked against the effective netlist. Label loose harnesses, keyed connectors and logic vs power paths. A proposed net is NOT an installation instruction when its actual module connector/pad is unverified. Do not insert the BMS into the high-current actuator path yet.

### Phase B — unpowered mechanical layout and continuity
Mount ready-made modules without custom PCB; keep motor VM and coil VH physically **disconnected**, coil load leads not energized, main battery isolated. Inspect chafe/waterproofing/strain relief; prove connector polarity and reference ground with a meter before any powered logic test. There is no evidence of actual clearance/thermal measurements yet.

### Phase C — isolated, **current-limited logic-only** bench
Only after the relevant R23/R25 logic-level and supply-path checks are signed, verify each converter separately using the manufacturer current/voltage range. Verify ESP boot/USB backfeed, 3.3 V sensor and RF rails, shared I2C scan and pull-ups, M100/E22 5 V-powered TX/AUX/SDA levels, radio carriers and battery-sense **without the unverified INA231 shunt**. Run selected target diagnostic and a separate *default-locked* R30A INA231-only bench sketch; DO NOT enable INA231 physical commissioning or actuator outputs based on a simulated fixture. Use a suitable protected low-current sense feed only after INA231 actual BUS continuity is checked.

### Phase D — common 3S actuator power **BLOCKED**
The chosen common 3S bus would eventually feed both MDD20A VM and EKM014 VH. **DO NOT connect it yet.** Prerequisites include actual battery/BMS/cable/terminal protection, verified isolation/STOP (could be ONE shared DC-rated master disconnect; **not a separate battery/coil supply**), watchdog failure modes, output stuck-on protection, post-pulse current-zero and measured bounded releases. Existing code presently expects *separate feedback contact fields*; these cannot simply be tied together or simulated by MCP23017 register readback. Either qualify compatible physical feedback or update the architecture and rerun failure tests before load power.

### Phase E — physically supervised dummy-load, dry-land and controlled-water testing **BLOCKED**
Only after Phase D acceptance: measure both motor directions, speed, peak/start/stall draw (within BMS and conductor ratings), all four release pulse widths/current/temperature with **one active at a time**, failure on MCU freeze, RF loss and restoration, battery collapse, UART/I2C faults, radio/authentication/replay, GPS/Home reliability, geofence, 3 m target arrival and repeatable test outcomes. Initial water trials require safe retrieval; software RTH alone cannot guarantee a hazard-free route or obstacle avoidance.

## Software and comments/readme audit

- **Compile/test proof:** R29A Boat host Actions [45040175603](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45040175603) PASS. Board-specific diagnostic [45038989512](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45038989512) PASS on source commit `b9964105`; later R29 modifications were tests/docs/tools only. New R30A changes require fresh CI observation.
- **Safety code:** `boat_physical_driver_adapter.h` accepts no driver without separate `independentHardwareCommissioned`; `boat_r29_shared_bus_contract.h` rejects simultaneous motor+solenoid commands; `boat_battery_policy.h` handles low/critical/invalid readings; `boat_radio_safety.h` handles both-link loss and conditional 10-second RTH. All are **software gates**, not proof of relay power cutoff.
- **Radio security:** must provision real authenticated pair keys using the documented secure commissioning path; diagnostic runtime does not establish full real nRF/E22 operation by itself.
- **Firmware readiness:** present diagnostic firmware is commented and compiles, but **full operating firmware with real actuator HAL, trusted physical feedback provider, live telemetry and confirmed navigation is NOT complete**. No claim of final production firmware or Boat VER015.
- **Documentation hygiene:** the old Heltec screenless V2 R30/R31 notes, R21/R22 ADS1115 tests and raw R23 power connections are historical only; never apply them on top of this merged schedule.

## Binary release and readiness label

**Current release markers remain Boat VER014 / Remote VER028.** No new operational ZIP release promoted. Safe label: **`R30A — Design handoff ready for bench planning and unpowered fit/continuity preparation; not actuator-power commissioning ready.`** Physical work must stop at any unknown connector polarity, electrical limits, missing feedback, unsigned power check or unqualified protection.

## R30A observed full CI verification — 2026-11-22

- [**Boat host Actions #45042597229**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042597229), head `308fafa10a1d506e4ae42179e2c563608a51ae38`: **SUCCESS**. The R29 shared-3S-fault C++17 integration, R30A 73-effective-net/24-gate package validator and its seven hostile-mutation cases, and all downstream Boat host steps completed successfully. The successful R30A package checker confirms **17 software source references**, **64 unwired + 7 NC + 2 aliases**, and **81 physical NOT_RUN** lines; no real wire or safety component was tested.
- [**Boat ESP32-S3 target Actions #45042370480**](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45042370480), head `36b5a8afe7343a744584a62f1e3c2a9441c51c2d`: **SUCCESS**. Compiled diagnostic Boat firmware **408681 bytes program / 21160 bytes global RAM**; historical locked R22 ADS bench **302617 bytes / 18432 bytes**; new **selected locked R30A INA231 bench 302553 bytes / 18440 bytes**. Firmware binaries and build logs uploaded as GitHub Actions artifacts.
- Both runs are **source compilation and document/fault simulation evidence only**. The new INA231-only binary has `INA231_PHYSICAL_COMMISSIONED=false`, does not configure I2C pins in default build, and has no motor/release API. Hardware application still has no `BoatPhysicalDriver` attached.
- This verifies the R30A design handoff package **for unpowered/inspection-only prototype preparation and future supervised logic-only bench**, not for motor/solenoid high-current connection or water use. All 24 R30A physical gates OPEN, old 81 physical checks NOT_RUN. No new regulator, breaker/contactor, BMS or fuse SKU selected and no physical electrical acceptance performed. Boat VER014 / Remote VER028 unchanged.

