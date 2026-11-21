# R33 — Commercial bridge and I/O module sourcing validation
Date 2026-11-21. No purchase authorized. All stock figures are snapshots and must be rechecked before ordering.

## Manufacturer evidence
NXP SC18IS602B **No Longer Manufactured**: https://www.nxp.com/products/SC18IS602B
NXP SC18IS606 **Active**, replacement with DIFFERENT pinout; 400 kHz I2C, 1.8 Mbit/s SPI, 1024-byte FIFO, three chip-select/GPIO pins: https://www.nxp.com/products/SC18IS606
The two bridge chips are NOT footprint/pin-compatible and the firmware protocol must be independently verified.
NXP assembled SC18IS606-EVB listed as active evaluation board: https://www.nxp.com/products/SC18IS606

## Approved supplier search findings
| Module | Supplier | URL | Finding |
|---|---|---|---|
| MIKROE-3743 I2C to SPI Click (SC18IS602B) | Comet BG | https://store.comet.bg/en/CatalogueRS/Product/10265597/ | 0 Comet local, 7 distributor indicated, EUR 19.817 excl VAT; silicon EOL — NOT recommended for new design |
| MIKROE-1838 Expand 2 Click (MCP23017) | Comet BG | https://store.comet.bg/CatalogueRS/Product/10054043/ | 0 Comet local, 24 distributor indicated, EUR 12.259 excl VAT; promising premade expander |
| SC18IS606-EVB | NXP/DigiKey BG | https://www.digikey.bg/en/products/detail/nxp-usa-inc/SC18IS606-EVB/15777142 | Real factory board, listing previously showed 0 in-stock; check current availability before purchase |

## Engineering constraints
- A bridge board does NOT automatically work with RF24 Arduino library: its SPI transactions are asynchronous I2C commands, not hardware SPI.transfer() calls. Need transport adaptation and timing/latency testing.
- nRF24 CE remains a dedicated output unless proven safe to move. CSN can be bridge chip select only if command framing/boot defaults verified.
- MCP23017 direction and slow requests may be moved off native GPIO, but output default is input/high impedance on reset: external pull-downs and independent contactor required; no reliance on expander alone for safe motor/solenoid state.
- Direct MDD20A Arduino stage currently expects 6 native GPIO and 20 kHz PWM. A new non-actuating HAL adapter is needed before changing direction control to I2C.
- The screenless V2 pin map remains NOT electrically approved; GPIO21 TFT backlight rejected for PWM, GPIO40–42 TFT shared, GPIO43/44 boot UART.
- Independent hardware STOP, contactor, onehot solenoid pulse, brownout and current-zero verification remain unresolved.
- No custom PCB, no second MCU, sonar deferred, official releases held.

## Decision
Reject SC18IS602B as the preferred new-design bridge due to manufacturer EOL. Investigate factory-made SC18IS606-EVB, and MIKROE-1838 as a candidate direction expander. **Do not buy yet**: module availability, bridge transport, exact V2 pin exposure, safe output defaults and independent safety must be proven.
