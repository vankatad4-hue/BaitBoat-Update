# Remote Hardware Block and Wiring Diagrams R1

## Power distribution
```mermaid
flowchart LR
 PACK[1S4P Pack] --> F[F_MAIN]
 F --> Q[Reverse protection]
 Q --> BAT[BAT_PROT]
 BAT --> W[Waveshare battery input]
 BAT --> TPS[TPS63070]
 TPS --> R3[RADIO_3V3]
 R3 --> NRF[nRF24 PA/LNA]
 BAT --> LBOOST[TPS61023 dedicated 5V boost]
 LBOOST --> LORA[E22 LoRa VCC]
 BAT --> MON[Battery monitor ADC]
```

LoRa module accepts 2.7–5.5V; 5V guarantees output power. A dedicated TPS61023 5V branch powers E22 only, so the CS8501 controller/display rail stays within its board-load specification. This is a separate load/converter, not parallel outputs. nRF has separate 3.3V converter.

## Controller and I/O
```mermaid
flowchart TD
 ESP[Waveshare ESP32-S3] --> I2C[I2C GPIO8/9]
 I2C --> BTN[MCP23017 0x20: 16 buttons]
 I2C --> GPIO[MCP23017 0x21: radio IRQ/enable]
 I2C --> AX[ADS1115 0x4A: joystick axes]
 I2C --> MON[ADS1115 0x4B: battery/NTC]
 I2C --> UART[SC16IS750 0x48: UART bridge]
 UART --> E22[E22 LoRa UART]
 ESP --> SPI[SPI GPIO11/12/13]
 SPI --> SD[TF card]
 SPI --> NRF[nRF24]
 ESP --> CSN[GPIO15: nRF CSN]
 ESP --> CE[GPIO6: nRF CE]
```

GPIO6 repurposes Sensor AD; GPIO15 repurposes RS485 TX. RS485 bus must be disconnected/disabled. Exact header crosswalk and continuity verification still required.

## RF connectors
- LEFT: 2.4GHz nRF24 PA/LNA antenna.
- RIGHT: 868MHz LoRa antenna.
- ESP32 Wi-Fi/Bluetooth: onboard antenna.
