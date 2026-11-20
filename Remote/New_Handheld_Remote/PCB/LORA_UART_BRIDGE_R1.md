# LoRa UART bridge electrical and firmware design R1

## Why a bridge was added
Waveshare GPIO resources are tight and GPIO15/16 are wired to RS485; GPIO19/20 are USB/CAN muxed. Direct UART on those pins would risk electrical contention. R2 uses NXP SC16IS750 as an I2C-to-UART bridge to free ESP pins and keep the E22 serial interface.

## U_SC16IS750
- I2C/SPI select pin HIGH selects I2C mode.
- A0 and A1 both HIGH selects 7-bit address 0x48 (datasheet table displays 8-bit 0x90).
- VDD 3.3V; GND.
- 100nF local decoupling and 1µF near device.
- I2C SDA/SCL shared controller bus.
- 1.8432MHz crystal between XTAL1 and XTAL2 candidate for exact 9600 baud divisor; crystal tolerance ±100ppm candidate. Load capacitors calculate from crystal load capacitance and PCB parasitic, not arbitrarily set.
- UART TX → E22 RXD; E22 TXD → UART RX.
- GPIO0=M0, GPIO1=M1, GPIO2=AUX input, GPIO3=NRST output. GPIO configuration reset state and M0/M1 pull-downs must be safe.
- IRQ active-low to MCP23017 0x21 GPA2 or direct verified MCU GPIO.
- UART: 9600 8N1 initial; E22 configured through documented AT/config mode, return to transparent mode after verified settings.

## Address plan
| Device | 7-bit address |
|---|---:|
| MCP23017 button | 0x20 |
| MCP23017 radio controls | 0x21 |
| SC16IS750 UART bridge | 0x48 |
| ADS1115 joystick | 0x4A (ADDR tied SDA) |
| ADS1115 monitor | 0x4B (ADDR tied SCL) |

This is a proposed map; scan the Waveshare onboard bus and check address conflicts before soldering.

## Performance caveat
I2C bridge is a pin-saving compromise; it is not ideal for high-rate serial. Use 400kHz I2C only if bus rise time and onboard devices allow. Bounded FIFO reads, task priorities and latency measurement are required. If it cannot reliably service LoRa packet bytes/ACK timing, move to a direct UART on verified spare pins or SPI bridge. Do not let radio bridge polling block LVGL.
