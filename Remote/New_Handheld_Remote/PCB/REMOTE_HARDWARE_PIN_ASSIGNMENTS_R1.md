# Remote Hardware Pin / Interface Assignment R1

**Target board:** Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948. **Status: logical assignments frozen; exact physical PORT1 pin crosswalk and electrical coexistence are validation blockers.** Do not manufacture final wiring harness or PCB until board revision schematic and continuity test agree.

## Existing board-owned GPIO (do not repurpose)
| GPIO | Existing function | Constraint |
|---|---|---|
| 0,1,2,3,5,7,10,14,17,18,21,38–42,45–48 | LCD RGB / timing | Reserved |
| 4 | Touch IRQ | Touch can possibly be polled if IRQ is reassigned; validate |
| 8 | I2C SDA | Shared touch/expander/ADC bus |
| 9 | I2C SCL | Shared touch/expander/ADC bus |
| 11 | TF MOSI | Shared SPI |
| 12 | TF SCK | Shared SPI |
| 13 | TF MISO | Shared SPI |
| 15 | RS485 TX / CAN mux-related | Do not assign without checking transceiver path |
| 16 | RS485 RX / CAN mux-related | Do not assign without checking transceiver output contention |
| 19,20 | USB/CAN mux | Reserved unless board mux mode and USB behavior validated |
| 6 | Sensor AD | Analog/sensor terminal; can be repurposed only after confirming circuit/loading |

GPIOs 22–37 are not assumed available; module flash/PSRAM and board-specific routing may reserve them. GPIO43/44 are not assumed free; USB-UART/console cross-check required.

## Logical peripheral allocation (carrier)
| Peripheral | Logical pins / signals | Preferred connection | Notes |
|---|---|---|---|
| I2C bus | SDA=GPIO8, SCL=GPIO9 | Waveshare I2C connector / confirmed PORT1 pins | Existing pull-ups must be measured; bus scan and voltage verify |
| ADS1115 joystick | 0x48 | I2C | ADDR=GND; four axes |
| ADS1115 monitor | 0x49 | I2C | ADDR=3V3; battery/NTC/optional radio rail |
| MCP23017 button | 0x20 | I2C | 16 button inputs |
| MCP23017 radio control | 0x21 | I2C | CE, radio power enable, IRQ status and auxiliary controls; chip-select must not be driven through I2C for timing-critical nRF transactions |
| nRF24 SPI | MOSI GPIO11, SCK GPIO12, MISO GPIO13 | shared TF bus | TF-card access and radio share SPI; bus mutex required |
| nRF24 CSN | direct ESP GPIO, TBD | low-latency direct output | NOT I2C expander; exact available pin/crosswalk TBD |
| nRF24 CE | direct ESP GPIO, TBD | direct output | Required for RF24 TX/RX state transitions; no slow expander control |
| nRF24 IRQ | GPIO input, TBD or expander input | direct interrupt preferred | Optional polling only if latency test supports |
| LoRa UART | ESP UART TX/RX pair, TBD | direct MCU UART | No UART GPIO selected until RS485/CAN/USB path and PORT1 crosswalk checked |
| LoRa M0/M1 | two GPIO or fixed strap | module mode controls | Must never float; strap normal transparent mode if config workflow approved |
| LoRa AUX | input | direct GPIO/interrupt | Observe busy/ready; don't block waiting |
| LoRa RESET | output or RC/reset pad | direct GPIO or controlled reset | Optional but recommended |
| Power monitoring | ADS1115 | I2C | ADC channels described in schematic |

## Connector pinouts — external carrier
### J_I2C (4-pin, keyed)
1. 3V3_CONTROLLER (only after header voltage verified)
2. GND
3. SDA
4. SCL

### J_SPI_RADIO (6-pin, keyed)
1. GND
2. 3V3_CONTROLLER
3. MOSI
4. SCK
5. MISO
6. CSN (separate direct GPIO)

CE and IRQ use separate direct GPIO connector pins or dedicated radio-control header; do not combine with I2C expander on production layout.

### J_LORA_MODULE (module-specific castellated footprint or test socket)
- VCC: filtered 5V candidate (module accepts 2.7–5.5V, but 5V required to guarantee maximum output power per manual)
- GND
- UART_RXD: MCU TX
- UART_TXD: MCU RX
- M0, M1: defined normal-mode levels
- AUX
- NRST (recommended)
- ANT: 50-ohm feed to RIGHT antenna exit
The E22-900T22S has a 26-pad/1.27mm footprint; pin numbering/land pattern must come from the exact selected datasheet/revision, not a generic connector.

### J_ANT
- LEFT = nRF24 2.4GHz, 50Ω coax/antenna
- RIGHT = LoRa 863–870MHz regional antenna, 50Ω coax/antenna
- ESP32 Wi-Fi/BLE: onboard antenna

## GPIO freeze gate
1. Identify board silkscreen SKU/revision and connector orientation.
2. Obtain the matching official schematic.
3. Confirm PORT1 connector number ↔ GPIO with schematic AND continuity/voltage measurements.
4. Confirm that UART candidate pins do not conflict with the onboard SP3485, CAN/USB multiplexer, USB-UART or touch IRQ.
5. Confirm unused GPIO strapping behavior and boot levels.
6. Update this document with actual connector pin numbers; then freeze harness/PCB.
