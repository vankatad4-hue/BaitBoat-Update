# Boat and Remote GPIO hardware assignment register R1

**Branch:** `platformio-unified-r30h20`
**Status:** software design allocation only. The physical boards, headers, cable orientation, logic levels, and peripheral revisions have not been inspected or signed. A listed candidate is not permission to energize it.

This register consolidates the current source maps and board-owned peripherals. It treats every usable GPIO as assigned to a function or explicitly reserved. It does not allocate pins to unreviewed hardware just to remove a `reserved` entry.

## Boat — ESP32-S3-DevKitC-1, N32R16V candidate

| GPIO | Current owner / disposition | Notes |
|---:|---|---|
| 0, 3, 45, 46 | Boot/strapping reserve | Do not attach external circuits that can change reset levels. |
| 1, 2, 41, 42 | Reserved for pending safety design | No selected physical STOP/watchdog/feedback interface is wired. Keep unavailable to sensors/debug. |
| 4, 5 | HGLRC M100 PRO UART TX/RX | UART and connector levels remain unverified. |
| 6 | E22 NRST | Candidate active-low modem reset. |
| 7, 8 | Shared I²C SCL/SDA | GY-91 (MPU9250/BMP280), INA231 and MCP23017; M100 compass is also listed on this bus in current docs. Verify the actual bus topology and addresses. |
| 9, 10 | nRF24 CSN/CE | Candidate radio controls. |
| 11, 12, 13 | nRF24 MOSI/SCK/MISO | Boat radio SPI. |
| 14, 15, 16 | E22 AUX/M0/M1 | Candidate modem control signals. |
| 17, 18 | E22 UART TX/RX | Candidate 3.3 V UART path. |
| 19, 20 | Native USB D−/D+ | Reserve for USB. |
| 21 | nRF24 IRQ | Optional interrupt; polling can be used if IRQ is omitted. |
| 22–34 | Not available on the selected ESP32-S3 GPIO set | Not header GPIO assignments for this board target. |
| 35–37 | N32R16V octal memory reserve | Do not use as external GPIO. |
| 38, 48 | Revision-dependent RGB LED / low-voltage reserve | Do not repurpose without exact board-revision confirmation. |
| 39, 40 | MDD20A PWM1/PWM2 | Candidate outputs, still locked from runtime activation; these pins also replace JTAG functions. |
| 43, 44 | USB-UART console TX/RX | Keep for serial diagnostics. |

The MCP23017 at candidate address `0x27` supplies the expander functions: `GPA0–GPA3` for EKM014 logical requests and `GPB0–GPB1` for MDD20A direction. Expander readback is not independent current or safety feedback. GY-91 uses I²C for MPU9250 and BMP280; the Boat water-temperature source remains a sensor data interface question, not a spare GPIO assignment.

### Boat sonar allocation

The Open Echo shield still has **no Boat GPIO assignment**. Its expected interface needs a dedicated SPI chip-select, IO1, IO2, OUT3, OUT4 and analog VOUT sampling; SPI clock/data may be shared only with deliberate bus arbitration. The remaining general-purpose pins are reserved or already owned, and the available candidate map does not currently show a conflict-free analog input plus four control/status pins. Do not connect shield UNO pin numbers directly to Boat GPIOs. Revisit this allocation after the exact shield/Boat revisions and analog/output levels are measured, and choose a vetted alternate interface or additional controller if the pin/ADC budget remains insufficient.

## Remote — Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948

| GPIO | Current owner / disposition | Notes |
|---:|---|---|
| 0, 1, 2, 3, 5, 7, 10, 14, 17, 18, 21, 38–42, 45–48 | RGB display bus | Board-owned LCD timing/data signals. GPIO38/48 and module variant require exact revision confirmation. |
| 4 | GT911 touch IRQ | Board-owned input. |
| 6 | Board sensor-AD / SD-library dummy-CS path | Do not use for radio CE; old CE assignment conflicts. |
| 8, 9 | Shared I²C SDA/SCL | GT911/CH422G and external expanders/converters. |
| 11, 12, 13 | TF-card SPI MOSI/SCK/MISO | Board-owned SD bus. nRF24 uses the external SC18IS606 bridge, not these MCU SPI pins. |
| 15, 16 | Onboard RS485/CAN TX/RX | Board-owned; old direct nRF24 CSN assignment conflicts with GPIO15. |
| 19, 20 | Native USB D−/D+ | Reserve for USB. |
| 22–34 | Not available on the ESP32-S3 GPIO set | Not usable GPIO numbers. |
| 35–37 | Module/resource reserve pending exact memory-routing check | Do not allocate externally. |
| 43, 44 | CH343 USB-UART console TX/RX | Reserve for serial diagnostics. |

The display's GPIO data/timing mapping and the rest of the wiring are mirrored in [`remote_pin_assignments.h`](../Remote/source/remote_pin_assignments.h). External controls use I²C devices: 16 button positions across MCP23017 devices `0x20`/`0x21`; four joystick axes on ADS1115 `0x4A`; E22 UART/control through SC16IS750 `0x48`; nRF24 SPI through SC18IS606 `0x28`, with CE via MCP23017 `0x21` GPA0. These bridge/expander allocations are still candidates and remain unsigned. CH422G EXIO1/2/4/5 provide touch reset, backlight, TF CS, and USB/CAN select respectively; EXIO pins are not ESP32 GPIOs.

Remote receives sonar summaries over the existing authenticated radio telemetry path; it has no sonar GPIO or local sonar sensor assignment. GPIO6/15 remain unavailable for direct nRF24 controls, and all radio commissioning flags remain false.

## Commissioning boundary

- Source maps describe intended ownership only. Board identity, connector orientation, voltage compatibility and peripheral pinout still need physical inspection.
- No new pins are enabled in firmware by this register. Radio, motor and release output commissioning remains controlled by the existing signed-audit gates.
- Do not mark an unsigned candidate physically verified from source inspection or CI results.

## Source and board references

- [Boat source pin map](../Boat/source/boat_pin_assignments.h)
- [Boat header crosswalk](../Boat/docs/BOAT_R20_GPIO_RESOURCE_CROSSWALK.csv)
- [Boat GPIO/header map](../Boat/docs/BOAT_GPIO_HEADER_MAP_R20.md)
- [Remote source pin map](../Remote/source/remote_pin_assignments.h)
- [Remote GPIO map](../Remote/docs/REMOTE_R30H_10_PIN_MAP.csv)
- [Remote Waveshare resource audit](../Remote/New_Handheld_Remote/PCB/WAVESHARE_GPIO_RESOURCE_AUDIT_R1.md)
- [Open Echo wiring feasibility gate](../Boat/docs/BOAT_OPEN_ECHO_DIRECT_WIRING_GATE_R1.md)
