# Remote Power + Controls Block Diagram R1

This block diagram is architectural only. It is not a circuit schematic, pin-complete netlist or ERC result.

```mermaid
flowchart TD
  PACK[1S4P Li-ion pack] --> BMS[Pack BMS]
  BMS --> FUSE[4A fuse candidate]
  FUSE --> QREV[Reverse polarity MOS candidate]
  QREV --> BAT[ BAT_PROT ]
  BAT --> WAVE[Waveshare battery input]
  WAVE --> BOOST[Onboard CS8501 boost]
  BOOST --> SYS5[Board 5V system]
  SYS5 --> MCU[ESP32-S3 + display + touch]
  BAT --> RADIOREG[TPS63070 buck-boost]
  RADIOREG --> R3V3[RADIO_3V3]
  R3V3 --> NRF[nRF24 PA/LNA]
  BAT --> BATSENSE[220k/68k divider + filter]
  BATSENSE --> GPIO6[GPIO6 ADC candidate]
  MCU3V3[Controller 3V3] --> NTCDIV[NTC divider/filter]
  NTCDIV --> NTCADC[ADC input candidate]
  MCU --> I2C[I2C GPIO8/9]
  I2C --> ADC[ADS1115 4-axis ADC]
  ADC --> JOY[2 joystick modules]
  I2C --> EXP[MCP23017 16-bit expander]
  EXP --> BUTTONS[16 physical buttons]
  MCU --> SPI[SPI GPIO11/12/13 shared TF]
  SPI --> SD[TF card]
  SPI --> NRF
```

## Key constraints
- Onboard controller boost is baseline; external TPS61023 is a fallback only.
- Radio has its own buck-boost rail.
- ADS1115 and MCP23017 share the board I2C bus with onboard peripherals; addresses/pullups must be verified.
- TF card and nRF share SPI; software must arbitrate and prioritize radio.
- GPIO/connector assignments remain provisional.
