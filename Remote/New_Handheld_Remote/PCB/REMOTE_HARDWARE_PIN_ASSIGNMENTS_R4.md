# Remote Hardware Pin Assignment R4 (logical GPIO, not header numbers)

**Physical PORT1 crosswalk remains unverified. Do not manufacture a cable/PCB from GPIO names alone.**

| ESP32-S3 GPIO | Function | Assignment/status |
|---|---|---|
| GPIO8 | I2C SDA | Shared onboard touch/RTC/CH422G + external I2C |
| GPIO9 | I2C SCL | Shared I2C |
| GPIO11 | SPI MOSI | Shared TF/nRF SPI |
| GPIO12 | SPI SCK | Shared TF/nRF SPI |
| GPIO13 | SPI MISO | Shared TF/nRF SPI |
| GPIO6 | nRF CE | Sensor AD terminal repurposed; no sensor connected; check loading |
| GPIO15 | nRF CSN | RS485 TX path repurposed; RS485 cable disconnected/transceiver inactive |
| GPIO4 | Touch IRQ | Reserved for touch |
| GPIO19/20 | USB | Reserved |
| GPIO15/16 | RS485 | GPIO15 repurposed as CSN only after RS485 isolation; GPIO16 unused |

## External I/O
- nRF IRQ_N→MCP23017 U_RADIO_IO@0x21 GPA1 (poll first).
- SC16IS750@0x48: I2C; GPIO0 M0,GPIO1 M1,GPIO2 AUX,GPIO3 NRST; UART E22 9600 8N1.
- MCP23017 buttons@0x20: GPA left8,GPB right8 active-low.
- MCP23017 radio IO@0x21: GPA0 spare,GPA1 nRF IRQ,GPA2 bridge IRQ,GPA3 RADIO_EN.
- ADS1115 axes@0x4A ADDR=SDA: AIN0 LX,AIN1 LY,AIN2 RX,AIN3 RY.
- ADS1115 monitor@0x4B ADDR=SCL: AIN0 BAT_SENSE,AIN1 NTC,AIN2 optional RADIO_3V3,AIN3 spare.
- E22 UART bridge avoids using ESP32 GPIO15/16 for UART. Module logic3.3V.
- E22 VCC=LORA_5V from dedicated TPS61023; nRF VCC=RADIO_3V3 from TPS63070.
