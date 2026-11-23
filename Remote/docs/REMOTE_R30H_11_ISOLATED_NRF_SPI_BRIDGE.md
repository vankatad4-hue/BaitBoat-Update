# R30H-11 — Remote nRF24 isolated SPI transport and hardware test plan

**Date:** 2026-11-23. **Status:** SOFT-CANDIDATE / PREBUILT BOARD NOT YET CONFIRMED OR ACCEPTED. This document is an implementation/bench handoff, **not an approval to wire, transmit, flash or operate actuators**.

## 1. Decision
Use a **ready-made NXP SC18IS606-EVB** I²C-to-SPI bridge evaluation board as a candidate, with no additional ESP32, custom PCB or stolen Waveshare GPIO. SC18IS606 has a 400 kHz I²C target interface, internal oscillator and separate SPI master (up to 1.875 Mbit/s). Our provisional firmware uses SPI mode 0, 455 kHz. Existing Waveshare ESP32-S3-Touch-LCD-4.3 **Standard** TF card remains on host GPIO11 MOSI / 12 SCK / 13 MISO and CH422G EXIO4 real CS; GPIO6 remains SD dummy CS, GPIO15 remains RS485 TXD. Remote nRF CSN, CE, SCK, MOSI, MISO and IRQ have **no direct MCU GPIO** (`-1`). Do not connect the radio to Waveshare TF SPI wires.

**Alternative rejected:** MCP23017 I²C CSN+CE while sharing SD SPI would leave TF card selected by CH422G EXIO4 during current SD operations. It also requires a separate RF24 GPIO HAL and transaction arbitration. An isolated SPI bridge avoids that contention without rewiring the SD card. This choice is a software prototype until actual hardware and latency prove acceptable.

**R30H-11 CE correction (2026-11-23):** SS2 is not usable as guaranteed fail-LOW active-high CE through reset because NXP resets it to SPI chip-select mode. The new source uses existing MCP23017@0x21 GPA0 and retains external CE pull-down; this improves POR high-impedance default but **does not guarantee CE LOW if the MCP latch remains HIGH during ESP32 reset, stuck MCU or I²C failure**. No physical safety or RF commissioning can proceed without an engineered reset-safe solution and measured evidence.

## 2. NXP EVB connector and protocol schedule — not a physically verified harness
Manufacturer manual UM11666 identifies onboard EEPROM on **SS1**. NXP datasheet §7.1.10 also says SS2 resets as a SPI slave-select output, **not GPIO**; inactive-high reset output could force active-high nRF CE HIGH despite a passive pull-down. Hence **SS2 must remain UNCONNECTED**. The existing ready-made MCP23017@0x21 takes CE on GPA0 (with external pull-down). This is a software *candidate* crosswalk, **not a physically approved harness**:

| Waveshare / SC18IS606-EVB | Connection | Purpose |
|---|---|---|
| ESP32 GPIO8 SDA (shared I²C) | EVB JP2-6 SDA | I²C to external SPI bridge |
| ESP32 GPIO9 SCL (shared I²C) | EVB JP2-3 SCL | Verify idle pullups and effective clock |
| 3.3 V suitable logic rail | EVB JP2-5 VCC | Verify maximum board load/current and regulator margin |
| Logic GND | EVB JP2-4 GND | Common signal reference; no motor current |
| EVB JP1-2 SPICLK | nRF SCK | **Bridge SPI**, not Waveshare GPIO12 |
| EVB JP1-3 MOSI | nRF MOSI | Bridge SPI, not Waveshare GPIO11 |
| EVB JP1-4 MISO | nRF MISO | Bridge SPI, not Waveshare GPIO13 |
| EVB JP1-7 SS0 | nRF CSN | Automatic select per 0x01 SPI command |
| EVB JP1-5 SS2 | **UNCONNECTED** | Reset-mode chip select may be driven HIGH; must never connect directly to active-high CE |
| EVB JP1-6 SS1 | **ONBOARD EEPROM** | Remains on EVB EEPROM; do not route to nRF |
| Existing MCP23017 @ 0x21 GPA0 | nRF CE | Existing external I²C module; external pull-down, low latch before output direction, GPIO readback |
| nRF IRQ | **UNCONNECTED** | Poll STATUS/FIFO; bound latency in physical timing test |
| EVB JP2-1 RESET / JP2-2 INT | Not assigned | No spare MCU GPIO assumed |
| nRF 3V3 PA/LNA VCC | Dedicated selected regulated 3.3V radio rail | Measure transient/ripple; not EVB control output |
| nRF ground | Radio/logic ground | Common signal reference, RF ground and appropriate antenna |

**I²C address:** the EVB manual uses 8-bit write address `0x50` with JP3 all jumpers ON, equivalent to the Arduino `Wire` 7-bit address **`0x28`**. The factory strap positions and onboard I²C pullups JP4 must be checked physically; I²C scanner and clock-scope capture precede any bridge initialization. The bridge cannot be assumed present on 0x28. Other software bus devices include CH422G/touch, MCP23017 0x20/0x21, SC16IS750 0x48, ADS1115 0x4A. No multi-master bus is introduced.

**Manufacturer documents:** [SC18IS606 data sheet](https://www.nxp.com/docs/en/data-sheet/SC18IS606.pdf), [SC18IS606-EVB manual UM11666](https://www.nxp.com/docs/en/user-manual/UM11666.pdf), [EVB product](https://www.nxp.com/design/design-center/development-boards-and-designs/SC18IS606-EVB), [Waveshare Standard manual](https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3). An authentic prebuilt board exists; **seller stock, shipping to Bulgaria and final bill of materials are NOT confirmed**. Verify retail availability with reputable distributors prior to purchase, and remember this is an *evaluation* board, not a sealed outdoor-certified assembly.

## 3. Implemented software
- `sc18is606_port.h/.cpp`: require correct `SC18IS606` version ID, 5-attempt bounded idempotent setup retry, 12 × 1 ms bounded read wait while bridge is busy, **no retry of non-idempotent SPI transfer writes**. No bridge GPIO (including SS2) is assigned to CE. Maximum SPI command + 32-byte RF payload = 33 bytes.
- `mcp23017_ce_port.h/.cpp` and `mcp23017_ce_wire_io.h/.cpp`: use existing `MCP23017@0x21 GPA0` for CE, preserving GPA4 physical OK input. Read IOCON BANK0, write CE latch LOW **before** setting GPA0 output, clear its internal pull-up while preserving OK pull-up, verify IODIRA and physical GPIOA readback. Abort with best-effort CE LOW on I²C fault. Hardware pull-down during expander power-on high-Z remains required. This code cannot de-energize a stuck HIGH output after MCU crash or I²C failure; add physical reset/watchdog supervision before releasing RF.
- `sc18is606_wire_io.h/.cpp`: Arduino `TwoWire` adapter using the already initialized board I²C bus. It does not independently call `Wire.begin()` or change its frequency.
- `sc18is606_nrf_link.h/.cpp`: 32-byte fixed packet nRF24 role/address profiles matching Boat, channel 76, 250 kbit/s, RF_PA_LOW, CRC16, auto ACK, retry count 15. Status/FIFO **polling** means no nRF IRQ GPIO. TX queue one at a time, 90 ms stuck-TX fail-close; limited receive service (two frames per poll); corrupted application CRC rejected. An admitted frame is **not** proof Boat accepted/authenticated STOP.
- `boat_radio.cpp` uses the bridge radio object, not the direct RF24 Arduino GPIO instance; a bridge error immediately revokes `g_nrfReady` and clears the primary transport-ready flag. Existing command authentication/session/LoRa STOP backup semantics are retained.
- Three required runtime gates remain `0`: `REMOTE_NRF_PIN_AUDIT_SIGNED`, `REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED`, `REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED`. The compile-time `#error` forbids enabling unreviewed hardware by changing a single flag. Both PlatformIO Upload tasks remain denied, and no Boat motor/solenoid driver is attached.
- `Remote/tests/sc18is606_bridge_nrf_test.cpp` uses a fake protocol peer and injected loss, verifies silicon ID, expander GPA0 CE vs unused SS2 and EEPROM SS1, default-low/readback, nRF packet TX and RX, CRC, retry/timeout and no duplicate SPI frame on transfer NACK. **Host PASS proves software only.**

## 4. Ordered physical acceptance — separate from source CI
1. Confirm actual hardware revision of Waveshare, NXP EVB and exact nRF PA/LNA; photograph EVB JP1/JP2/JP3/JP4/JP5, verify physical JP pin orientation. **No radio wired to Remote yet**.
2. On safely limited 3.3V logic-only power, measure EVB SCL/SDA, address 0x28, version `SC18IS606`, MCP23017@0x21 GPA0 direction and nRF CE pull-down. With nRF disconnected, capture CE through cold start, MCU reset, MCP reset and bridge reset; test the **MCP output remaining HIGH across an MCU reset** and I²C loss. A simple pull-down cannot force a driven output LOW. Only an approved physical reset/watchdog gate may close this issue. Check EVB EEPROM SS1 remains inactive during SS0 transfers.
3. Without transmitting RF, capture bridge SS0/SS2 (SS2 must be unconnected), MCP CE GPA0, MOSI/MISO/SCK and independent Waveshare TF SPI and CH422G EXIO4 under simultaneous SD access. Confirm no electrical connection between two SPI masters, and no EEPROM select/miso contention.
4. With nRF attached only after validated physical wiring, stable regulated radio supply, legally compliant antenna/load and low-power profile approval, measure SPI response, RF acknowledges and authenticated session with real Boat. Use a single calibrated analyzer for latency. Confirm worst-case I²C occupancy and panel/touch/ADS1115/SC16IS750 load; compare against manual-control 100 ms update and STOP timing requirements.
5. Inject I²C NACK, bridge power interruption, nRF missing/lockup, CE stuck, SD import/map activity and simultaneous button STOP. Confirm software transitions to no primary, manual/release inhibit and backup STOP delivery. Physical STOP acceptance still requires independent current-zero dummy-load proof outside this radio test.
6. Sign R30H-10 H10-02/03/04/05/07 only from **real measured data and independent review**. All 18 H10 gates remain NOT_RUN/BLOCKED; do not change their CSV merely because bridge driver compiles.

## 5. Development / next work
```sh
# root VS Code + PlatformIO; build only, no flashing
python Boat/tools/verify_r30h_radio_preflight.py
python Boat/tools/verify_r30h10_physical_lock.py
python Boat/tools/verify_r30h10_gpio_map.py
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/tests/bridge_stubs -IRemote/source Remote/source/sc18is606_port.cpp Remote/source/mcp23017_ce_port.cpp Remote/source/sc18is606_nrf_link.cpp Remote/tests/sc18is606_bridge_nrf_test.cpp -o /tmp/remote_bridge && /tmp/remote_bridge
pio run -d Remote -e remote_waveshare_43_diagnostic
pio run -d Boat -e boat_n32r16v_diagnostic
```

**Open issues:** Remote bridge + nRF real-air RF interoperability, I²C delay/clock stretch under contention, packet/STOP latency, CE external pull-down, EVB jumper/electrical continuity, procurement stock, and independent STOP/current zero. No hardware tests performed. **VERSIONS remain Boat 014 / Remote 028.**
