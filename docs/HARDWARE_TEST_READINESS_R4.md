# Hardware Test Readiness R4

## Remote
| Item | Status | Gate |
|---|---|---|
| Waveshare ESP32-S3-Touch-LCD-4.3 | Candidate | Exact revision and PORT1 crosswalk |
| Remote 1S4P pack/protection | Design candidate | Cells, BMS, fuse, connector, polarity, pack assembly |
| CS8501 controller rail | Onboard design | Load/ripple/thermal/USB behavior |
| TPS63070 nRF rail | Capture spec | Native circuit/layout, load and transient test |
| TPS61023 E22 5V rail | Capture spec | Reference design/component values, load/thermal test |
| nRF module | Bench driver present | GPIO6 CE/GPIO15 CSN isolation and SPI/SD arbitration |
| E22/SC16 bridge | Driver/config present | I2C scan, config readback, UART/RF pair |
| ADS1115/MCP23017 | Pin plan | Address scan, voltage/pullup and GPIO tests |
| Mechanical envelope | 300×210×70 target | Physical mockup, screen/cell/joystick/antenna fit |
| Carrier PCB | 100×75 proposal | Stack/placement/connector fit, native CAD |

## Boat
| Item | Status | Gate |
|---|---|---|
| Heltec Tracker | Candidate | Exact revision, pin map and connector crosswalk |
| 3S4P/BMS20A | Reported | Confirm actual pack and limits |
| Electronics 5V buck | Candidate≥2A | Select module/circuit, load/thermal test |
| nRF | Driver scaffold | Pin map and SPI allocation |
| E22 | Driver/test sketch | UART/M0/M1/AUX pin map and config readback |
| MDD20A | Candidate, footprint conflict | External mount/compartment redesign and motor current test |
| Bait/hook coils | Nominal12V models | Coil current/resistance, voltage tolerance, MOSFET/diode/fuse design |
| Boat carrier | 77×79×14mm preliminary | CAD fit; separate motor driver |

## Firmware
- Radio drivers/protocol scaffold: source present.
- Authenticated communication: **disabled** (key store returns false).
- Session epoch: random8-bit scaffold; persistent replay-safe session negotiation incomplete.
- App ACK and state synchronization: not implemented.
- Boat command executor/actuator integration: not implemented.
- Boat failsafe helper: not integrated with main loop/watchdog.
- Firmware compile: not run.
- Radio bench: not run.

## Electrical/CAD
- Remote/boat electrical capture docs and pin CSVs: present.
- Native KiCad schematics: not created.
- ERC/DRC: not run.
- PCB layout/fabrication: not created.
- Component footprints: not finalized.
- RF power/antenna regulatory compliance: not verified.
- Duty-cycle counter: RAM-only, not field-ready.

## Safe next bench sequence
1. Verify board revision and connector crosswalk before wiring.
2. Build controller/display power only with current-limited supply; verify polarity, rails and temperature.
3. Test I2C/ADC/expander peripherals without radios.
4. Test nRF primary pair using fixed probe frames, antennas attached, low power, close range.
5. Test E22/SC16 configuration and UART loopback.
6. Test E22 radio pair, fixed probe frames, legal low-power profile, record RSSI/packet loss/latency.
7. Test simultaneous radio/display/Wi-Fi/SD power and SPI/I2C bus contention.
8. Boat controller/power tests with all motor/solenoid outputs disconnected.
9. Measure motor/coil currents on isolated fixtures before selecting drivers/fuses.
10. Only after firmware safety, authentication, local watchdog and actuator interlocks are tested, integrate boat/hull.

**Status:** electronics are at design/test-scaffold stage. This is not yet a final fabrication or field test release.
