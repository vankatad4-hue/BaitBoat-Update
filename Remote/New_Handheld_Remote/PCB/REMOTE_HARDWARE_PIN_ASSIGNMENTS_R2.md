# Remote Hardware Pin Assignment R2 — test harness proposal

**Target:** Waveshare ESP32-S3-Touch-LCD-4.3 Standard (SKU 25948). The GPIO functions below are from official board documentation; physical PORT1 header numbers still require schematic revision/continuity confirmation. R2 uses an I2C UART bridge to avoid allocating a direct UART on the board's RS485/CAN/USB-multiplexed pins.

## Fixed board functions
| GPIO | Function | Keep reserved? |
|---|---|---|
| 0,1,2,3,5,7,10,14,17,18,21,38–42,45–48 | RGB LCD and timing | Yes |
| 4 | Touch IRQ | Optional IRQ; can poll GT911 if repurposed but do not repurpose in R2 |
| 8,9 | I2C SDA/SCL | Shared bus |
| 11,12,13 | TF-card SPI MOSI/SCK/MISO | Shared SPI bus |
| 15,16 | RS485 UART pins | Reserved; not used for LoRa UART |
| 19,20 | USB/CAN mux | Reserved |
| 6 | Sensor AD terminal | Reassigned as direct nRF24 CSN only if sensor terminal unused; confirm analog circuit and PORT1 crosswalk |

## Controller bus assignment
- I2C: GPIO8 SDA / GPIO9 SCL, 400kHz target only after rise-time/pullup check.
- SPI: GPIO11 MOSI / GPIO12 SCK / GPIO13 MISO, shared TF card. Start nRF SPI at 1MHz for bring-up, increase only after test.
- nRF24 CSN: GPIO6 candidate (sensor AD terminal repurposed, no sensor attached); direct GPIO is required for low-latency CSN.
- nRF24 CE: MCP23017 U_RADIO_IO @0x21 GPA0. Bench timing test; if fails, move to direct GPIO once exact connector map confirms spare.
- nRF24 IRQ: U_RADIO_IO GPA1 input; active-low; poll expander at 1kHz maximum or route to available MCU interrupt pin if confirmed.
- LoRa E22 UART: NXP SC16IS750 I2C-UART bridge U_LORA @0x48; UART 9600 8N1 initial.
- E22 M0/M1/AUX/NRST: SC16IS750 GPIO0/GPIO1/GPIO2/GPIO3. M0=M1=LOW for normal transparent mode. AUX input; RESET output with reset pull-up.
- SC16IS750 IRQ: MCP23017 U_RADIO_IO GPA2 or MCU interrupt if physical GPIO available.
- U_BUTTON MCP23017 @0x20: GPA0–7 left bank, GPB0–7 right bank.
- U_RADIO_IO MCP23017 @0x21: GPA0 CE, GPA1 nRF IRQ, GPA2 SC16IS750 IRQ, GPA3 RADIO_EN (LoRa module supply switch candidate), GPB reserved.
- ADS1115 joystick U_AXIS @0x4A (ADDR tied SDA): AIN0 LX, AIN1 LY, AIN2 RX, AIN3 RY.
- ADS1115 monitor U_MON @0x4B (ADDR tied SCL): AIN0 BAT_SENSE, AIN1 NTC, AIN2 RADIO_3V3 sense (optional), AIN3 spare.
- SC16IS750 I2C address: strap A0/A1 per datasheet to select 0x48; verify address pins and board-level pullups. I2C UART bridge IRQ can be polled; IRQ pin must not be left floating.
- I2C device address scan before any peripheral is plugged. Existing Waveshare touch/RTC/CH422G addresses may coexist; confirm on board revision.

## R2 allocation is test-build, not frozen PCB
GPIO6 may be connected to Sensor AD circuitry and GPIO15/16 to RS485 transceiver; nRF CSN GPIO6 must be checked for load/boot interactions. CE via MCP23017 must be bench-tested for RF24 timing. If either fails, allocate direct GPIO through verified PORT1 header or revise to SPI/I2C bridge. The exact 40-pin physical crosswalk is the remaining controller pin freeze blocker.
