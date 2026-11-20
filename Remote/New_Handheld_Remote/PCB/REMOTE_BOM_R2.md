# Remote Hardware BOM R2 — revised prototype

| Ref | Qty | Part/category | Function | Status |
|---|---:|---|---|---|
| U_CTRL | 1 | Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948 | Display/controller | Confirm board revision |
| U_NRF | 1 | nRF24L01+ PA/LNA breakout | Primary RF | Exact variant/connector and power current validate |
| U_LORA | 1 | EBYTE E22-900T22S UART/SX1262 module | Backup RF | Candidate; exact EU variant and channel/power legality confirm |
| U_UART | 1 | NXP SC16IS750 TSSOP24 or HVQFN24 | I2C-UART bridge to LoRa module; 64B FIFO | Add to carrier; address 0x48; clock/crystal required |
| X_UART | 1 | 1.8432MHz crystal, tolerance ≤100ppm | UART bridge clock | Load caps selected from crystal CL + parasitic |
| C_XTAL | 2 | Crystal load capacitors | Clock oscillator | Calculate; candidate 18–22pF range only |
| U_AXIS | 1 | ADS1115 | 4 joystick axes | I2C 0x4A, ADDR=SDA |
| U_MON | 1 | ADS1115 | Pack/NTC/radio rail | I2C 0x4B, ADDR=SCL |
| U_BTN | 1 | MCP23017 | 16 buttons | I2C 0x20 |
| U_RADIO_IO | 1 | MCP23017 | CE/IRQ/bridge IRQ/enable | I2C 0x21; CE-expander timing test |
| U_RADIO_PWR | 1 | TPS63070 | nRF 3V3 buck-boost | Validate TI reference circuit, loop, ratings |
| L_RADIO | 1 | 1.5µH high Isat inductor | TPS63070 | Candidate Coilcraft XFL4020-152ME; verify |
| F_MAIN | 1 | 4A fuse candidate | Pack short protection | Recalculate and coordinate with pack |
| Q_REV | 1 | P-MOS reverse protection candidate | Reverse polarity | FDS6575 candidate; pin/thermal verification |
| C_BAT | set | 100µF/10µF/100nF | Pack rail decoupling | Rating/ESR/layout validate |
| C_NRF | set | 100nF+10µF+22–47µF | nRF burst bypass | Scope transient |
| C_LORA | set | 100nF+10µF+100µF | E22 module decoupling | Voltage rating ≥ pack max; scope transient |
| R_BAT | 2 | 220k/68k | Battery sense | 4.2V→0.977V |
| NTC | 1 | 10k B3950 candidate | Battery temperature | Curve/placement/open-short detection validate |
| JOY | 2 | Dual-axis potentiometer joysticks | Manual axes | Choose exact footprint and 3.3V-compatible module |
| BTN | 16 | Tactile switches/caps | Physical buttons | Low profile, cap dimensions and durability |
| CELL | 4 | Matched 18650 | 1S4P battery | Cell model and pack construction safety review |
| BMS | 1 | 1S protection/PCM | Pack protection | Current rating, short protection and connector verify |
| J_PACK | 1 | Keyed ≥4A connector | Pack input | Choose connector/cable gauge |
| ANT_NRF | 1 | 2.4GHz 50Ω antenna | LEFT | Match SMA/IPEX coax |
| ANT_LORA | 1 | EU868 50Ω antenna | RIGHT | Antenna EIRP/ERP gain and feed loss calculate |
| ENC | 1 | Custom shell | Ergonomic enclosure | R2 envelope target 300×210×70mm fit-check |

**Do not order PCB assembly yet.** Pin crosswalk, footprints, RF legality, electrical tests and KiCad ERC/DRC are not complete.
