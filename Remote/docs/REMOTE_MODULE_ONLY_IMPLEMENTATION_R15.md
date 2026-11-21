# R15 — Remote premade-modules-only implementation
**Active replacement of earlier custom Remote PCB, 2026-11-21.** Maintain existing handheld 3D-printed mechanical design, display and 16-button arrangement. No new or fabricated electrical board.

## Premade modules
| Function | Ready module | Active status |
|--|--|--|
| Main UI/display/SD/Wi-Fi | Waveshare ESP32-S3-Touch-LCD-4.3 Standard | selected; actual board revision/header crosswalk unverified |
| 16 physical buttons including OK | Adafruit MCP23017 STEMMA QT/Qwiic breakout (16 GPIO) | candidate to replace custom button carrier; I2C 0x20 candidate |
| Four analog joystick axes | Adafruit ADS1115 breakout #1 (4ch) | candidate, 0x49, all AIN0..3 joystick; 3.3V supply and input range |
| Pack voltage/NTC and optional diagnostics | Adafruit ADS1115 breakout #2 (4ch) | candidate, 0x4A, conditioned inputs only; requires premade voltage divider/sensor modules |
| Primary nRF24 PA/LNA | Existing manufactured radio board and SMA antenna | selected class, pins TBD; 3.3V rail requires TX current test |
| nRF 3.3V regulator | Pololu S9V11F3S5, 2–16V input, 3.3V/1.5A | candidate **replaces custom TPS63070 design** |
| Backup E22 UART LoRa | Existing manufactured E22-900T22S module | selected, matching Boat onboard SX1262 interoperability UNPROVEN |
| E22 UART bridge | Premade SC16IS750 I2C-to-UART breakout | candidate; bridge I2C address **0x48** currently reserved |
| Dedicated E22 5V rail | Pololu S13V20F5 2.8–22V/5V/2A buck-boost | candidate **replaces custom TPS61023 PCB**, no 5V paralleling |
| Controller/display 5V | Waveshare **onboard CS8501** power system | selected, battery port single-cell PH2.0; power/charging needs verification |
| 1S4P cells, protection, charging | Commercial battery pack with BMS + approved manufacturer charger/connector (as applicable) | battery pack/charging not yet chosen |
| Switch/fuse/wiring | Premade power switch, commercial inline fuse/locking harness | part ratings, draw and room TBD |

## I2C collision prevention
- `0x20` planned Adafruit MCP23017 (verify A0–A2 logic and conflicts with display/CH422G).
- `0x48` reserved for SC16IS750. Existing other documentation suggests ADS1115 at 0x48: **BLOCKED** due to address conflict.
- `0x49` ADS1115 #1 for 4 joystick axes; `0x4A` ADS1115 #2 for conditioned 1S pack voltage/temperature/radio diagnostics. Actual straps on the ready-made breakout and driver startup probes **not validated**.
- Do not assume internal Waveshare touch/SD I2C pins can be shared arbitrarily; check schematic/board revision and test. Expander IRQ radio path over slow I2C remains an unproven latency item; direct GPIO required for time-critical IRQ if latency fails.

## Power / pack restrictions
Only a **single-cell 1S4P** battery pack can feed the Waveshare battery connector. Do not attach 3S. Charge management integrated into the Waveshare board must be tested with the chosen protected 1S4P pack; onboard CS8501 USB charger current/termination may not meet pack specification. Do not parallel independent regulator outputs or bypass BMS.

No generic 2.54mm prototyping board, custom flex PCB, bespoke joystick/button PCB, or 3D printed electrically conductive traces. Use factory made modular joystick and button assemblies with keyed/removable cables and factory-manufactured multiway connectors. Enclosure 3D print remains allowed.

## Software transition — NOT hardware commissioned
Existing LVGL UI, calibration, radio, 16 buttons and fail-safe navigation do not need replacement. However actual I2C ADC addresses, key scan and cable pin maps must be updated and host/target tested before hardware attach. No actual GPIO commits are authorized in R15, since board header pinout and mechanical clearances remain unverified. All firmware remains fail-closed.

**Sonar**: unchanged, undecided module; no software/hardware integration now.
