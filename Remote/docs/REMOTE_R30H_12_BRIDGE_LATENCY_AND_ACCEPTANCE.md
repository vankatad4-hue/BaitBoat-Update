# R30H-12 — non-actuating I²C timing and physical evidence handoff

**2026-11-23. Status: source-only; no real ESP32, SC18IS606-EVB or nRF24 instrument readings.** R30H-11 isolated SPI transport is a *candidate*, not an RF installation approval. No change to Boat VER014, Remote VER028 or the existing 18 unsigned R30H-10 gates.

## What was checked against the manufacturer
NXP [SC18IS606 datasheet Rev 1.2 (2026-10-13)](https://www.nxp.com/docs/en/data-sheet/SC18IS606.pdf) §7.1.1–7.1.5: SC18IS606 may NACK its I²C address while busy; the host writes a function ID, then reads the SPI response from the bridge buffer. The device supports up to 400 kHz I²C and 455 kHz SPI with F0=0x01. The [EVB user manual UM11666](https://www.nxp.com/docs/en/user-manual/UM11666.pdf) specifies JP1/JP2 connections and an onboard EEPROM associated with CS1.

The nRF hardware design remains **SC18IS606 SS0 → CSN**, **MCP23017@0x21 GPA0 → CE**; **SS1 EEPROM and SS2 unconnected to nRF**. The Waveshare onboard TF SPI (GPIO11/12/13) is a *different controller*. The actual boards, expander GPIO reset behavior, power supply and RF antenna have not been physically validated.

## New software safeguard
`Sc18is606Port` now checks an elapsed-time budget of **12 ms per individual bridge I²C write and read/retry sequence** using a wrapping monotonic counter. Its write side never retries non-idempotent SPI commands after an ambiguous NACK. It refuses a late *successful* read, disables the bridge transport and best-effort drives MCP CE low on a failure; it does not publish a potentially late RF frame. Test injections cover busy NACKs, late read, delayed write, millis overflow, and no duplicate non-idempotent SPI transmission.

**Limit:** This is a **soft, post-call deadline**. A single `TwoWire::endTransmission()` or `TwoWire::requestFrom()` may block longer than 12 ms before returning. The code cannot interrupt it, preempt a stuck peripheral, or physically pull CE low if MCP output remains driven HIGH / I²C fails. Do not treat 12 ms as a measured STOP bound or hardware watchdog. Production operation must await real Wire timeout, I²C clock/stretching and external reset gating validation.

## Optimistic bus occupancy model (not measured)
The compiled helper `Remote/source/r30h12_i2c_wire_budget.h` counts 9 wire bits per I²C address/data byte, assumes no contention or clock stretching, and **omits** START/STOP overhead, FreeRTOS scheduling, SPI processing and any retries. It applies only to the nominal code paths.

| Operation | I²C bits counted | At 100 kHz | At 400 kHz |
|---|---:|---:|---:|
| One 32-byte payload bridge transfer (33 SPI bytes) | 621 | 6.21 ms | 1.55 ms |
| One CE toggle on MCP23017 including readback | 99 | 0.99 ms | 0.25 ms |
| `sendFrame()` wire-only floor, including three CE transitions | 1,089 | 10.89 ms | 2.72 ms |
| Completion+up-to-two-RX poll path | 1,800 | 18.00 ms | 4.50 ms |

These are **lower bounds** on bus wire occupancy for those source paths, NOT worst-case delays, theoretical STOP response guarantees or RF airtime. The actual UI, I²C expanders and E22 UART bridge also use the Waveshare bus. The bridge SPI at 455 kHz and external device delays are additional. The source estimates do not prove that the stated control/STOP deadlines are met.

## Physical evidence sequence
1. Verify actual board/EVB/nRF markings, connector pin 1, pullup placement, and logic 3.3 V compatibility. Keep motors and solenoids physically disconnected and upload gated; use independent current-limited bench logic power.
2. Passively probe CE GPIOA0, SS0, CS1 EEPROM, SS2 unconnected, bridge MOSI/MISO/SCK and Waveshare TF CH422G EXIO4. Confirm **CE LOW across MCU reset or stalled I²C**, especially after previously HIGH CE. Without a physical reset-safe solution this condition is **BLOCKED**.
3. With approved non-actuating RF-only test profile, record raw shared-clock captures for BUSY NACK and clock stretching, 32-byte bridge reads/writes, nRF ACK, maximum I²C occupancy, simultaneous SD map access and all UI/joystick inputs.
4. Instrument 30 physical emergency-STOP presses and authenticated Boat reception edges on one analyzer. Measure each trial and report p50/p95/p99/max and 95% confidence only when there are actual samples; do not invent values or a passing threshold.
5. Simulate loss of bridge power, MCP I²C NACK, CE output stuck HIGH, corrupted packet CRC, radio RF loss and reconnection, and concurrent heavy SD usage. Re-test the independent Boat hardware STOP/current-zero circuit with dummy loads separately.
6. For each check, enter true values into [R30H-12 physical evidence register](REMOTE_R30H_12_PHYSICAL_MEASUREMENTS.csv). Leave unsigned entries `NOT_RUN`/`BLOCKED`. Any source-only or GitHub Actions PASS is **not physical acceptance**.

## Software build / regression commands (development only)
```sh
python Boat/tools/verify_r30h10_physical_lock.py
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/source Remote/tests/r30h12_i2c_wire_budget_test.cpp -o /tmp/remote_i2c_budget && /tmp/remote_i2c_budget
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/tests/bridge_stubs -IRemote/source Remote/source/sc18is606_port.cpp Remote/source/mcp23017_ce_port.cpp Remote/source/sc18is606_nrf_link.cpp Remote/tests/sc18is606_bridge_nrf_test.cpp -o /tmp/remote_nrf_bridge && /tmp/remote_nrf_bridge
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```
Do **not** invoke `pio upload`; both projects intentionally deny uploads. Real hardware acceptance requires separate, signed, externally witnessed measurements.
