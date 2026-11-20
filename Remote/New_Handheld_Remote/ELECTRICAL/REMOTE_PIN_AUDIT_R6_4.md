# Remote R6.4 — Waveshare ESP32-S3-Touch-LCD-4.3 Pin Audit

**Status:** source-based audit; no pin assignment frozen. Audit is for the touch model SKU 25948, not the 4.3B/4.3C variants.

## Board functions that must be treated as occupied

| ESP32 GPIO | Board function | Design treatment |
|---|---|---|
| 0,1,2,3,5,7,10,14,17,18,21,38–42,45–48 | RGB LCD signals | unavailable |
| 4 | Touch IRQ | reserved |
| 8,9 | I2C SDA/SCL; touch, CH422G and external I2C header | shared bus, available for I2C peripherals only |
| 11,12,13 | TF card SPI | possible SPI repurpose only if TF card is not used and loading/CS is checked |
| 15,16 | RS485/CAN circuitry | avoid until mux/transceiver loading and selection are reviewed |
| 19,20 | USB data | reserved |
| 43,44 | USB-UART CH343 connection | potential UART pins only with USB serial isolation/conflict analysis |
| 6 | Sensor AD input terminal | candidate only; verify analog front-end/connector loading and boot behavior |

## I2C peripherals planned

| Device | Planned address | Notes |
|---|---|---|
| Touch controller (GT911) | address selected by board wiring (commonly 0x5D or 0x14) | Existing device; verify in firmware / board schematic |
| MCP23017 button expander | 0x20–0x27 | Set A0–A2 straps; reserve one address |
| ADS1015 joystick ADC | 0x48–0x4B | Set ADDR; default often 0x48 |
| CH422G | proprietary / board-specific I2C address behavior | Existing onboard expander; avoid assumptions; verify library and bus scan compatibility |
| PCF85063A RTC | 0x51 | onboard; do not collide |

**Action:** Perform an I2C scan on the board in bench firmware before finalizing address straps. Confirm that MCP23017/ADS1015 can coexist with the board's touch and CH422G drivers.

## SPI / primary radio strategy

- If the TF card is not a project requirement, GPIO11/12/13 are candidates for nRF24 SPI (SCK/MOSI/MISO) after reviewing the SD slot and SD_CS behavior.
- nRF CSN must be a direct ESP GPIO for clean timing. CE can be a direct GPIO or an expander output if latency and pulse timing are validated; do not assume the expander is appropriate for high-rate control.
- nRF IRQ can connect to an MCP23017 interrupt input or a direct GPIO, but interrupt latency and servicing must be tested.
- Radio SPI must remain independent of LVGL frame processing and have bounded transaction duration.

## E22 backup-radio UART strategy

The standard E22 uses UART. GPIO43/44 are attached to the onboard CH343 USB-UART circuit, so simply wiring the E22 to those pins risks contention while USB serial is active. GPIO15/16 are connected to RS485/CAN circuitry and are not clean uncommitted pins either.

**Required design choice before schematic:**
1. Verify if board-level jumpers/0-ohm links can isolate CH343 from GPIO43/44; if so, make a selectable UART route (USB-debug position vs E22 position), or
2. Use a small dedicated UART interface MCU/bridge for radio/control peripherals, or
3. Select an E22 configuration that can be managed by a verified alternate interface—do not assume it is SPI.

The remote must retain USB firmware flashing and diagnostic capability; design the UART route to avoid permanently losing it.

## Safety I/O

- Physical power switch is a hardware power control input, not a normal UI button.
- Bait/hook release buttons are separate guarded inputs. They require independent confirmation and must not be driven by radio failsafe or duplicated packet replay.
- On startup, external expanders/ADC missing or invalid readings inhibit boat-control TX until a self-test passes.
- On radio-link loss, manual control outputs go neutral; autonomous return behavior is a boat-side policy and must be explicit.
- Battery voltage monitoring uses a high-impedance divider, switched or buffered if needed to limit pack drain; ADC scaling and protection are calculated in the final schematic.

## Conclusion

The Waveshare board is usable as the touchscreen/UI computer, but the pin audit confirms that it is **not a freely assignable ESP32-S3 carrier**. The project should not freeze the radio wiring yet. A pin-safe daughterboard/interface plan is required and must preserve USB debug, touch/I2C, and display operation.


> **Archive reconciliation correction (2026-11-20):** Treat this audit as a fresh warning list, not as a replacement pin plan. Established R5 plan reuses GPIO11/12/13 for shared TF/nRF SPI, GPIO6 as direct nRF CE and GPIO15 as CSN only with onboard Sensor AD and RS485 paths isolated, and uses the SC16IS750 bridge for E22 UART. Exact PORT1 crosswalk remains provisional. Refer to ARCHIVE_RECONCILIATION_AND_NEXT_GATE_R6_4.md.