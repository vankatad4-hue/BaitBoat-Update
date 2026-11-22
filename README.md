# Flytec V080 modular bait-boat retrofit

Active development branch: `hardware-ready-dual-radio-r1`

This repository contains the Boat controller, handheld Remote, firmware, hardware decisions, tests and commissioning procedures for the Flytec V080 retrofit.

## Current hardware authority

- **Boat:** [Hardware Selection R20](Boat/docs/BOAT_HARDWARE_SELECTION_R20.md)
- **Remote:** [Remote project master status](Remote/docs/PROJECT_MASTER_STATUS.md)
- **Battery/RTH behavior:** [R79 low-battery failsafe](Boat/docs/BOAT_R79_LOW_BATTERY_FAILSAFE.md)
- **Bench procedure:** [R79 battery and radio-loss dummy-load test](Boat/docs/BOAT_R79_LOW_BATTERY_BENCH_TEST.md)
- **Continuation checkpoint:** [CONTINUATION_CHECKPOINT_VER001.md](docs/CONTINUATION_CHECKPOINT_VER001.md)
- **Progress review:** [PROJECT_PROGRESS_REVIEW_R1.md](docs/PROJECT_PROGRESS_REVIEW_R1.md)
- **Historical archive:** [docs/archive/2026-11-22](docs/archive/2026-11-22/README.md)

Older Heltec Wireless Tracker/SX1262 and central-carrier-PCB documents are archived. The active design uses ready-made modules and removable wiring; no custom PCB is required.

## Boat hardware baseline

| Function | Active selection |
|---|---|
| Main controller | ESP32-S3-DevKitC-1-N32R16V |
| GNSS and compass | HGLRC M100 PRO with QMC5883L |
| IMU/barometer | GY-91 (MPU-9250 + BMP280) |
| Primary radio | nRF24L01+ PA/LNA |
| Backup radio | EBYTE E22-900T22S |
| I/O expansion | MCP23017 |
| Battery voltage sensor | **Ardboard INA231 product 1491, BUS-only**, 3.3V, 0x40 candidate |
| Motor driver | Cytron MDD20A dual channel |
| Four-channel release driver | EKM014 |
| Main 5V rail | Olimex DCDC6-16-TO5 |
| EKM014 logic 5V | TRACO TSR 2-2450 |
| Sensor/RF 3.3V rails | 2× Pololu D24V10F3 #2830 |
| Boat battery | 3S4P 18650 Li-ion, 11.1V nominal, ~10Ah, 20A BMS |

**R27 current selection:** Boat battery sensing uses **Ardboard INA231 product 1491 in VBUS-only mode**, separate from the 20A current path. Never connect the board's 0.1Ω R100 shunt in series with the battery or motor loads. Battery sense and all actuator supplies remain uncommissioned/disconnected until actual PCB/pin validation. See [R27 selection and acceptance](Boat/docs/BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md). The former ADS1115 divider/R21 documents are historical.

## Remote hardware baseline

- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480, LVGL 9.6.
- nRF24L01+ PA/LNA primary link and E22 backup link.
- 1S4P 18650 remote battery pack.
- SD-backed offline maps and up to 16 saved waypoints.
- Bulgarian operating interface and joystick calibration.

The Remote shows the **Boat** battery voltage and estimated charge separately from the Remote's own battery. Green means OK, yellow means mid-level/recharge recommended, red means low. Stale or missing Boat battery telemetry is shown as unavailable, never as a fabricated percentage.

## R79 low-battery behavior

The Boat measures its own 3S pack locally; radio connectivity is not required for the decision.

- Above 11.4V: green.
- 10.8–11.4V: yellow.
- At or below 10.8V for 3 seconds: red; hold outbound mission motion and initiate validated Home diversion.
- At or below 10.2V for 1 second: critical STOP.
- At or below 9.9V: immediate critical STOP.
- **An invalid sample stops propulsion immediately**; a previously valid sample older than one second also fails closed.
- If an autonomous mission is active, low battery cancels the outbound target and remaining release sequence, then returns Home only with valid Boat NVS Home, navigation and local safety interlocks.
- If no autonomous mission is active, the Remote warns and manual return remains the operator's responsibility. A new mission is blocked at red.
- All hopper and hook outputs remain OFF during battery RTH. Critical battery overrides RTH and stops all outputs.

The voltage-to-percentage curve is an estimate, not coulomb counting. Thresholds are commissioning defaults and must be validated with the actual cell model, pack under load and calibrated divider before use on water.

## Radio-loss behavior

Both radio links lost: immediate STOP. After 10 seconds from the last accepted authenticated frame, local RTH is permitted only if an active eligible mission, matching CRC-valid Boat NVS Home, valid navigation and physical safety interlocks are available. Only Home navigation is allowed without radio; solenoids stay OFF.

## Software and hardware acceptance

Host tests and target builds are being run by GitHub Actions. A successful compile or host simulation does not prove physical sensor calibration, radio range, power integrity, motor-driver behavior or solenoid feedback.

The selected parts are not yet physically commissioned. The candidate GPIO map is documented, but all pin-audit flags remain unsigned until the purchased board revision is checked. Actual motors and solenoids must remain disconnected until the dummy-load procedures pass. Boat VER014 and Remote VER028 remain the current version markers; no physical acceptance or release bump is implied.

Sonar selection remains deferred.

## R22 isolated Boat battery-sensor bench

The [R22 ADS1115 battery bench sketch](Boat/hardware_test/ADS1115_Battery_Bench/ADS1115_Battery_Bench.ino) is built **separately from the Boat actuator firmware**. It is GPIO locked by default. Following a real physical pin/power audit, a private local header may enable I²C readout of ADS1115 raw codes and an uncalibrated pack-voltage estimate as serial CSV. It imports no radio, motor or solenoid code and grants **no safety authority**. See [R22 sensor commissioning instructions](Boat/docs/BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md). Both the Boat host tests and the main+locked-bench ESP32-S3 target compilation passed. Physical testing remains NOT_RUN.

## R23 Boat GPIO and modular wiring authority

[**R23 manufacturer-header GPIO and connection guide**](Boat/docs/BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md) is the current physical-wiring *design* reference, with the [64-net CSV harness schedule](Boat/docs/BOAT_R23_MODULE_CONNECTION_MATRIX.csv). It confirms the **18 distinct DevKitC-1 GPIO/header positions**, 0x27 Waveshare MCP23017 address jumper expectation, E22-900T22S 26-pad mapping, 3.3V nRF24 supply, 3.3V-safe EKM014 control inputs, and selected four regulator rail roles. [The current R20 purchase register](docs/PROJECT_PURCHASE_REGISTER_R20.csv) has been reconciled: Pololu D24V10F3 #1 supplies nRF24; #2 supplies GY-91/INA231/MCP23017; E22 pad25 is supplied from main 5V. The optional Ardboard nRF socket adapter is **not** an alternative approved 5V power path.

The actual purchased PCB revision and headers, M100 PRO 5V-powered UART/I²C signal levels, E22 TXD/AUX levels, RF carriers, independent STOP/watchdog/current-zero hardware, battery ADC calibration and output dummy-load acceptance remain **unverified**. Firmware safety flags remain off and high-current motor/solenoid inputs must stay disconnected.

The [R23 electrical sign-off worksheet](Boat/docs/BOAT_R23_GPIO_AND_LEVEL_SIGNOFF.csv) enumerates 32 physical tests (18 GPIOs and 14 interface/power/safety checks), all **NOT_RUN**. The documented header mapping is not a signed physical acceptance.

## R24 boot/reset diagnostic follow-up

The [R24 shared MCP23017 boot/reset safety review](Boat/docs/BOAT_R24_BOOT_RESET_OUTPUT_SAFETY.md) records the manufacturer's EKM014 120 kΩ input pull-down behavior and adds a **non-actuating software register audit** covering both EKM014 GPA0–3 and MDD20A DIR GPB0–1. It includes simulated reset, active output and I²C-failure regression tests in Boat host CI. This is **not an independent electrical watchdog or cutoff**, does not claim coil-current-zero, and does not authorize motor/solenoid high-current wiring. All R23 electrical signoffs are still NOT_RUN; Boat VER014 / Remote VER028 unchanged.

## R25 isolated power and level safety review

The [R25 preflight](Boat/docs/BOAT_R25_POWER_LEVEL_PREFLIGHT.md) and [unsigned 28-gate evidence sheet](Boat/docs/BOAT_R25_POWER_LEVEL_EVIDENCE.csv) cover the selected four logic regulator rails, isolated M100 PRO/E22 logic levels, connector identities, PWM/coil boot idle behavior, ADC supply sequencing and physical STOP/watchdog/current-zero acceptance. All measurements remain NOT_RUN. The [R25 evidence checker](Boat/tools/verify_r25_preflight.py) rejects unsupported sign-offs and is run in CI in explicitly **unsigned** mode; passing this check does not commission hardware.

**Important:** R21 150kΩ/33kΩ ADS1115 battery divider must stay **disconnected from the 3S pack** until the ADC VDD versus AIN0 sequencing is engineered/qualified; an unpowered ADS1115 may otherwise see an out-of-limit input even though 2.272V at 12.6V pack is acceptable when VDD=3.3V. High-current MDD20A and EKM014 paths also remain physically disconnected. Boat VER014 / Remote VER028 unchanged.

## R26 battery-sense alternative research

The [R26 INA226 voltage-only module review](Boat/docs/BOAT_R26_INA226_VOLTAGE_ONLY_REVIEW.md) assesses TI INA226 as a potential **factory-assembled substitute** for the R20 ADS1115 divider module. TI explicitly supports measuring the 0–36V VBUS input even with chip supply off, avoiding the specific R21 ADC back-powering hazard. The [15-gate candidate acceptance sheet](Boat/docs/BOAT_R26_CANDIDATE_SIGNOFF.csv) is wholly **NOT_RUN**; identity/power/connector and series-shunt risks remain. A host-only INA226 voltage codec/test is compiled under Boat CI, with `INA226_HARDWARE_SELECTED=false` so no change reaches actual firmware or safety authorization. The current ADS1115 stays selected but must **not** have its pack-sense wire connected until safe sequencing is proven or the owner approves a vetted assembled replacement. Boat VER014 / Remote VER028 remain frozen. Independent hard STOP/coil feedback remains outstanding.

## R27 active Boat battery sensor selection

**Selected:** [Ardboard INA231 I²C product 1491](Boat/docs/BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md), protected **VBUS-only 3S measurement**, NOT a series pack-current sensor. The physical Ardboard 0.1Ω shunt cannot be placed in the 20A motor/solenoid circuit. [21 physical acceptance checks](Boat/docs/BOAT_R27_INA231_ACCEPTANCE.csv) remain **NOT_RUN**. The retired ADS1115 polling path is software-blocked; real INA231 I²C acquisition awaits purchased-board inspection and bench tests. Until commissioned, no valid Boat battery safety sample exists and the Remote must show unknown rather than a fabricated charge level. Original versions Boat VER014 / Remote VER028 unchanged.

The [R27 10-net wiring override](Boat/docs/BOAT_R27_INA231_HARNESS_OVERRIDE.csv) supersedes the old ADS1115 connections in the R23 wire matrix only for the battery sensor; all new INA231 nets remain unwired or deliberately no-connect until exact board inspection.

## R28 logic-only prototype integration

Added [R28 INA231 acquisition and fault-simulation package](Boat/docs/BOAT_R28_INA231_LOGIC_SIMULATION.md): new-conversion `0x4122` BUS-only triggered measurement, CVRF check, 40ms timeout, staleness and I²C error rejection, Arduino `TwoWire` framing and Boat sensor interface integration. Local portable host tests passed; the exact latest Boat target/Actions results are not independently verified. The global hardware-commissioning flag, local pin/board/calibration flags and actuator output gates all remain OFF. No actual sensor or propulsion hardware has been commissioned. Boat VER014 / Remote VER028 frozen.

## R29 one common Boat power bus

**Owner-confirmed topology:** Both MDD20A motor power `VM` and EKM014 solenoid FET power `VH` connect to **one common 3S4P battery power bus and return** (parallel loads), **not separately isolated or independently switched motor/solenoid supplies**. EKM014 `VL` is still regulated 5V because it is a different logic voltage, derived from that same pack. [R29 eight-net wiring override](Boat/docs/BOAT_R29_SHARED_3S_BUS_CONNECTIONS.csv) and [integrated fault simulation](Boat/docs/BOAT_R29_SHARED_BUS_FAULT_SIMULATION.md) supersede old separate actuator power wording. No high-current wiring, common fuse, independent STOP/feedback or measured motor/coil currents are approved yet; the shared 20A BMS cannot by itself certify startup/stall current safety.

R29 adds host-only tests for R78 radio loss/conditional RTH, R79 battery critical STOP, R28 INA231 acquisition failure, invalid nav/stale safety, and one-hot software output shapes. Real physical driver commissioning remains false and Boot/Remote versions remain 014/028. New R29 GitHub CI status still unverified.

## R30A documented Boat prototype handoff

The current [R30A Boat assembly and physical-test handoff](Boat/docs/BOAT_R30A_PROTOTYPE_ASSEMBLY_HANDOFF.md) consolidates the historical R23 wire list, selected R27 INA231 voltage-only sensor and owner-confirmed R29 one-common-3S VM/VH power distribution into [73 effective connection rows](Boat/docs/BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv) (64 still unwired, 7 deliberate no-connects, 2 reference-only aliases). The [24 open acceptance blockers](Boat/docs/BOAT_R30A_READINESS_GATES.csv) and [17-subsystem software matrix](Boat/docs/BOAT_R30A_SOFTWARE_INTEGRATION_MATRIX.csv) document what is and is not complete. R30A adds a **default-locked INA231 sensor-only bench sketch** and source-consistency CI; no new hardware, custom PCB or separately switched motor/solenoid bus.

**Boundary:** This is documentation for procurement, unpowered inspection/assembly planning and later logic-only testing after real PCB/rail signoff. It is **not** a released powered motor/coil design, and the default `Flytec_Boat.ino` is still a non-actuating diagnostic. The one 3S BMS supply, power wiring ratings, independent failsafe hardware, release current feedback and complete installed HAL remain unverified. Boat VER014 / Remote VER028 unchanged.

## R30B shared 3S application power-fault guard

[Boat R30B brownout/freshness testing](Boat/docs/BOAT_R30B_SHARED_SUPPLY_FAULT_SIMULATION.md) now covers an application-integrated local battery-evidence veto for shared-bus power faults, INA231 staleness, missing conversions and critical battery. [Boat host CI](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45043490071) passed. The physical driver, independent STOP, actual wiring/connector current ratings and full operational Boat firmware are **not** commissioned. Releases remain Boat VER014 / Remote VER028.

## R30C one shared 3S supply — physical safety feedback pending

The Boat's [R30C physical feedback contract](Boat/docs/BOAT_R30C_ONE_BUS_SAFETY_FEEDBACK_CONTRACT.md) identifies a remaining mismatch between legacy dual-contactor firmware assumptions and the selected ONE 3S4P pack shared by Cytron MDD20A VM and EKM014 VH. R30C defines a software-only review contract for ONE common DC-rated disconnect and separate hardwired STOP/watchdog/current-zero/coil limiting functions, without adding or energizing hardware. `COMMON_BUS_HARDWARE_COMMISSIONED=false` preserves the output lock, and all physical approval gates remain OPEN.

## R30E — local-only Boat safety fault telemetry

The Boat now has documented [R30E local fault reasons and diagnostic envelope](Boat/docs/BOAT_R30E_LOCAL_FAULT_TELEMETRY.md), with a default-locked USB serial report and portable host tests for the read-only common-bus safety inputs. An actual hardware safety reader is still unselected; the report correctly states `NO_READER`, not healthy. This is **not** authenticated Remote RF telemetry or a safety-override command. Shared 3S motor/solenoid supply, unsigned physical interlocks, and version markers remain unchanged.

## R30F — Boat ↔ Remote unverified safety diagnostic messages

[New R30F protocol document](Boat/docs/BOAT_R30F_DIAGNOSTIC_WIRE_PROTOCOL.md). Boat and Remote now share a strict version-1 subtype-8 optional authenticated fault telemetry format, with no change to the established 32-byte radio frame or command types. Remote displays `ДИАГН.: НЕПОТВ.` independently of STOP and battery warnings, while rejecting diagnostic frames as proof of commissioned electrical safety or primary control-link freshness. Actual external STOP/watchdog/current-zero hardware remains unavailable; Boat reports `NO_READER` and retains all actuator locks. The one common 3S pack and Boat VER014 / Remote VER028 remain unchanged.

## R30F confirmed verification — 2026-11-22

Actual GitHub Actions results were inspected after each job completed:

- **[Boat host #45048750474](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048750474): SUCCESS.** R30F strict subtype-8 mirrored wire-vector tests, static Boat/Remote security contract and all other Boat portable host tests passed.
- **[Remote host #45048847021](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048847021): SUCCESS.** R30F paired codec golden vectors, invalid/forged-commissioned packet rejection and telemetry-isolation test passed, including a test that diagnosis cannot clear an already confirmed STOP. All other Remote portable host tests passed.
- **[Boat ESP32-S3 target #45048442529](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048442529): SUCCESS.** Diagnostic-only Boat target includes R30F optional authenticated send route, plus locked INA231 and historical locked ADS1115 benchmarks. This is a compilation test, **not** real RF transmission or actuator work.
- **[Remote Waveshare ESP32-S3 4.3-inch target #45048618202](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048618202): SUCCESS.** Complete Remote LVGL/SD/UI/radio target compiled, **1,460,005 / 3,145,728 program bytes (46%)**, **179,512 / 327,680 global dynamic RAM bytes (54%)**, 148,168 bytes reported available for local variables. This is compile output, not a measured runtime memory budget or physical board acceptance.

Intermediate early R30F CI runs failed because their respective commit snapshots invoked `verify_r30f_diagnostic_protocol.py` before that file and its linked R30F document were committed; both strict C++ wire codecs already passed in those runs. The later complete host runs above are **SUCCESS**. No command type or radio-frame authentication contract was changed.

**Status at R30F:** Boat and Remote software paths, strict codecs, host tests and ESP32-S3 builds are verified. Physically uninstalled and unverified STOP/watchdog, shared DC cutoff, independent one-hot coil limiter/current-zero sensors and actual RF range remain **outstanding**. Both endpoints retain all motor/coil STOP locks; R30A 24 physical gates OPEN and prior 81 physical tests NOT_RUN. Single shared 3S pack, no separately switched motor/coil domains, no new module selection. Boat VER014 / Remote VER028 unchanged.

**Next engineering step:** R30G — review rate limiting and bounded-queue/backpressure behavior of diagnostic packets during manual control and deliberate link loss, then regression-test no control-freshness or priority inversion. Physical radio acceptance and motor power remain blocked.

## R30G — Boat diagnostic traffic cannot queue behind controls

[Verified R30G design and tests](Boat/docs/BOAT_R30G_RADIO_DIAGNOSTIC_PRIORITY.md) add a one-slot, short-lived, primary-nRF-only optional diagnostic budget. Heartbeat, STOP/ACK, mission and radio-loss policy run before any diagnostic attempt; old optional observations are dropped instead of accumulating. [Boat host #45049989270](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45049989270) and [Boat ESP32-S3 target #45049610592](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45049610592) passed. Remote firmware, selected ONE common 3S4P battery bus, physically unsigned independent STOP and official Boat VER014 / Remote VER028 remain unchanged.
