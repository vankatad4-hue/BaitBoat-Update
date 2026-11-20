# Remote Peripheral Interface Pinout Draft — superseded by R2

Current schematic-stage assignments are in `WAVESHARE_HEADER_MAPPING_AND_PIN_ASSIGNMENTS_R2.md`.

- Shared SPI: GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO (TF-card bus; serialized, separate CS).
- I2C: GPIO8 SDA, GPIO9 SCL.
- ADS1115 @0x48: AIN0 L-X, AIN1 L-Y, AIN2 R-X, AIN3 R-Y.
- MCP23017 @0x20: 16 active-low button inputs.
- MCP23017 @0x21: GPA0 CE, GPA1 CSN, GPA2 IRQ_N candidate. Expander radio-control timing must be bench-validated.
- GPIO6/Sensor AD is occupied; battery-monitor assignment is open.

This supersedes the earlier tentative GPIO15/GPIO43 control assignments. No hardware validation yet.
