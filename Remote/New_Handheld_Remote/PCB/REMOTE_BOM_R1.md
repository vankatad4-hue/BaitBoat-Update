# Remote Hardware BOM R1 — prototype candidates

**All parts remain candidates until exact datasheets, footprints, stock, connector pinouts and tests are verified.**

| Ref | Qty | Part / category | Purpose | Status / notes |
|---|---:|---|---|---|
| U_CTRL | 1 | Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948 | Display/controller | Existing target; board revision confirm |
| U_NRF | 1 | nRF24L01+ PA/LNA module with SMA/IPEX | Primary radio | Module variants differ; 3.3V burst power validated |
| U_LORA | 1 | EBYTE E22-900T22S SX1262 UART | Backup radio | Candidate; EU868 configure, antenna pigtail; 5V supply recommended for full RF output |
| ANT_NRF | 1 | 2.4GHz 50Ω antenna | LEFT antenna | Match connector/pigtail |
| ANT_LORA | 1 | EU868 50Ω antenna | RIGHT antenna | EIRP includes gain and feed loss |
| U_AXIS | 1 | ADS1115 | 4 joystick axes | I2C 0x48 |
| U_MON | 1 | ADS1115 | battery/temp/rail | I2C 0x49 |
| U_BTN | 1 | MCP23017 | 16 physical buttons | I2C 0x20 |
| U_RADIO_IO | 1 | MCP23017 | aux radio control/status | I2C 0x21; not CSN/CE timing unless test proves |
| U_RADIO_PWR | 1 | TPS63070 | 3.3V nRF buck-boost | Validate exact TI topology/values/layout |
| L_RADIO | 1 | 1.5µH inductor, high saturation current | Converter | Candidate XFL4020-152ME; check saturation and footprint |
| F_MAIN | 1 | 4A fuse candidate | Battery protection | Recalculate using load, BMS and I²t |
| Q_REV | 1 | P-channel MOS reverse protection | Reverse polarity | FDS6575 candidate; validate pinout/Rds/thermal |
| R_BAT | 2 | 220k/68k | Pack voltage divider | Battery ADC |
| NTC | 1 | 10k B3950 thermistor | Pack temperature | Validate placement/curve |
| J_PACK | 1 | Keyed ≥4A connector | Pack input | Choose connector/cable gauge |
| J_RADIO | 2 | Keyed/locking connector | Radio interconnect | Footprint/pinout frozen after GPIO allocation |
| J_ANT | 2 | SMA bulkhead + matched coax pigtail | Antenna exits | RIGHT 868MHz / LEFT 2.4GHz |
| C_RADIO | set | 10–47µF + 100nF nRF; LoRa 100µF+10µF+100nF | RF decoupling | Measure transients; choose ESR/voltage ratings |
| JOY | 2 | Dual-axis joystick modules | Manual control | Exact module and mechanical model pending |
| BTN | 16 | Low-profile tactile switches/caps | Buttons | Exact footprint/cap and durability pending |
| CELL | 4 | Matched 18650 Li-ion cells | 1S4P battery | Cell brand/model not selected; pack/BMS design required |
| BMS | 1 | 1S protection/PCM | Pack protection | Ratings and balancing architecture reviewed; single-cell parallel pack |
| ENC | 1 | Custom printed enclosure | Ergonomics | Proposed envelope 300×205×68mm; fit-check before freeze |
