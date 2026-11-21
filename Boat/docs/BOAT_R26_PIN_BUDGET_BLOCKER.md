# R26 — Verified pin budget constraint, no invented shared SPI
Date: 2026-11-21. Host design analysis only, no wiring freeze.

## Evidence
Heltec v1.1 manufacturer datasheet shows J3 GPIO17/7/6/5/4 and J2 GPIO26 exposed; it also lists integrated SX1262 on GPIO8–14 and GNSS/TFT on other dedicated pins. https://resource.heltec.cn/download/Wireless_Tracker/Wireless%20tracke.pdf
A v1.2-targeting third-party board_pins.h reports the same onboard SX1262/GNSS/TFT GPIO values: https://github.com/jbjardine/HTIT-Tracker-v1.2-forIDF/blob/main/components/board/board_pins.h
**Neither proves physical v1.2 header continuity.** The Heltec v1.1 header table does NOT show GPIO8/9/10/11 among the external header pins; therefore the SX1262 SPI bus must NOT be assumed accessible for nRF24 wiring.

## Minimum viable native pin budget (before motor DIR, solenoids and safety feedback)
| Consumer | Pins |
|---|---:|
| nRF24 independent SPI SCK/MOSI/MISO | 3 |
| nRF24 CE + CSN | 2 |
| I2C SDA + SCL | 2 |
| MDD20A PWM left/right | 2 |
| **Minimum** | **9** |
| Provisional external candidates (4,5,6,7,17,26) | **6** |
| **Shortfall** | **3** |

A *hypothetical* proven shared SX1262 SPI bus reduces demand to six, but this cannot be selected without manufacturer board evidence and physical access. The six GPIO candidate set is not qualified for v1.2, and even a six-pin design leaves motor DIR and feedback to vetted expansion modules. The R25 recommendation that assumed shared SPI was conditional and MUST NOT be implemented as-is.

## Feasible directions to investigate, without selecting one
A. Find an official v1.2 schematic and extra physically accessible GPIOs; do not treat boot straps/USB/GNSS/TFT pins as spare.
B. Find an off-the-shelf SPI GPIO bridge module on I2C, with demonstrably adequate nRF24 throughput and deterministic CE/CSN timing. Requires engineering proof, not a generic MCP23017 promise.
C. Use a preassembled dual-radio controller module or commercially supported external interface that handles nRF24 SPI without a second MCU, if available and owner-approved.
D. Revise radio allocation with explicit owner approval. Never silently drop the nRF primary or change the controller.

## Executable analysis
`python3 Boat/tools/r26_pin_budget.py` provides four synthetic budget tests. Passing tests only establishes arithmetic, not board compatibility.

## Release gates
NO numeric GPIO pin freeze, NO nRF24 wiring, NO actuator power until manufacturer v1.2 pin evidence, a feasible native GPIO budget, verified independent safety hardware, and physical acceptance.
