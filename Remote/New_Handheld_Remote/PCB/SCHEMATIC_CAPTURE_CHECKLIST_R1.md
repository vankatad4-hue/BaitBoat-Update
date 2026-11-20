# Native KiCad Capture Checklist R1

## Sheet capture status
- [ ] 00 System overview
- [ ] 01 Battery input/protection
- [ ] 02 Controller battery input (Waveshare onboard boost)
- [ ] 03 TPS63070 radio rail
- [ ] 04 ADS1115 joystick ADC
- [ ] 05 MCP23017 buttons
- [ ] 06 MCP23017 radio controls — experimental
- [ ] 07 Shared SPI radio + TF
- [ ] 08 connectors/testpoints

## Pin/package checks
- [ ] F_MAIN footprint and fuse DC interrupt/I²t/derating verified.
- [ ] FDS6575 SO-8 pin numbering, D/S orientation and body diode verified against current datasheet.
- [ ] TPS63070 package/pin mapping, VSEL/FB2/PS-SYNC and EN treatment checked against datasheet.
- [ ] ADS1115 footprint, address straps, input range, bypass and analog RC reviewed.
- [ ] MCP23017 package, A0-A2 addresses, RESET, pullups, interrupt pins and voltage checked.
- [ ] Connector part/pin-1/keying/current ratings selected.
- [ ] Waveshare board exact revision and PH2.0 battery polarity verified.

## Functional checks
- [ ] No competing 5V boost output; external TPS61023 not populated.
- [ ] Reverse protection path simulation/bench checked.
- [ ] Battery monitor divider worst-case input voltage/ADC range.
- [ ] NTC curve/open/short behavior reviewed.
- [ ] I2C address scan and pullups verified.
- [ ] Joystick channels simultaneously sampled and latency measured.
- [ ] Radio CE/CSN via MCP23017 timing benchmarked; fallback decision recorded.
- [ ] Radio and TF SPI bus arbitration tested.
- [ ] Radio PA/LNA peak supply current/droop tested.
- [ ] Power-on/reset state outputs safe (radio off; button inputs high).
- [ ] ERC run and all warnings reviewed; footprints audited.
- [ ] Independent schematic review.
- [ ] PCB layout/DFM/RF review before fabrication.
