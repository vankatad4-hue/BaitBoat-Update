# R30H-11 — Remote nRF24 isolated SPI transport and hardware test plan

**Date:** 2026-11-23. **Status:** SOFT-CANDIDATE / PREBUILT BOARD NOT YET CONFIRMED OR ACCEPTED. This document is an implementation/bench handoff, **not an approval to wire, transmit, flash or operate actuators**.

## 1. Decision
Use a **ready-made NXP SC18IS606-EVB** I²C-to-SPI bridge evaluation board as a candidate, with no additional ESP32, custom PCB or stolen Waveshare GPIO. SC18IS606 has a 400 kHz I²C target interface, internal oscillator and separate SPI master (up to 1.875 Mbit/s). Our provisional firmware uses SPI mode 0, 455 kHz. Existing Waveshare ESP32-S3-Touch-LCD-4.3 **Standard** TF card remains on host GPIO11 MOSI / 12 SCK / 13 MISO and CH422G EXIO4 real CS; GPIO6 remains SD dummy CS, GPIO15 remains RS485 TXD. Remote nRF CSN, CE, SCK, MOSI, MISO and IRQ have **no direct MCU GPIO** (`-1`). Do not connect the radio to Waveshare TF SPI wires.

**Alternative rejected:** MCP23017 I²C CSN+CE while sharing SD SPI would leave TF card selected by CH422G EXIO4 during current SD operations. It also requires a separate RF24 GPIO HAL and transaction arbitration. An isolated SPI bridge avoids that contention without rewiring the SD card. This choice is a software prototype until actual hardware and latency prove acceptable.

## 2. NXP EVB connector and protocol schedule — not a physically verified harness
Manufacturer manual UM11666 identifies onboard EEPROM on **SS1**. Do not use SS1 for nRF CE: it could leave EEPROM selected while nRF SPI is active. The approved *candidate* crosswalk is:

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
| EVB JP1-5 SS2/GPIO2 | nRF CE | F6=0x04, F7=0x10, F4 low/0x04 high; externally pull down to ensure CE=LOW through reset/fault |
| EVB JP1-6 SS1 | **ONBOARD EEPROM** | Leave unused for nRF; avoid selecting EEPROM |
| nRF IRQ | **UNCONNECTED** | Poll STATUS/FIFO; bound latency in physical timing test |
| EVB JP2-1 RESET / JP2-2 INT | Not assigned | No spare MCU GPIO assumed |
| nRF 3V3 PA/LNA VCC | Dedicated selected regulated 3.3V radio rail | Measure transient/ripple; not EVB control output |
| nRF ground | Radio/logic ground | Common signal reference, RF ground and appropriate antenna |

**I²C address:** the EVB manual uses 8-bit write address `0x50` with JP3 all jumpers ON, equivalent to the Arduino `Wire` 7-bit address **`0x28`**. The factory strap positions and onboard I²C pullups JP4 must be checked physically; I²C scanner and clock-scope capture precede any bridge initialization. The bridge cannot be assumed present on 0x28. Other software bus devices include CH422G/touch, MCP23017 0x20/0x21, SC16IS750 0x48, ADS1115 0x4A. No multi-master bus is introduced.

**Manufacturer documents:** [SC18IS606 data sheet](https://www.nxp.com/docs/en/data-sheet/SC18IS606.pdf), [SC18IS606-EVB manual UM11666](https://www.nxp.com/docs/en/user-manual/UM11666.pdf), [EVB product](https://www.nxp.com/design/design-center/development-boards-and-designs/SC18IS606-EVB), [Waveshare Standard manual](https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3). An authentic prebuilt board exists; **seller stock, shipping to Bulgaria and final bill of materials are NOT confirmed**. Verify retail availability with reputable distributors prior to purchase, and remember this is an *evaluation* board, not a sealed outdoor-certified assembly.

## 3. Implemented software
- `sc18is606_port.h/.cpp`: require correct `SC18IS606` version ID before enabling outputs, 5-attempt bounded idempotent setup command retry, 12 × 1 ms bounded read wait while bridge is busy, **no retry of non-idempotent SPI transfer writes**. Startup F4 CE LOW before F6/F7 configuring GPIO2, then F5 readback verifies CE LOW. CE I²C write failure disables transport (best-effort only, not an independent electrical cutoff). Maximum SPI command + RF payload = 33 bytes.
- `sc18is606_wire_io.h/.cpp`: Arduino `TwoWire` adapter using the already initialized board I²C bus. It does not independently call `Wire.begin()` or change its frequency.
- `sc18is606_nrf_link.h/.cpp`: 32-byte fixed packet nRF24 role/address profiles matching Boat, channel 76, 250 kbit/s, RF_PA_LOW, CRC16, auto ACK, retry count 15. Status/FIFO **polling** means no nRF IRQ GPIO. TX queue one at a time, 90 ms stuck-TX fail-close; limited receive service (two frames per poll); corrupted application CRC rejected. An admitted frame is **not** proof Boat accepted/authenticated STOP.
- `boat_radio.cpp` uses the bridge radio object, not the direct RF24 Arduino GPIO instance; a bridge error immediately revokes `g_nrfReady` and clears the primary transport-ready flag. Existing command authentication/session/LoRa STOP backup semantics are retained.
- Three required runtime gates remain `0`: `REMOTE_NRF_PIN_AUDIT_SIGNED`, `REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED`, `REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED`. The compile-time `#error` forbids enabling unreviewed hardware by changing a single flag. Both PlatformIO Upload tasks remain denied, and no Boat motor/solenoid driver is attached.
- `Remote/tests/sc18is606_bridge_nrf_test.cpp` uses a fake protocol peer and injected loss, verifies silicon ID, GPIO2 CE vs EEPROM SS1, nRF packet TX and RX, CRC, timeout and no duplicate SPI frame on transfer NACK. **Host PASS proves software only.**

## 4. Ordered physical acceptance — separate from source CI
1. Confirm actual hardware revision of Waveshare, NXP EVB and exact nRF PA/LNA; photograph EVB JP1/JP2/JP3/JP4/JP5, verify physical JP pin orientation. **No radio wired to Remote yet**.
2. On safely limited 3.3V logic-only power, measure EVB idle SCL/SDA and pullups; verify address 0x28, read version `SC18IS606`, validate `-RESET` pullup and CE LOW/pulldown through both reset and powered failure. Check whether EVB EEPROM SS1 stays high while SS0 transactions occur.
3. Without transmitting RF, capture bridge SS0/SS2, MOSI/MISO/SCK and the independent Waveshare TF SPI and CH422G EXIO4 under simultaneous SD access. Confirm no electrical connection between two SPI masters, and no EEPROM select/miso contention.
4. With nRF attached only after validated physical wiring, stable regulated radio supply, legally compliant antenna/load and low-power profile approval, measure SPI response, RF acknowledges and authenticated session with real Boat. Use a single calibrated analyzer for latency. Confirm worst-case I²C occupancy and panel/touch/ADS1115/SC16IS750 load; compare against manual-control 100 ms update and STOP timing requirements.
5. Inject I²C NACK, bridge power interruption, nRF missing/lockup, CE stuck, SD import/map activity and simultaneous button STOP. Confirm software transitions to no primary, manual/release inhibit and backup STOP delivery. Physical STOP acceptance still requires independent current-zero dummy-load proof outside this radio test.
6. Sign R30H-10 H10-02/03/04/05/07 only from **real measured data and independent review**. All 18 H10 gates remain NOT_RUN/BLOCKED; do not change their CSV merely because bridge driver compiles.

## 5. Development / next work
```sh
# root VS Code + PlatformIO; build only, no flashing
python Boat/tools/verify_r30h_radio_preflight.py
python Boat/tools/verify_r30h10_physical_lock.py
python Boat/tools/verify_r30h10_gpio_map.py
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/tests/bridge_stubs -IRemote/source Remote/source/sc18is606_port.cpp Remote/source/sc18is606_nrf_link.cpp Remote/tests/sc18is606_bridge_nrf_test.cpp -o /tmp/remote_bridge && /tmp/remote_bridge
pio run -d Remote -e remote_waveshare_43_diagnostic
pio run -d Boat -e boat_n32r16v_diagnostic
```

**Open issues:** Remote bridge + nRF real-air RF interoperability, I²C delay/clock stretch under contention, packet/STOP latency, CE external pull-down, EVB jumper/electrical continuity, procurement stock, and independent STOP/current zero. No hardware tests performed. **VERSIONS remain Boat 014 / Remote 028.**
