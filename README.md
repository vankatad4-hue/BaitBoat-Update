> **R30H-25 (2026-11-23) · physical-driver fail-closed latch and radio-session neutral rearm:** Boat's **production** application adapter `Boat/source/boat_application_port.cpp/.h` and motor supervisor now (a) require a fresh authenticated **neutral** manual input on a noninitial radio-session generation change or local fault before accepting nonzero manual sticks; and (b) permanently refuse all subsequent actuator commands in the same application instance after `BoatPhysicalDriver::apply()` reports failure, reasserting `disable()`, rejecting queued requests and reporting physical STOP as UNKNOWN rather than claiming current zero. One-shot sensor-read failure locally STOP-latches; fresh primary CLEAR_STOP plus neutral is required for controlled *software* recovery. A 9.8-V simulated pack collapse also remains STOP after voltage and session recovery. New host-only suites use real `BoatApplicationPort.cpp` with replaceable stubs and run in Boat host and PlatformIO CI, plus mutation-enforced source audit. **Boat host PASS [#45209114670](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45209114670)** at source commit `6f51c14c`; verify [both PlatformIO targets](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45209114729) before declaring PASS. [R30H-25 report](docs/R30H_25_DRIVER_FAULT_AND_NEUTRAL_RECOVERY.md). **No hardware tests or physical STOP/current-zero acceptance**; R30H-10 12 NOT_RUN / 6 BLOCKED, R30H-12 7 NOT_RUN / 5 BLOCKED. Boat **VER014**, Remote **VER028** remain frozen.

> **R30H-24 (2026-11-23) · local navigation witness, `millis()` rollover and application battery retry regression:** Boat's real actuator supervisor now independently refuses autonomous motor output on locally **stale GNSS/heading, invalid geofence/accuracy or invalid compass angle**, even if an earlier calculated navigation result is still marked valid. No manual-control/STOP admission is loosened. New host suites `boat_r30h24_wrap_failsafe_test.cpp` and `boat_r30h24_application_wrap_test.cpp` exercise exactly 100/300/2,000-ms local sample boundaries over 32-bit rollover; manual motor lease 200 ms; coil pulse and 500-ms idle-feedback deadline across rollover; backup-only interruption/no coil replay; battery low-RTH latch after 3 seconds, one Home request despite repeated 10.7-V reports; and immediate 9.8-V critical STOP with no auto-rearm. **Boat host CI PASS** at [#45207416020](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45207416020) on `1c21d511`. [Full report](docs/R30H_24_WRAP_NAV_INPUT_APPLICATION_BATTERY.md). Check [Boat+Remote PlatformIO CI](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45207416065) conclusion for the same executable commit before calling target PASS. **Physical hardware/RF safety gates remain locked:** R30H-10 12 NOT_RUN / 6 BLOCKED; R30H-12 7 NOT_RUN / 5 BLOCKED. Official Boat **VER014** / Remote **VER028** unchanged.

> **R30H-23 (2026-11-23) — integrated simulated local failsafe acceptance matrix:** New `Boat/tests/boat_r30h23_failsafe_integration_test.cpp` exercises production Boat radio watchdog, 10,000-ms authenticated dual-link-loss gate, mission controller, motor/coil supervisor, locally sampled GNSS/heading/safety feedback, Home-record validation, and shared-3S battery/critical-voltage guards. **Software-only** scenarios include immediate STOP before 10s; eligible local Home-only navigation after 10s; backup-only continued outbound mission without release permission; 15 independently invalid Home-admission inputs; seven mid-RTH faults; 3-second low-battery RTH and immediate 9.8V critical STOP; interrupted coil with no replay; and primary STATUS=UNKNOWN when feedback is invalid. CI: [Boat host run #45205207217](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45205207217), [PlatformIO run #45205207291](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45205207291); verify exact commit verdict before announcing PASS. [R30H-23 engineering report](docs/R30H_23_LOCAL_FAILSAFE_SIMULATION.md). **No live hardware acceptance:** R30H-10 12 NOT_RUN / 6 BLOCKED, R30H-12 7 NOT_RUN / 5 BLOCKED. Boat VER014 / Remote VER028 unchanged.

> **R30H-22 (2026-11-23) — two-endpoint authenticated STOP / ACK / STATUS failure-injection simulation:** New portable `tests/r30h22_dual_stop_scheduler_test.cpp` composes **production Boat STOP ACK/status priority outboxes, Remote bounded STOP retry, Remote ACK tracker/confirmation gate, and each endpoint's real 32-byte HMAC/CRC/command/payload codecs**. Tests busy TX, first-packet drops, failed Boat ACK TX, UNKNOWN feedback, all-STOP/ACK/STATUS loss, timeout/retry exhaustion, duplicate ACK, new STOP ID and new session/epoch key. Fixed cross-endpoint duplicate declarations with one shared portable payload-header guard; neither wire ABI nor operational GPIO changed. **Boat host PASS [#45201870070](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45201870070), Remote host PASS [#45201870153](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45201870153), source audit PASS [#45201870177](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45201870177) on `e1475248`; PlatformIO verdict must be checked for the same source commit. [R30H-22 report](docs/R30H_22_DUAL_ENDPOINT_STOP_SCHEDULER.md). This is **logical software evidence only**, not real RF delay/current-zero or hardware commissioning. R30H-10 **12 NOT_RUN / 6 BLOCKED**, R30H-12 **7 NOT_RUN / 5 BLOCKED**; **Boat VER014 / Remote VER028 unchanged.**

> **R30H-21 (2026-11-23) · `platformio-unified-r30h20` · primary nRF STOP/STATUS TX sequence-budget fix:** Boat now checks that an ordinary primary TX slot is truly free **before** assigning a 16-bit authenticated frame sequence to heartbeat, mission feedback, noncritical telemetry and ordinary command ACKs. Pending STOP ACK and latest authoritative STATUS preempt these messages; the separate LoRa path and STOP ACK retry budgets are unchanged. Portable 50,000-attempt busy-TX scheduling stress and 1,024-frame authenticated Boat/Remote CRC/HMAC/loss/replay/STATUS tests added, plus mutation-enforced source audits and PlatformIO CI. [Engineering report](docs/R30H_21_PRIMARY_TX_BUSY_SEQUENCE_BUDGET.md). A host PASS verifies only simulated software; check CI against exact new HEAD. **R30H-10 12 NOT_RUN / 6 BLOCKED, R30H-12 7 NOT_RUN / 5 BLOCKED; Boat VER014 / Remote VER028 unchanged.**

> **R30H-20 (2026-11-23) — NEW PRIMARY BRANCH [`platformio-unified-r30h20`](https://github.com/vankatad4-hue/BaitBoat-Update/tree/platformio-unified-r30h20).** Complete tracked-file project inherited from `hardware-ready-dual-radio-r1` at `e0b6220d`; source, UI, CAD, documents, tests and previous releases retained. Added a VS Code / PlatformIO audit/build/test/snapshot command (`tools/flytec.py`), full-project Git-tracked ZIP/SHA256 manifest, CI triggers for the new branch, and authenticated 32-byte STOP/ACK/STATUS fault tests compiled separately against Boat and Remote protocol source. Fixed critical battery STATUS reserved `0x80` bit and invalid STATUS outbox priority starvation. This is **compile-only software development, not physical approval**; CI results require exact-commit verification. [R30H-20 handoff](docs/PLATFORMIO_UNIFIED_R30H20.md). **R30H-10 12 NOT_RUN / 6 BLOCKED; R30H-12 7 NOT_RUN / 5 BLOCKED; Boat VER014 / Remote VER028 unchanged.**

> **R30H-19 (2026-11-23) — prioritized real Boat STATUS after STOP ACK:** A new one-slot/latest-status outbox prevents 100 ms heartbeat TX from starving the subsequent 200 ms Boat STATUS. Scheduler order: nRF TX completion → authenticated RX → correlated STOP ACK → latest STATUS → heartbeat → optional diagnostics/UTC. Each STOP or session/PHY fault invalidates prior STATUS; snapshots expire after 300 ms. No synthetic stopped reading: status still comes only from application safety/output-feedback samples. Integrated portable Boat ACK + STATUS + Remote gate simulation and source/CI audits added. **Boat host test PASS** on code commit 7863ea0; both PlatformIO targets on code commit 99a7046 require final CI verdict. [R30H-19 report](docs/R30H_19_STOP_ACK_STATUS_INTEGRATION.md). Physical R30H-10 **12 NOT_RUN / 6 BLOCKED**; R30H-12 **7 NOT_RUN / 5 BLOCKED**. Boat VER014 / Remote VER028 unchanged.

> **R30H-18 (2026-11-23) — Boat STOP ACK priority under nRF24 TX-busy:** Boat now retains a single command-ID STOP ACK when its primary RF24 TX slot is occupied, sends it before heartbeat/mission/diagnostic traffic and retries failed radio TX within 8 admissions / 75 ms gap / 2.5 s maximum age. Session/PHY fault resets the outbox. This is an ACK delivery improvement, **not physical STOP or current-zero evidence**; Remote still requires matching authenticated STOP ACK **and subsequent fresh primary stopped-STATUS**. [R30H-18 technical report](docs/R30H_18_BOAT_STOP_ACK_TX_BUSY_PRIORITY.md). Portable Boat host CI PASS on code revision 8ce51f7; PlatformIO outcome must be checked on latest source revision. Physical R30H-10 **12 NOT_RUN / 6 BLOCKED**, R30H-12 **7 NOT_RUN / 5 BLOCKED**; Boat VER014 / Remote VER028 frozen.

> **R30H-17 (2026-11-23) — correlated STOP ACK / primary STATUS evidence:** A newly pressed STOP can no longer be confirmed solely by a previously emitted Boat `stopState=1` status. Remote now requires an authenticated **matching command-ID STOP ACCEPTED/EXECUTED ACK**, followed by fresh primary Boat stopped-status. Until then the STOP latch remains asserted. The 8-admission / 150-ms / 2.5-s bounded retry policy remains; after the 8th admission heartbeat is permitted so Boat can send feedback, without unlocking movement. [R30H-17 report](docs/R30H_17_STOP_ACK_STATUS_CORRELATION.md). All physical R30H-10/R30H-12 STOP/current-zero gates still unsigned; compile-only and releases Boat VER014 / Remote VER028 frozen.

> **R30H-16 (2026-11-23) — bounded primary nRF STOP retransmission:** Remote now keeps STOP queued after an nRF buffer-admitted TX; it reattempts at most **8 transmissions** and at least **150 ms apart** within a **2,500 ms local deadline**, preserving command ID and preempting ordinary primary heartbeat. Only authenticated Boat STATUS `stopState==1` clears pending STOP and its confirmation latch; ACK ACCEPTED or TX admission does not. Queue resets on session/bridge fault/expiry; local STOP confirmation requirement remains asserted on failure. [Full R30H-16 report](docs/R30H_16_STOP_DELIVERY_CONFIRMATION.md). Code-commit host CI Boat #45133874972 PASS, Remote #45133874907 PASS, source inventory #45133874893 PASS; target compile verdict check [PIO #45133874886](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45133874886). No physical signoff: R30H-10 12 NOT_RUN/6 BLOCKED; R30H-12 7 NOT_RUN/5 BLOCKED. Boat 014 / Remote 028 unchanged.

> **R30H-15 — Remote primary nRF24 STOP TX-busy priority (2026-11-23):** Fixed a STOP packet-admission gap: if nRF TX is busy the authenticated STOP is now queued locally and reattempted **before primary heartbeat/mission traffic**, with unchanged command ID, a maximum 2,500 ms local age, and clearing on session reset/bridge fault. Backup E22 STOP admission still runs separately. Local queue acceptance is **NOT** Boat receipt, physical STOP or current-zero. Portable queue/fault tests and an order-sensitive source audit added. [R30H-15 details](docs/R30H_15_REMOTE_STOP_TX_PRIORITY.md). Physical R30H-10 **12 NOT_RUN + 6 BLOCKED** and R30H-12 **7 NOT_RUN + 5 BLOCKED**, all unsigned; hardware/RF/actuator/upload locks unchanged; Boat VER014 / Remote VER028 frozen.

> **R30H-14 (2026-11-23) — nRF24 cross-endpoint source integration:** Boat RF24 RX is now polled **once per main radio tick** and bounded to the 3-deep hardware FIFO; `receiveFrame()` only drains its software queue. Real RF read and STOP latency **NOT MEASURED**. Host mocks cover CRC rejection, TX-in-progress isolation, 120 ms pending watchdog and consistent Boat/Remote channel 76, 250 kbps, CRC16, 32-byte frames, retries and opposing TX/RX pipe addresses. [R30H-14 report](docs/R30H_14_CROSS_ENDPOINT_RF_INTEGRATION.md). Latest source-only host CI Boat **#45131342414 PASS**, Remote **#45131342373 PASS**, inventory **#45131342360 PASS**; latest PlatformIO **#45131671825 Boat PASS / Remote PASS**. All 18 H10 and 12 R12 physical gates still unsigned; Boat VER014 / Remote VER028 unchanged.

> **CURRENT R30H-13 PlatformIO source migration (2026-11-23):** All **208 active** Boat/Remote source files covered by [repository-wide audit](docs/R30H_13_PLATFORMIO_SOURCE_MIGRATION.md); 17 exact shared protocol pair checks. Remote obsolete direct-SPI RF24 code archived, dead Remote RF24 dependency removed, Boat nRF TX 120 ms software watchdog/fail-closed transport handling added. VS Code multi-root tasks and optional local licensed LVGL font staging available. Both endpoints remain PlatformIO **compile-only**, independent physical STOP unverified; all 18 R30H-10 + 12 R30H-12 physical evidence gates unsigned. Versions Boat **014** / Remote **028** frozen. CI results must be read per last commit; target build PASS is not hardware acceptance.

> **R30H-12 source-only update (2026-11-23):** [Bridge latency and acceptance guide](Remote/docs/REMOTE_R30H_12_BRIDGE_LATENCY_AND_ACCEPTANCE.md) + [12 unsigned physical measurements](Remote/docs/REMOTE_R30H_12_PHYSICAL_MEASUREMENTS.csv). SC18IS606 write/read operations now have a **12 ms post-call software deadline**, reject late success, and never replay an ambiguous non-idempotent SPI transaction. The modeled nominal `sendFrame()` uses **1,089 I²C wire bits**, a theoretical **10.89 ms at 100 kHz / 2.723 ms at 400 kHz**, excluding any SPI, RTOS, clock stretching, queueing and Wire API blocking. **These are not measured STOP delays or a guarantee.** Remote host tests [#45127040546](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45127040546) PASS; fresh target builds after this change are unconfirmed at this document edit. The 18 original physical gates remain 12 NOT_RUN + 6 BLOCKED; **the 12 new probes have no physical PASS**. No nRF CE fail-LOW hardware signoff, upload or actuator authorization. Boat VER014 / Remote VER028 unchanged.

> **CURRENT DEVELOPMENT HANDOFF — R30H-11, 2026-11-23:** The Remote nRF24 candidate transport now uses a **separate, ready-made NXP SC18IS606-EVB I²C→SPI bridge** rather than Waveshare direct GPIO or its TF SPI. On the NXP evaluation board: I²C 7-bit candidate `0x28` (EVB 8-bit write `0x50`); `JP1-7 SS0 → nRF CSN`, `MCP23017@0x21 GPA0 → nRF CE`; **JP1-5 SS2 stays UNCONNECTED (reset-state SPI select may drive HIGH); JP1-6 SS1 is already connected to an onboard EEPROM and must NOT be used for CE**. EVB JP1-2/3/4 carries independent SCK/MOSI/MISO. Waveshare `GPIO11/12/13` remain TF-only; no SD/nRF SPI sharing or CS arbitration is needed **if physically wired as specified**. Existing `GPIO6` SD dummy-CS and `GPIO15` RS485 are untouched. No new ESP32 GPIO or additional MCU is needed. MCP23017 GPA0 must be initialized LOW before output mode, with a real pull-down; its retained latch during MCU reset and behavior on I²C loss still require physical supervision and acceptance. This is a **software candidate and not a hardware part approval**. `REMOTE_NRF_PIN_AUDIT_SIGNED=0`, `REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED=0`, `REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED=0`; RF, flashing and actuator outputs stay denied; Boat `VER014` / Remote `VER028` unchanged. [Detailed bridge plan](Remote/docs/REMOTE_R30H_11_ISOLATED_NRF_SPI_BRIDGE.md).

> **CURRENT R30H-10 PROJECT STATE — 2026-11-22 (GPIO map and continuation synchronization)**
>
> **R30H-10 GPIO decision:** [Boat candidate map](Boat/docs/BOAT_R30H_10_PIN_MAP.csv) retains GNSS UART 4/5, I²C 7/8, nRF CSN9/CE10/SPI11–13/IRQ21, E22 6/14–18, and blocked motor PWM39/40; GPIO1/2/41/42 are safety-reserved. [Remote resource map](Remote/docs/REMOTE_R30H_10_PIN_MAP.csv) retains I²C8/9, TF SPI11–13, external ADS1115 0x4A/MCP23017 0x20+0x21/SC16IS750 0x48. **Remote nRF CE/CSN are now deliberately `-1/-1`, not GPIO6/15** (SD dummy-CS and onboard RS485 conflicts); shared TF/CH422G EXIO4 SPI arbitration remains unresolved. No validated PPS pin on either endpoint (`-1`). [Engineering report](docs/R30H_10_GPIO_RESOURCE_EVALUATION.md).
>
> **Checked development evidence (2026-11-22):** Boat and Remote portable host tests passed on GPIO commit `bb8daa2` ([Boat #45066383547](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066383547), [Remote #45066383423](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066383423)); source inventory passed after guard correction `c8ff6e9` ([#45066535687](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066535687)). **Both Boat and Remote PlatformIO target jobs PASSED** in [#45066383457](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066383457) for the GPIO source revision; compilation-only, no flashing. Boat reference target passed [#45066383455](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066383455).
>
> **Physical hardware status:** 18 R30H-10 gates remain unsigned (12 `NOT_RUN`, 6 `BLOCKED`); Independent STOP/current-zero, separate solenoid pulse limiter/watchdog, nRF/E22 acceptance, real GPS/PPS measurements, motor/coil power and firmware upload are **NOT approved**.
>
> **Versions:** Boat `VER014`, Remote `VER028` remain unchanged; packaging is a development snapshot, not a signed firmware release. VS Code + PlatformIO is primary; Arduino reference CI is secondary.
>
> This banner takes precedence over historical GPIO or build wording further down.

# Flytec V080 modular bait-boat retrofit

Active development branch: `platformio-unified-r30h20` (preserved parent: `hardware-ready-dual-radio-r1`)

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

## VS Code + PlatformIO — primary development environment (R30H-10)

**From 2026-11-22, VS Code + PlatformIO is the primary IDE/build/CI path for Boat and Remote.** Both real source trees compiled in [PlatformIO GitHub Actions #45063024581](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024581). Arduino CLI is retained as a **secondary compatibility regression**; its older ESP32 Arduino core is not the primary development toolchain.

- [Primary PlatformIO workflow and Windows commands](docs/PLATFORMIO_PRIMARY_WORKFLOW_R30H10.md), [multi-folder VS Code workspace](Flytec_V080.code-workspace).
- [Boat PlatformIO configuration](Boat/platformio.ini): ESP32-S3-DevKitC-1 N32R16V, 32 MB OPI flash / 16 MB OPI PSRAM.
- [Remote PlatformIO configuration](Remote/platformio.ini): Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 16 MB QIO flash / OPI PSRAM, LVGL 9.6.0.
- [Primary compile CI](.github/workflows/platformio-migration-build.yml), [historical parallel-migration troubleshooting guide](docs/PLATFORMIO_VSCODE_MIGRATION_R1.md).
- [R30H-10 physical test protocol](Boat/docs/BOAT_R30H_10_HARDWARE_ACCEPTANCE.md) and [18 unsigned physical acceptance gates](Boat/docs/BOAT_R30H_10_PHYSICAL_GATES.csv).

Build from a VS Code / PlatformIO terminal (compile-only):

```powershell
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

Boat uses one guarded PlatformIO entry adapter for its nested sketch; Remote compiles its root `.ino` **once** and does not use a second adapter. `espressif32 @ 8.0.0` includes Arduino-ESP32 core 3.3.12. The former Arduino CLI core 3.0.2 remains a comparison for regressions. 

**Strict hardware separation:** both PlatformIO upload actions remain blocked; source RF/marker flags stay 0/-1; Remote GPIO6 (nRF CE / SD dummy-CS) and GPIO15 (nRF CSN / RS485) remain unsigned physical conflicts. Never connect MDD20A VM or EKM014 VH for RF-only tests. Real RF, GNSS/PPS, GPIO levels and the **independent physical STOP/current-zero** hardware have **not passed physical acceptance**. Boat VER014 / Remote VER028 unchanged. No successful compile unlocks actuators or release firmware.

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


## R30H actual RF latency acceptance preparation

The [R30H RF measurement procedure](Boat/docs/BOAT_R30H_RF_LATENCY_BENCH_PROCEDURE.md), [22-case unsigned test matrix](Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv), [empty event CSV template](Boat/docs/BOAT_R30H_RF_EVENTS_TEMPLATE.csv), and [host-only timestamp analyzer](Boat/tools/analyze_r30h_rf_events.py) are ready. **No real-radio, STOP-current, or RTH timing was measured.** The nRF/E22 radios and independent common-3S physical STOP/interlock gates still need purchased-board signoff; motor VM and solenoid VH remain disconnected. Firmware release markers stay Boat VER014 and Remote VER028. See [continuation checkpoint](docs/CONTINUATION_CHECKPOINT_VER001.md) for remaining gates.


## R30H-1 — actual radios: SC16IS750 FIFO repair and unsigned RF commissioning

- [R30H-1 inspection and bridge-fix report](Boat/docs/BOAT_R30H_1_RADIO_BRINGUP_AND_BRIDGE_FIX.md) and [22 physical RF signoffs (NOT_RUN)](Boat/docs/BOAT_R30H_1_RF_MODULE_SIGNOFF.csv). [R30H STOP/10s RTH trials](Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv) remain 22 NOT_RUN.
- **Fixed a real source defect:** Remote `SC16IS750Bridge::writeFifo()` previously computed `64 - TXLVL` despite NXP TXLVL reporting *free slots*, making an empty FIFO appear full. Corrected Remote TX writes and `flush()`, added pure 64-byte FIFO logic, host C++ regression and static negative tests.
- **Fail-closed startup:** Remote primary nRF GPIO6/15 and SC16IS750/E22 now require signed PCB, carrier, RF-interop and airtime flags; Boat E22 adds a signed airtime gate. All new signoff flags remain `0`. E22 `EST_AIRTIME_MS=25` is explicitly *unmeasured*; do not treat 1% software counter as legal compliance. Existing R30A physical-actuator locks remain false.
- [Boat host R30H-1 #45051432923](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45051432923) and [Remote host R30H-1 #45051429416](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45051429416) **SUCCESS** for source/CI regressions. Rebuilds after final header comments must be inspected separately; no physical pairing, scope measurements, STOP output time or real RTH was verified.
- **Next:** Inspect purchased exact board and modem suffix revisions, scope UART TXLVL/9600-8N1 and TTL voltages, verify regulatory 868MHz airtime and profile readback; only then attempt RF-only authenticated pairing with all MDD20A VM/EKM014 VH disconnected. Boat VER014 and Remote VER028 unchanged; sonar deferred.

## R30H-2 — Boat-local diagnostic trace and simulated dual-radio timing (2026-11-22)

Added [R30H-2 Boat-local trace & simulation](Boat/docs/BOAT_R30H_2_LOCAL_TRACE_AND_SIMULATION.md): bounded 32-record Boat trace for authenticated STOP reception, logical STOP intent, dual-link stale/recovered transitions and RTH entry, with source-local millis() and last accepted authenticated RX timestamp. Optional USB output is **disabled by default** and compiled only in a separately staged, **non-actuating, radio-locked** test variant. No physical motor/solenoid driver or live RF link is enabled.

New C++11 ring/timer regression and Python serial-log analyzer reject missing STOP intent, buffer overflows, invalid/early (under 10s) RTH and misleading physical-current labels. `Boat/tools/verify_r30h_radio_preflight.py` now checks the trace remains locked and no critical-path USB I/O was added. Existing 250ms primary / 6000ms backup freshness and last-authenticated-RX +10000ms conditional RTH are **unchanged**; these show a potential up-to-6s stale-E22 stop delay in software, not an RF measurement.

[Boat host run #45052442054](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45052442054) completed SUCCESS with the new R30H-2 tests; [Boat host #45052521183](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45052521183) completed SUCCESS after serial delimiter correction. A separate Boat target build with R30H2 USB trace enabled was in progress at last inspection; its result must be confirmed before declaring that optional target compile accepted.

**Physical state:** R30H-1 board/rail/antenna gates 22 NOT_RUN, R30H RF/STOP/RTH test matrix 22 NOT_RUN, R30A physical safety gates still OPEN. No real RF/STOP timings, no current-zero observation, no module purchase or physical driver commission. Boat VER014 / Remote VER028 held; sonar deferred. **Next R30H-3**: signed radio SKU/level/airtime and shared-clock RF-only bench capture, then revise unsafe backup pacing/freshness if measured evidence warrants.

## R30H-3 — software-only E22 STOP worst-case audit and Remote queue trace (2026-11-22)

**Implemented in current branch:** [R30H-3 engineering review](Boat/docs/BOAT_R30H_3_RF_STOP_EXPOSURE_REVIEW.md), [16 unsigned physical acceptance gates](Boat/docs/BOAT_R30H_3_STOP_RF_ACCEPTANCE.csv), [source-derived 1ms phase simulator](Boat/tools/simulate_r30h3_rf_timing.py) and exhaustive host tests. The Remote now has a compile-time-disabled 16-event local STOP trace (`r30h3_stop_trace.h`, `boat_radio.cpp`) distinguishing request, nRF driver-buffer admission, E22 3-second pacing deferral and E22 UART-buffer admission. This does **not** change the RF send pacing, command authorization, HMAC, or real STOP execution.

**Software-derived exposure, not measured:** E22 STOP can be locally deferred up to 3000ms by 3s pacing, with unpredictable UART/RF/Boat latency afterward; after simultaneous radio cut, the Boat's 6000ms backup freshness may keep its software watchdog from total-loss STOP for ~3.002–6.001s in the modeled heartbeat phases, plus unknown runtime delay. The 10s conditional RTH clock remains anchored to last Boat authenticated RX, not RF cut. At an entirely **hypothetical** 25ms E22 frame airtime, 1200 heartbeats/hour cost 30s of a provisional 36s per-hour software budget; 30ms hypothetical airtime leaves zero command headroom. **Actual airtime, legal EU868 duty and EIRP need physical verification.**

[Boat host #45054319954](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45054319954) passed the corrected phase-sweep regression; [Remote host #45054438088](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45054438088) passed the new Remote event-ring regression with corrected timing parser. New preflight CI additionally forbids any R30H-3 fabricated physical signoff. Latest Remote ESP32 target compilation with trace disabled is checked separately. **All R30H-3 16 physical gates NOT_RUN**, existing R30H-1 22 module gates and R30H 22 RF trials NOT_RUN. R30A common-3S independent hardware STOP/one-hot/current-zero requirements remain OPEN. Boat VER014 / Remote VER028 unchanged; sonar still deferred.

**Next R30H-4:** obtain hardware-specific module/level/antenna/airtime measurements with a common-clock logic analyzer, demonstrate actual emergency STOP delivery and total-link-loss latency with MDD20A VM/EKM014 VH disconnected, determine an independently approved maximum STOP limit, then redesign any unacceptable 6s/3s exposures before physical-actuator commissioning.

## R30H-4 — common-clock RF-only capture protocol; physical data still NOT_RUN (2026-11-22)

- [External clock bench procedure](Boat/docs/BOAT_R30H_4_EXTERNAL_CLOCK_RF_BENCH.md) defines the safe RF-only capture order, including physical disconnection of MDD20A VM / EKM014 VH (the active **single common 3S battery bus** remains unchanged). [Empty capture CSV](Boat/docs/BOAT_R30H_4_LOGIC_CAPTURE_TEMPLATE.csv) and [UNSPECIFIED manifest](Boat/docs/BOAT_R30H_4_CAPTURE_MANIFEST_TEMPLATE.json) are new **templates, not measured evidence**. [10-channel testpoint worksheet](Boat/docs/BOAT_R30H_4_RF_ONLY_PROBE_CROSSWALK.csv) has **10 NOT_READY** mappings; in particular authenticated Boat STOP/RTH electrical marker GPIOs **do not exist yet**.
- [External analyzer read-only validator](Boat/tools/analyze_r30h4_common_clock_capture.py) enforces source identity, capture SHA256, one physical clock, microsecond timestamp ordering and label/edge polarity, and refuses to equate nRF CSN/IRQ or E22 UART with authenticated STOP, physical zero-current or true end-to-end radio latency. A synthetic or even apparently complete hardware trace gets **REVIEW_REQUIRED exit 4**, never physical PASS. Empty evidence gets NOT_RUN exit 3; malformed captures exit 2.
- [R30H-4 hostile host regressions](Boat/tests/test_r30h4_common_clock_capture.py) and original [R30H safety preflight](Boat/tools/verify_r30h_radio_preflight.py) reject fabricated external captures, signed probe/manifest templates, inconsistent clocks, 9,999,999us RTH, and missing disconnection evidence. [Boat host #45055157836](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45055157836): **SUCCESS** for the initial source/preflight integration; the subsequent additional negative-regression run must be verified separately.
- **No physical milestones changed:** R30H original 22 trial scenarios NOT_RUN; R30H-1 22 module gates NOT_RUN; R30H-3 16 RF/STOP acceptance gates NOT_RUN; R30A 24 physical readiness gates OPEN plus earlier 81 NOT_RUN. Boat/Remote RF pin and duty flags remain 0, STOP 3-second E22 queue and 6-second backup freshness remain unresolved hazards, and independent STOP/current-zero hardware remains uncommissioned. Boat VER014 / Remote VER028 frozen, sonar deferred.
- **Next R30H-5:** prepare a *non-actuating* verified debug-marker allocation plan for authenticated STOP/RTH before any real E2E timing claims; actual measurements demand purchased modules, safe antennas/RF compliance, reviewed voltage levels, and one common external logic-analyzer clock.

## R30H-5/6 — GPS UTC on both ESP32-S3 wall clocks and marker readiness (2026-11-22)

[GPS UTC and R30H-6 report](BOAT_R30H_6_GPS_UTC_AND_MARKER_READINESS.md): the Boat reads verified GNSS UTC from M100 PRO (RMC/GGA/GSA or UBX NAV-PVT with valid fix/time). After two consistent samples the Boat disciplines its CLOCK_REALTIME. Boat sends versioned, HMAC-authenticated subtype-9 UTC via priority-limited nRF24; Remote accepts only valid session/sequence/primary frames, requires two coherent samples and disciplines its CLOCK_REALTIME via settimeofday. No PPS is commissioned; timing includes unmeasured UART/RF delays.

The two monotonic millis clocks remain independent and untouched: STOP, link freshness and last-authenticated-RX +10000ms RTH do not use civil UTC. GPS UTC is COARSE display/log time, never shared physical microsecond STOP evidence. R30H-6 adds shared fail-closed clock quality/marker policy, last GPS UTC sample age and Remote boatRadio_utcQuality() which reports INVALID without fresh primary. No marker GPIO selected or driven; seven actual board/IO/common-clock safety conditions are still unsigned.

[Boat host #45056406215](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056406215) SUCCESS; [Remote host #45056411520](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056411520) SUCCESS for source-level clock quality and marker isolation. Real GNSS/radio/oscillator measurements remain NOT_RUN. R30H original 22 RF checks and R30H-1 22 board signoffs NOT_RUN, R30H-3 16 safety acceptance NOT_RUN, R30H-4 ten probe mappings NOT_READY, R30A independent hardware STOP/current-zero OPEN. Boat VER014 and Remote VER028 unchanged. Next R30H-7: physical UART/PPS review and RF-only external-clock timing after purchased module inspection.

## R30H-5 — GNSS UTC RTC discipline on Boat and Remote (2026-11-22)

**Source implemented; physical UTC/PPS/RF accuracy NOT_RUN.** [R30H-5 GPS/UTC engineering report](Boat/docs/BOAT_R30H_5_GPS_UTC_CLOCK_DISCIPLINE.md) defines the new UTC source authority: Boat HGLRC M100 PRO UBX NAV-PVT valid date/time/tAcc/nano, NMEA RMC date fallback, Gregorian validation and fix freshness, and `settimeofday()` of Boat ESP32 system CLOCK_REALTIME (only under a commissioned tracker sensor config). Remote receives a dedicated **authenticated, sequence-checked, nRF-only telemetry subtype 9** with UNIX UTC seconds/milliseconds and applies RTC update after two consistent observations, at most once/minute. Shared wire code: [Boat](Boat/source/radio_utc_time_v1.h) and [Remote](Remote/source/radio_utc_time_v1.h), and read-only `boatRadio_utcNow()` for Remote UI/logs.

**Safety isolation:** UTC is informational; time packets never refresh radio/control freshness, reset STOP, change link lease, count toward 10s RTH, authorize manual/hopper/hook or cause E22 airtime. The existing 250ms/6000ms heartbeat freshness and authenticated-RX +10000ms conditional RTH remain **monotonic `millis()`**. No physical radio or actuator HAL enabled. Without actual inspected GNSS modules, UTC is unavailable and no board was synchronized on the bench. UART serial readout + one-way RF transfer **do not provide PPS/microsecond clock alignment**, so the R30H-4 external common-clock analyzer remains mandatory for physical STOP timing.

[Boat GNSS UTC C++ tests](Boat/tests/r30h5_gnss_utc_clock_test.cpp), [Remote clock tests](Remote/tests/r30h5_utc_clock_test.cpp), [fail-closed UTC source preflight](Boat/tools/verify_r30h5_gps_time_isolation.py) and hostile tests cover date validity, leaps, invalid NAV-PVT, stale time, time regressions, wraparound and radio HMAC/sequence isolation. [Boat host CI #45056333757](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056333757) and [Remote host CI #45056333751](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056333751) both **SUCCESS**. [Boat ESP32 target #45055961006](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45055961006) **SUCCESS**; latest Remote target compile must be checked individually and is not claimed PASS here.

**Next R30H-6:** non-actuating, read-only [UTC quality/marker readiness contract](Boat/source/r30h6_utc_marker_policy.h) — no debug GPIO, PPS wire, physical current-zero or external-marker signoff has been installed. R30H-1 22 module signoffs, R30H 22 RF trials, R30H-3 16 physical STOP gates are NOT_RUN; R30H-4 ten mappings NOT_READY; R30A 24 safety gates OPEN. Boat VER014 / Remote VER028 frozen; sonar undecided.

## R30H-7 — GPS UTC consistency and physical PPS signoff gates (2026-11-22)

[Engineering review](Boat/docs/BOAT_R30H_7_GNSS_CLOCK_RECOVERY_REVIEW.md) and [12 unsigned real GNSS/PPS clock-offset tests](Boat/docs/BOAT_R30H_7_GNSS_PPS_RF_CLOCK_SIGNOFF.csv) are committed. Boat NMEA UTC now requires **RMC date/UTC second equal to the valid GGA fix second**; unequal epochs refuse the wall-clock discipline even when a previous position remains fresh. New host regression verifies mismatch is denied and matching GGA restores qualification. Remote `boatRadio_utcNow()` now also refuses a stale authenticated nRF primary link, independent of any previously set RTC. Both ESP32 still synchronize **only CLOCK_REALTIME from GNSS**, not millis()/STOP/RTH timebase, and neither is PPS calibrated. There is no deployed GNSS or real RF timing measurement.

Shared preflight and hostile tests verify epoch gating, primary freshness, identical UTC wire codec and no fabricated PPS data. [Remote host #45056804347](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056804347) **SUCCESS** after static guard repair. [Boat host #45056804415](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056804415) was in progress at last inspection; do not claim final compile/test success until checked.

**Physical states unchanged:** R30H-7 12 GNSS/PPS gates NOT_RUN; R30H-4 10 probe mappings NOT_READY; R30H-3 16 STOP gates NOT_RUN; R30H-1 22 module signoffs NOT_RUN, original R30H 22 RF scenarios NOT_RUN; R30A independent current-zero safety gates OPEN. Official Boat VER014 / Remote VER028 frozen; no actuator authorization. **Next R30H-8:** inspect real GNSS UART mode/PPS pin/logic level, commission RF-only pairing and measure UTC phase / STOP latency against a single independent external clock before considering precise timestamp calibration.

## R30H-7 — UTC header + matched-epoch recovery and physical-clock blockers (2026-11-22)

**Implemented:** Remote `boat_ui.cpp` now shows **GPS UTC hh:mm:ss ~** from authenticated Boat-origin radio UTC, with **GPS UTC --:--:--** on no fresh/authenticated nRF time. Formatting is in [R30H-7 UTC UI helper](Remote/source/r30h7_utc_clock_ui.h), [portable C++ regression](Remote/tests/r30h7_utc_clock_ui_test.cpp), and the existing `boatRadio_utcQuality()` path. This label is read-only, not connected to STOP, battery interlocks, manual control, RTH or timestamp certification. The tilde means coarse/unmeasured UART+RF latency; the displayed timezone is explicitly **UTC**, not Bulgaria EET/EEST. The optional Boat NMEA `TrackerGnss::utcSnapshot()` must reject GGA/RMC differing time epochs, a guard verified by the R30H-7 source-hostile tests. [Engineering review](Boat/docs/BOAT_R30H_7_GNSS_CLOCK_RECOVERY_REVIEW.md).

**Verified:** [Remote host #45056813182](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056813182) **SUCCESS** with the new UTC header formatter tests. [Boat host #45056804415](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45056804415) **SUCCESS** for the matched-epoch guard and other host regressions. The new **Remote target LVGL compile remains unconfirmed**; host C++ is not a substitute for real Waveshare display/battery/radio tests.

**Unsigned physical evidence:** [R30H-7 12-point GNSS/PPS/clock accuracy acceptance matrix](Boat/docs/BOAT_R30H_7_GNSS_PPS_RF_CLOCK_SIGNOFF.csv) is still **12 NOT_RUN**. No actual GNSS baud or antenna/board variant confirmed, no audited PPS pin, no common physical clock or measured RF transfer delay. Source-implemented RTC discipline updates only `CLOCK_REALTIME` and does **not** synchronize `millis()`; all STOP, nRF/E22 250/6000ms freshness and conditional 10s RTH remain independent monotonic safety paths. R30H-4 external analyzer and unsigned electrical STOP/zero-current hardware gates remain mandatory. Versions Boat VER014 / Remote VER028 remain unchanged; sonar deferred.

**Next R30H-8:** physically inspect/obtain M100 PRO and both ESP32 boards, verify NMEA vs UBX output and any accessible PPS, record legal radio and voltage checks, then use one external-clock RF-only bench setup to quantify real Boat/Remote UTC skew and STOP latency, with VM/VH physically disconnected. Do not enable propulsion or certify STOP based only on GNSS UART UTC timestamps.

## R30H-8 — dual ESP GPS/PPS common-clock analysis preparation (2026-11-22)

[Engineering procedure](Boat/docs/BOAT_R30H_8_SHARED_CLOCK_ALIGNMENT_READINESS.md) records a **software-only** timestamp/offset estimation tool (Python), [empty PPS/instrument CSV](Boat/docs/BOAT_R30H_8_SHARED_CLOCK_CAPTURE_TEMPLATE.csv), [unsigned capture manifest](Boat/docs/BOAT_R30H_8_SHARED_CLOCK_MANIFEST_TEMPLATE.json) and [8 additional physical clock signoff gates](Boat/docs/BOAT_R30H_8_CLOCK_ALIGNMENT_GATES.csv), all **NOT_RUN**. Given one independently verified GNSS PPS epoch and electrically captured Boat + Remote CLOCK_REALTIME sample edges on the same analyzer clock, the tool computes candidate device offsets/drift. It rejects independent USB/millis timestamps, missing pulses, invalid or replayed PPS epochs, mixed data provenance and fake hardware evidence. It **never produces physical PASS** (blank=exit 3, invalid=2, even reviewed-looking or synthetic records=4).

[Boat host R30H-8 #45057367348](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45057367348), [Boat unsigned physical preflight #45057408181](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45057408181), and [Remote shared-preflight #45057408139](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45057408139): **SUCCESS (host-only)**. No hardware GPS UART/PPS is measured or wired; PPS presence on the actual M100 PRO/carrier is still unknown; no debug GPIO assignments or actuator outputs were added. Existing Boat GPS → authenticated nRF subtype-9 → Remote GPS UTC only disciplines separate CLOCK_REALTIME values for coarse UI/logging, and **does not synchronize monotonic millis()** used for physical STOP/RTH/heartbeat ages. R30H-7 12 PPS gates, R30H-4 10 probes, R30H-3 16 STOP gates, R30H-1 22 module gates, R30H 22 RF trials and R30A hardware safety remain unverified. Boat VER014 / Remote VER028 unchanged; sonar deferred. **Next R30H-9:** conditional debug-marker/harness specification after actual board/pin/voltage audit; only then RF-only common-clock characterization.

## R30H-9 — non-actuating GPIO marker preparedness (2026-11-22)

[R30H-9 hardware pin/GPIO readiness](Boat/docs/BOAT_R30H_9_DIAGNOSTIC_GPIO_READINESS.md), [10 unsigned analyzer-probe routes](Boat/docs/BOAT_R30H_9_PROBE_CROSSWALK.csv) and [12 physical signoffs, all NOT_RUN](Boat/docs/BOAT_R30H_9_PHYSICAL_SIGNOFF.csv) are now committed. Source-only compile guards pin diagnostic output at **GPIO=-1 / OUTPUT_SIGNED=0** on both Boat ESP32-S3-DevKitC-1-N32R16V and Remote Waveshare Standard SKU25948. Boat GPIO1/2/41/42 stay reserved for independent safety hardware; Remote LCD/SD/touch/CAN/RS485 use the remaining practical pins. In particular, Remote provisional nRF CSN GPIO15 is onboard RS485 TXD; physical allocation not signed. Mirrored C++11 read-only event structures and source mutation preflight *do not create any GPIO writes or runtime logging* and cannot certify STOP.

[Boat R30H-9 host CI #45059841295](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45059841295) **SUCCESS**; [Remote R30H-9 host CI #45059846215](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45059846215) **SUCCESS**. Real bench RF/PPS/STOP/current-zero NOT_RUN; the single common Boat 3S4P VM/VH bus still has both loads physically uncommissioned. No independent hardware STOP has been accepted, no diagnostic or RTH interval has a measured physical bound. Boat VER014 / Remote VER028 unchanged, sonar deferred.

The separate experimental PlatformIO Remote compile remains a failure at the older `float -> LVGL precise integer` initializer in `boat_ui.cpp`; this is not an R30H-9 code failure. Next R30H-10 requires physical inspection of boards/cables and safe RF-only one-analyzer-clock capture, or else keep end-to-end authenticated STOP timing **NOT_INSTRUMENTED**.


## R30H-10 source optimization and verified dual-build migration — 2026-11-22

See [full source cleanup and build evidence](docs/SOURCE_CLEANUP_R30H_10_R1.md).

- **Both PlatformIO targets: SUCCESS** — [workflow #45063024581](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024581), Boat N32R16V and Remote Waveshare Standard with ESP32 Arduino core 3.3.12, compilation-only, no upload.
- **Both Arduino reference targets: SUCCESS** on the archived-source revision — [Boat #45061849197](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849197) and [Remote #45061849238](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849238). Latest Remote Arduino target after adapter archival also **SUCCESS**: [#45063024540](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024540).
- **Both host workflows: SUCCESS** after the Remote entrypoint correction — [Remote #45063024532](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024532); Boat host [#45061849147](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849147); static active-source inventory [#45063024449](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024449) SUCCESS.
- **197 active-source files, 56 sketch/C++ compile roots, one intentionally preserved reference/example header**. Six retired unreferenced or duplicated files were preserved in [history](https://github.com/vankatad4-hue/BaitBoat-Update/tree/hardware-ready-dual-radio-r1/docs/archive/2026-11-22/source_cleanup_r30h10) and removed from live sources. This is static review and safe cleanup, not complete runtime performance optimization.
- Remote LVGL 9.6 sonar DEMO point coordinates repaired (float-to-integer), duplicate SD status corrected with 4-case host test. Retired Heltec SX1262 and unused older configuration files archived. Boat uses selected nRF24/E22 UART and INA231 BUS-only; sonar still deferred.
- **Safety/physical acceptance remains OPEN**: no real radio STOP/RTH/PPS accuracy or independent STOP/current-zero acceptance, no motor/solenoid power approval. Remote GPIO6 (SD dummy CS/nRF CE) and GPIO15 (RS485/nRF CSN) remain electrically conflicted and unsigned. Keep Boat VER014 / Remote VER028 and reference Arduino build; passing PIO is a viable parallel compilation path, not firmware promotion.

