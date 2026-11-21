# R30 — Screenless V2 electrical conflict review
2026-11-21. This review corrects a critical misconception: **unused TFT is still physically connected**.

## Decision
The R29 allocation has **nine unique software GPIOs**, but is **not nine electrically independent signals**. In particular:
- GPIO21 is TFT backlight drive. Driving MDD20A PWM on it may pulse TFT backlight, load GPIO or couple noise. **Reject GPIO21 for motor PWM pending physical backlight isolation evidence.** Merely disabling TFT driver does not isolate the circuit.
- GPIO40 is TFT D/C input, and GPIO41/42 are TFT SPI inputs. These may be shared as outputs/inputs for nRF SPI only if TFT chip-select is provably inactive from power-on through reset/brownout, and signal levels validated.
- GPIO43/44 are UART0 pins. ROM output and startup state can glitch nRF CE/CSN; require oscilloscope evidence and passive default CE-low/CSN-high wiring.
- GPIO37 as PWM candidate must be verified at the exact header and boot state.
- TFT reset GPIO39 and CS GPIO38 cannot be treated as guaranteed high without checking external bias.
- All onboard LoRa RF PA, GNSS and USB pins remain reserved.

## Immediate consequence
The R29 mapping must not be released to wiring. Replace GPIO21 with a **verified spare native pin** if one exists, or move *both* motor PWM channels to a commercially assembled, fail-safe qualified PWM interface, subject to PWM frequency and independent STOP validation. A generic PCA9685 is not automatically acceptable for motor speed control.

## Static executable gate
`python Boat/tools/r30_v2_pin_conflict_gate.py -v` checks uniqueness, reserved nets and known sharing. Its passing tests assert that R29 has documented hazards, **not that hardware is safe**.

## Hardware gate
Obtain exact V2 PCB revision and official schematic net-level trace; inspect TFT backlight transistor/driver, CS pull resistors, UART boot behavior, and header exposure. Scope GPIOs at reset/brownout with all power sources and USB combinations. Only after this choose the final module and pin mapping.

## Status
V2 screenless = preferred *candidate*, not approved replacement. No firmware pin freeze, no actuator enable, no physical acceptance, no new official releases. Independent premade safety stack unresolved.
