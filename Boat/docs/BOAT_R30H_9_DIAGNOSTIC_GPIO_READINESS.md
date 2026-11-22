# R30H-9 — safe diagnostic event markers: pin-resource and RF-only instrument plan

**Stage:** software implementation/board-specific allocation review — **NOT physical GPIO commissioning**. Updated 2026-11-22. All 10 probes and 12 physical acceptance gates remain unsigned. Boat VER014, Remote VER028 remain frozen.

## Exact hardware targets; manufacturer references

**Boat:** selected **ESP32-S3-DevKitC-1-N32R16V** with ESP32-S3-WROOM-2-N32R16V, 32MB OPI flash/16MB PSRAM. Candidate header labels from [Espressif official DevKitC-1 v1.1 guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html) and [R20 J1/J3 resource crosswalk](BOAT_R20_GPIO_RESOURCE_CROSSWALK.csv). The actual purchased DevKitC-1 PCB version, module shield label, soldered attachments, pin continuity, and 1.8V 47/48 restrictions are **not yet verified in person**.

**Remote:** **Waveshare ESP32-S3-Touch-LCD-4.3 Standard**, **SKU 25948**, **not** 4.3B/4.3C, 16MB flash and 8MB PSRAM, ST7262 800×480 LCD, GT911 touch, CH422G, TF-card and CAN/RS485. See [official Waveshare 4.3 Standard pin map](https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3) and [project Waveshare resource audit](../../Remote/New_Handheld_Remote/PCB/WAVESHARE_GPIO_RESOURCE_AUDIT_R1.md). *GPIO15 is already routed to onboard RS485 TXD*, despite the provisional Remote nRF24 CSN software proposal. GPIO6 is Sensor AD; GPIO11/12/13 are shared TF-card SPI; GPIO8/9 are the touch/I2C/CH422G bus. LCD consumes GPIO0/1/2/3/5/7/10/14/17/18/21/38/39/40/41/42/45/46/47/48. Native USB uses 19/20. **Do not infer any free diagnostic GPIO or reassign GPIO15 without reviewing board schematic and disabling/routing the RS485 circuit**. The current radio initialization guard remains OFF.

### Pin-resource disposition

| Endpoint | Candidate physical outputs | Status/reason |
|---|---|---|
| Boat ESP32 | GPIO1 (J3-4), GPIO2 (J3-5), GPIO41 (J3-7), GPIO42 (J3-6) | **RESERVED FOR FUTURE INDEPENDENT PHYSICAL STOP/SAFETY FEEDBACK; not available for debug marker** |
| Boat ESP32 | GPIO4–18, 21, 39/40 (where individually listed in R23) | Already designated GNSS/I2C/nRF/E22/PWM; debug output prohibited |
| Boat ESP32 | GPIO35/36/37, 47/48, 0/3/45/46, 19/20, 43/44, 38 | OPI memory, board-specific 1.8V or RGB LED, strapping, USB/serial/JTAG or unknown; debug output prohibited |
| Remote Waveshare | All documented LCD/touch/TF SPI/I2C/RS485/CAN/USB/sensor interfaces | Onboard/shared functions; no GPIO is certified free |
| Both | Any proposed GPIO not physically present on a verified header | **NOT ASSIGNABLE** until continuity, voltage and load tests |

**Result:** there is **no approved marker output GPIO on either controller** today. The code expressly sets `BOAT_R30H9_MARKER_GPIO=-1` and `REMOTE_R30H9_MARKER_GPIO=-1`. It does not call `pinMode`, `digitalWrite`, `gpio_set_level`, timers or interrupts. A numbered candidate is **not evidence** that it is unused, safe, voltage compatible or physically exposed.

## Implemented source (no physical side effects)

- `Boat/source/boat_r30h9_marker_config.h` and `Remote/source/remote_r30h9_marker_config.h`: **compile-time #error** if pin allocation or hardware approval macro is modified from `-1 / 0` while no physical commissioning exists.
- Mirrored `Boat/source/r30h9_marker_contract.h` and `Remote/source/r30h9_marker_contract.h`: strict shared event names and `LocalRecorder` bounded FIFO (16 rows, no heap, no secret copies, overflow reported). Each record is tagged `SOFTWARE_LOCAL_UNCERTIFIED`. Neither the recorder nor its test cases can emit an electrical pin pulse. `currentlyApprovedOutput(...)` is *always false*. The tool cannot be turned into a live GPIO driver by setting a boolean in an unsigned worksheet.
- The real Boat `BoatDualRadioPort` and Remote `boat_radio.cpp` include the locked configuration, enforced by `static_assert`. They **do not instantiate or use** the new FIFO in critical paths. Existing optional, disabled R30H-2 Boat and R30H-3 Remote source-local traces already identify logical events. That historical USB-local timing is **not** common physical time.
- [Preflight](../tools/verify_r30h9_marker_preflight.py) and [host mutation tests](../tests/test_r30h9_marker_preflight.py) reject altered GPIO allocations, fake commissioning, tampered mirrored event headers, active probes inserted into passive signals, or forged test evidence. Boat/Remote portable C++11 tests validate FIFO ordering, overflow, monotonic rollover tolerance and hardcoded no-output policies. These tests prove **source invariants only**.
- [Ten unsigned candidate probes](BOAT_R30H_9_PROBE_CROSSWALK.csv), [12 physical gates NOT_RUN](BOAT_R30H_9_PHYSICAL_SIGNOFF.csv). There is no actual pin/connector number approved for active physical signals.

### Logical event-to-waveform mapping (not currently measured)

| R30H-9 logical event | Existing source anchor | Physical meaning / limitation |
|---|---|---|
| Remote STOP requested | `Remote/source/boat_radio.cpp:boatRadio_sendCommand()`, R30H-3 `STOP_BUTTON_REQUEST` | Software call can differ from real MCP23017 button electrical edge |
| Remote nRF TX buffer accepted | `radioSendTransport()`, R30H-3 `NRF_BUFFER_ADMITTED` | SPI/CE can be independently probed **only after measured pin/voltage signoff**; it does not prove over-air packet |
| Remote E22 UART buffer accepted | `radioSendTransport()`, R30H-3 `E22_UART_BUFFER_ADMITTED` | Not equivalent to RF airtime, Boat reception, or authenticated STOP |
| Boat authenticated STOP decoded | `BoatDualRadioPort::receive()`, R30H-2 `STOP_AUTH_RECEIVED` after HMAC and command decode | Local code event; **no electrical marker pin exists today** |
| Boat logical STOP intent | `BoatDualRadioPort::receive()`, R30H-2 `STOP_LOGICAL_INTENT` | Software intent is not MOSFET-off/zero current |
| Boat conditional RTH local entry | `BoatDualRadioPort::maybeStartFailsafeReturnHome()`, R30H-2 `RTH_LOCAL_ENTRY` | Not evidence that navigation/motors entered actual motion |
| Boat + Remote CLOCK_REALTIME samples | R30H-5/6 UTC discipline | Existing authenticated nRF subtype 9 makes coarse civil-time display, **not** an external microsecond clock |

In the current revision the realistic first capture is **passive observation of existing radio communication pins** with high-impedance, voltage-compatible probes: Remote nRF CSN (GPIO15 conflict *must* be resolved), Boat nRF IRQ (GPIO21/J3-18), E22 UART TX/RX at the inspected module pads, and independent RF-cut trigger. The acquisition must use **one external analyzer clock**. It can establish certain *electrical-edge intervals*, **not** true authenticated STOP timing, physical current-zero or PPS alignment.

## Physical RF-only handoff — do not skip gates

1. Identify the bought actual ESP32-S3 WROOM and Waveshare Standard board revisions, module firmware build/commit, both RF carriers, and wire/probe orientation. No guessed J1/J3 cavity or Waveshare connector contact is accepted.
2. Physically disconnect the **same common 3S4P battery bus** feeding MDD20A `VM` and EKM014 `VH`; retain no powered motors/coils. Independent hard STOP/current-zero hardware remains a separate unresolved acceptance blocker.
3. Power only inspected logic rails with proper current limits. Measure external probe input levels for nRF/E22 UART/IRQ, ESP32 logic, touch/RS485 and ground reference. Never attach 5V directly to 3.3V-only analyzer/ESP32. Do not run PA radios without suitable antennas/RF load and legally checked EU868 profile.
4. For the first non-actuating trial, sample `STOP_BUTTON_EDGE`, suitable SPI/UART edges and Boat IRQ/RX using **one calibrated external analyzer**. Record a minimum 30 STOP requests across nRF and E22 pacing phases only after R30H-1 RF signoffs. Record raw waveform, SHA256, both commits, clock-domain identity, probe attenuation and polarity. Time-of-reception of an authenticated STOP is **NOT_INSTRUMENTED** until independently reviewed physical firmware marker is installed.
5. PPS GNSS time alignment uses the separate [R30H-8](BOAT_R30H_8_SHARED_CLOCK_ALIGNMENT_READINESS.md) gate and three independent edges. Verify that the purchased M100 actually exposes PPS and its UTC-second association; never emulate PPS from UART or local `millis()`.
6. The [R30H-4 common-clock capture analyzer](../tools/analyze_r30h4_common_clock_capture.py) and [R30H-8 candidate-offset analyzer](../tools/analyze_r30h8_shared_clock.py) are **read-only and never return physical PASS**. Real evidence needs a human witness review, not a fabricated CSV signature or GitHub CI.
7. Do not enable any motor/solenoid GPIO, change radio-signoff flags or publish official versions as a side effect of instrumentation; all R30A physical safety, dummy-load and current-zero acceptance must pass separately before live hardware motion.

## Next R30H-10

After actual hardware procurement and GPIO resource clearance, select a **verified non-actuating marker** or another safe externally time-stamped method, implement a bounded pulse driver in an isolated hardware-only diagnostic build, and measure its instrumentation latency. Do not steal safety GPIO1/2/41/42, display/shared pins or RF communication lines without independent engineering approval.

**Unchanged releases:** Boat VER014, Remote VER028. **Unperformed tests:** R30H-9 ten probes + twelve physical gates, R30H-8 eight, R30H-7 twelve, R30H-3 sixteen STOP, R30H-1 twenty-two radio module and R30A safety checks.
