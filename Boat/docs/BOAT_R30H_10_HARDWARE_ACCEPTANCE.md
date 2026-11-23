> **R30H-12 source-only update (2026-11-23):** [Bridge latency and acceptance guide](../../Remote/docs/REMOTE_R30H_12_BRIDGE_LATENCY_AND_ACCEPTANCE.md) + [12 unsigned physical measurements](../../Remote/docs/REMOTE_R30H_12_PHYSICAL_MEASUREMENTS.csv). SC18IS606 write/read operations now have a **12 ms post-call software deadline**, reject late success, and never replay an ambiguous non-idempotent SPI transaction. The modeled nominal `sendFrame()` uses **1,089 I²C wire bits**, a theoretical **10.89 ms at 100 kHz / 2.723 ms at 400 kHz**, excluding any SPI, RTOS, clock stretching, queueing and Wire API blocking. **These are not measured STOP delays or a guarantee.** Remote host tests [#45127040546](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45127040546) PASS; fresh target builds after this change are unconfirmed at this document edit. The 18 original physical gates remain 12 NOT_RUN + 6 BLOCKED; **the 12 new probes have no physical PASS**. No nRF CE fail-LOW hardware signoff, upload or actuator authorization. Boat VER014 / Remote VER028 unchanged.

# R30H-10 — physical GPIO, dual-radio, GNSS/PPS and independent STOP acceptance

**Status: NOT_RUN / BLOCKED BY HARDWARE PROCUREMENT AND SAFETY SIGNOFF.** 2026-11-22. Boat VER014 / Remote VER028 frozen. This procedure is a test *plan*, not test results, not an actuator commissioning approval. Software development uses VS Code + PlatformIO as primary; its verified builds do not clear physical gates.

## Preconditions — evidence is required, no guessed approvals

- Identify and photograph actual Boat **ESP32-S3-DevKitC-1 N32R16V** PCB/module revision and Remote **Waveshare 4.3 Standard SKU25948** revision, including connector pin-1 orientation and module markings.
- Compare actual header/pad continuity against [Boat R23 wiring](BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md), [R30H-9 resource review](BOAT_R30H_9_DIAGNOSTIC_GPIO_READINESS.md), and Remote Waveshare schematic. Record measured idle voltages, input tolerances, ground references and conflict disposition. A schematic-only review does **not** sign the hardware gate.
- **Resolve the Remote GPIO6 collision** (proposed nRF CE / SD dummy-CS / Sensor AD) and **GPIO15 conflict** (nRF CSN / onboard RS485 TXD) through an electrically reviewed design **before** wiring or enabling nRF. No automatic substitute GPIO; no change to `REMOTE_NRF_PIN_AUDIT_SIGNED=0` until independent evidence is reviewed.
- Maintain Boat GPIO1/2/41/42 reserved for physical STOP/safety feedback. Do not repurpose these or LCD/TF/USB/boot pins as debug markers. Both R30H-9 marker GPIOs stay -1 and signoff flags stay 0.
- Inspect exact nRF24 PA/LNA and E22-900T22S module/carrier, supply rails, TTL voltage levels, common ground and suitable antennas/RF load. Verify local RF band/power/airtime authorization prior to intentional radio transmission.
- **For RF-only tests disconnect both MDD20A VM and EKM014 VH from the shared 3S power bus**, separately verify de-energized load terminals, and leave motors/coils disconnected. Use current-limited, safely isolated logic power, and prevent USB/external 5V backfeed.

## Ordered measurements

### A. GPIO and passive logic

Record continuity and waveforms for the actual connector/pad of each chosen nRF SPI (SCK/MISO/MOSI/CSN/CE/IRQ), E22 UART TX/RX/M0/M1/AUX, GNSS UART, I2C and diagnostic STOP button input. Measure voltage high/low, reset/boot level and any shared-peripheral contention. Record any input exposure to 5V. No active GPIO marker, output or wiring change can be enabled merely to make acquisition easier. Use [R30H-9 ten-probe crosswalk](BOAT_R30H_9_PROBE_CROSSWALK.csv) and its [twelve unsigned gates](BOAT_R30H_9_PHYSICAL_SIGNOFF.csv).

### B. Real nRF24 + E22, RF-only

With antenna/power/compliance signoffs, collect timestamped **electrical** SPI/IRQ and UART edges for each radio, primary/backup selection, loss of one link and loss of both. Use **one external calibrated logic-analyzer clock** for all compared edges. Record at least **30 STOP-button trials across the tested nRF/E22 pacing states**, sequence labels, raw capture plus SHA256 and a witness. An SPI or UART buffer-admission edge **does not** establish a transmitted over-air packet or an authenticated STOP accepted by the Boat. Without an independently measured Boat STOP-accept event, report **AUTHENTICATED_STOP_E2E=NOT_INSTRUMENTED**, not PASS.

### C. GPS/GNSS UTC, PPS and synchronization

Confirm whether the purchased **HGLRC M100 PRO** exposes a genuine PPS signal on the available carrier/pad; record physical source, polarity, voltage, signal integrity and the actual UTC second associated with that PPS. Log live GNSS fix validity and UTC via UART independently. Capture PPS, a physically observed radio signal and a reference analyzer clock **in one instrument's timebase**. If no genuine exported PPS exists, set `PPS=NOT_AVAILABLE` and keep the phase/accuracy gate **NOT_RUN**. GNSS-derived UTC for wall-clock display is not an ESP32 microsecond clock discipline or a substitute for PPS. Apply the separate R30H-7 and R30H-8 physical checklists and read-only analysis tools; do not invent sub-millisecond accuracy.

### D. STOP electrical proof — **separate authorization**, not part of RF-only tests

Do **not** connect motors/solenoids merely to perform R30H-10 radio captures. First establish independent hardware STOP architecture and physical acceptance with an authorized reviewer and safe **dummy loads**. Verify removal/inhibition of actuator energy even with ESP firmware stalled/reset, broken I2C or radio link, and when a software STOP is delayed; measure **motor driver/coil current decaying to zero** and output voltage under a calibrated instrument. Independently prove the solenoid one-at-a-time limit and non-retriggerable pulse cutoff under a fault. Shared 3S BMS and software watchdog alone **do not** prove an independent STOP. Record cutoff hardware and fail-closed wiring; without this evidence `INDEPENDENT_STOP=BLOCKED`, even if both PlatformIO targets and all RF-only cases pass.

### E. Conditional RTH / link-loss behavior

Only after separate independent STOP and navigation safety acceptance, inspect both links absent, immediate STOP, the **10-second** conditional RTH decision and its validity requirements (active eligible mission, matching CRC-valid stored Home, valid GNSS/heading/battery/physical interlocks). Never enable physical propulsion to demonstrate this while STOP/current-zero proof is open. A successful software-only RTH simulation is **not** evidence of physical motion or return-home accuracy.

## Required raw evidence for every physical trial

Capture-ID; UTC start and GNSS UTC source; instrument model/serial, sample rate and calibration; one clock ID; endpoint firmware commit/build environment; actual board/carrier revisions; net name and **measured** connector contact; probe voltage/attenuation/ground; power rail and load disconnect proof; test stimulus; observed electrical edge timestamps; raw waveform filename + SHA256; limitations (RF edge vs authenticated command); pass criterion; independent engineer/witness initials and review date. Blank fields are **not** PASS.

The [R30H-10 CI source-lock preflight](../tools/verify_r30h10_physical_lock.py) rejects premature completed physical-status claims, changed RF/marker locks or removed upload denial. It does not simulate a logic analyzer or validate current-zero.

Use [R30H-10 unsigned gate register](BOAT_R30H_10_PHYSICAL_GATES.csv), not a CI build log, as the physical acceptance record. Never replace an earlier unsigned worksheet with assumed measurements.

## Safe exit criteria

1. Only when all board/level/RF physical checks are signed may the **non-actuating RF-only** procedure be considered complete.
2. A safe GPIO marker requires a separate approved free output, validated latency, physical continuity, risk review and an isolated diagnostic firmware build; currently **none exists**.
3. R30H-10 is **not completed** until the independent STOP/current-zero/solenoid-limiter evidence passes independently, with physical hardware. Until then `ACTUATOR_POWER=DISCONNECTED`, `UPLOAD=DENIED`, `RF_AUDITS=0`, `PHYSICAL_ACCEPTANCE=OPEN`, and `Boat VER014 / Remote VER028` are unchanged.
4. Any actual wiring disagreement, voltage conflict, absent PPS, unsafe antenna/probe, or unexpected powered output: abort, de-energize, annotate BLOCKED/FAIL and require reviewed rework.

## R30H-10 passive GPIO pin-map update (source only)
Authoritative machine-readable [Boat pin map](BOAT_R30H_10_PIN_MAP.csv) and [Remote pin map](../../Remote/docs/REMOTE_R30H_10_PIN_MAP.csv), plus [engineering evaluation](../../docs/R30H_10_GPIO_RESOURCE_EVALUATION.md). The live Remote nRF CE/CSN proposals were retired to `-1/-1`; rejected GPIO6/15 remain documented. Remote shared TF SPI/CH422G EXIO4 CS needs arbitration before any nRF RF-only trial. GNSS PPS remains `-1` without a proved exported pin. The Boat candidate assignments remain unchanged; independent STOP and all actuator outputs remain locked.

## R30H-11 Remote isolated nRF bridge candidate — physical gate still BLOCKED
Previous Remote direct CE GPIO6/CSN GPIO15 proposal is retired. A **prebuilt NXP SC18IS606-EVB** is now the selected *software transport candidate* for Remote nRF24: Remote I²C8/9 to EVB `JP2`, then separate bridge SPI `JP1` to radio. `JP1-7 SS0` automatically drives CSN; `MCP23017@0x21 GPA0` (existing separate I²C expander) is the candidate CE output, initialized latch LOW before output mode with readback and external pull-down. `JP1-5 SS2` must remain UNCONNECTED because its reset SPI chip-select may be driven HIGH, and `JP1-6 SS1` is the EVB EEPROM. Even MCP GPA0 plus pull-down does not guarantee CE LOW if its output stays driven HIGH during MCU crash/reset or I²C loss; hardware supervision and physical acceptance remain BLOCKED. This removes proposed contention with Remote SPI11/12/13 TF card; TF actual CS remains CH422G EXIO4 and GPIO6 stays dummy CS. **No Remote ESP GPIO is reassigned**. These are proposed wiring nets, not measured headers. R30H-10 gates H10-02/03/04/05/07 and antenna/I²C timing and independent STOP gates still require actual test and signoff. Do not mark any physical gate PASS from a source-only driver or host test. See [Remote bridge test plan](../../Remote/docs/REMOTE_R30H_11_ISOLATED_NRF_SPI_BRIDGE.md).
