# Waveshare ESP32-S3-Touch-LCD-4.3 Standard — Header Verification & Remote Pin Assignment R2

**Status:** Manufacturer schematic/header mapping verified for documented board. Proposed assignments are schematic-stage, not hardware-tested. Confirm physical board revision before ordering.

## Verified PORT1 40-pin map

| Pin | Signal | Status/notes |
|---:|---|---|
| 1 | GND | Ground |
| 2 | 3V3 | Board rail; not radio supply |
| 3 | EN | Reset/enable; do not use |
| 4 | GPIO4 | Touch IRQ reserved |
| 5 | GPIO5 | LCD reserved |
| 6 | GPIO6 | Sensor AD circuit; reserved |
| 7 | GPIO7 | LCD reserved |
| 8 | GPIO15 | RS485/CAN; reserved |
| 9 | GPIO16 | RS485/CAN; reserved |
| 10 | GPIO17 | LCD reserved |
| 11 | GPIO18 | LCD reserved |
| 12 | GPIO8 | I2C SDA shared |
| 13 | GPIO19 | USB/CAN mux reserved |
| 14 | GPIO20 | USB/CAN mux reserved |
| 15 | GPIO3 | LCD reserved |
| 16 | GPIO46 | LCD/strapping reserved |
| 17 | GPIO9 | I2C SCL shared |
| 18 | GPIO10 | LCD reserved |
| 19 | GPIO11 | TF SPI MOSI; shared SPI candidate |
| 20 | GPIO12 | TF SPI SCK; shared SPI candidate |
| 21 | GPIO13 | TF SPI MISO; shared SPI candidate |
| 22 | GPIO14 | LCD reserved |
| 23 | GPIO21 | LCD reserved |
| 24 | GPIO47 | LCD reserved |
| 25 | GPIO48 | LCD reserved |
| 26 | GPIO45 | LCD/strapping reserved |
| 27 | GPIO0 | Boot strapping/LCD reserved |
| 28 | GPIO35 | Leave unused pending module memory-routing confirmation |
| 29 | GPIO36 | Leave unused pending module memory-routing confirmation |
| 30 | GPIO37 | Leave unused pending module memory-routing confirmation |
| 31 | GPIO38 | LCD reserved |
| 32 | GPIO39 | LCD reserved |
| 33 | GPIO40 | LCD reserved |
| 34 | GPIO41 | LCD reserved |
| 35 | GPIO42 | LCD reserved |
| 36 | RXD0/GPIO44 | USB-UART reserved |
| 37 | TXD0/GPIO43 | USB-UART reserved |
| 38 | GPIO2 | LCD reserved |
| 39 | GPIO1 | LCD reserved |
| 40 | GND | Ground |

## Final schematic-stage assignments
**Shared SPI:** GPIO11/PORT1-19 MOSI; GPIO12/PORT1-20 SCK; GPIO13/PORT1-21 MISO. Shared with TF card; separate chip selects, bus mutex, radio priority and chunked SD reads. Begin at conservative 1–4MHz; characterize before increasing.

**I2C:** GPIO8 SDA/PORT1-12; GPIO9 SCL/PORT1-17. Existing touch, CH422G, RTC and external connector remain. Confirm address scan and pullups; do not add pullups blindly.

**ADS1115 @0x48:** AIN0 Left X, AIN1 Left Y, AIN2 Right X, AIN3 Right Y; ADDR to GND; ALERT/RDY unused. Keep inputs within supply rails. Axis scan/control latency requires testing.

**MCP23017 @0x20 buttons:** GPA0–7 left bank; GPB0–7 right bank; normally-open buttons to GND active-low, internal pullups, poll/debounce.

**MCP23017 @0x21 radio-control candidate:** GPA0 CE; GPA1 CSN; GPA2 IRQ_N input; remaining pins spare. CE/CSN via I2C expander timing must be bench-validated. If latency or RF performance is inadequate, revise before firmware freeze.

**Battery monitor:** GPIO6 is tied to Sensor AD circuit and not assigned. Battery voltage sensing remains open pending review or ADC allocation change.

## Draft 8-pin radio connector
1 GND; 2 RADIO_3V3 (TPS63070 rail); 3 SCK; 4 MOSI; 5 MISO; 6 CSN (MCP23017@0x21 GPA1); 7 CE (GPA0); 8 IRQ (GPA2). Connector type/keying/pin-1 marking TBD. Do not use an adapter module as a supply unless its regulator is verified.

## Net labels
RADIO_SCK=GPIO12; RADIO_MOSI=GPIO11; RADIO_MISO=GPIO13; RADIO_CSN=IOX_RADIO_GPA1; RADIO_CE=IOX_RADIO_GPA0; RADIO_IRQ_N=IOX_RADIO_GPA2; JOY_LX/LY/RX/RY=ADS AIN0–AIN3; BTN_L[0..7]=MCP@0x20 GPA; BTN_R[0..7]=MCP@0x20 GPB; I2C_SDA=GPIO8; I2C_SCL=GPIO9.

## Required validation
Physical revision/header orientation; GPIO35–37 module memory use; I2C scan/pullups/capacitance; shared SPI/SD isolation; expander CE/CSN/IRQ timing and RF throughput; radio rail transient/thermal tests; Sensor-AD circuit; native schematic/ERC and review.

Sources: official Waveshare docs https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3 and schematic https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3/ESP32-S3-Touch-LCD-4.3-Sch.pdf
