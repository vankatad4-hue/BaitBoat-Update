# R31 — Actual firmware GPIO contract audit (2026-11-21)
Sources examined:
- Boat/source/boat_radio_hw_config.h: nRF CE/CSN and onboard SX1262 disabled by default.
- Boat/source/mdd20a_arduino_stage.cpp: begin() configures permit, watchdogEdge, dirLeft, dirRight, pwmLeft, pwmRight as SIX distinct direct GPIOs; 20kHz 10-bit LEDC on two channels.
- Boat/source/firmware/Flytec_Boat.ino: intentionally non-actuating; motor profile unassigned; no motor/solenoid driver commissioned.

## Corrected pin budget
| Consumer | Direct native GPIO in current HAL |
|---|---:|
| nRF SPI (SCK/MOSI/MISO) | 3 |
| nRF CE/CSN | 2 |
| I2C SDA/SCL | 2 |
| MDD20A stage permit/WDI/DIR-L/DIR-R/PWM-L/PWM-R | 6 |
| **Total** | **13** |

This is a lower bound for the current direct-GPIO firmware, before solenoid requests and independent safety feedback. R26/R29 nine-pin count was an architectural minimum that assumed offloading DIR and other controls; it is NOT compatible with the existing six-pin direct motor HAL without refactoring. An MCU permit does not replace independent hardware cut-off.

## Screenless V2 assessment
R29 proposed nine candidate pins 6,17,21,37,40,41,42,43,44. R30 flags GPIO21 as backlight conflict, GPIO40–42 as TFT shared nets, and GPIO43/44 as UART0 startup risk. No fully electrically qualified 13-pin native allocation exists. SC18IS602B may reduce nRF native pins but does not eliminate six native motor signals.

## Next engineering actions
1. Identify which direct MDD20A signals can move to a factory-made qualified expansion module; two 20kHz PWM outputs should remain native unless a verified module can provide equivalent frequency and safe defaults.
2. Add a separate V2 screenless board profile disabled by default and preserve non-actuating target until physical acceptance.
3. Verify TFT electrical sharing and UART boot states on exact V2 board.
4. Obtain manufacturer-approved V2 header mapping and independent actuator safety stack.
5. No motor/solenoid energization, no release publication; sonar deferred.
