# Remote BOM R3 — physical-control revision

This revision adds the R5 button/control layout. It is still a candidate BOM; do not order PCB assembly until exact board revision, footprints, wiring and current/thermal checks are complete.

| Ref | Qty | Part/category | Function | Status |
|---|---:|---|---|---|
| U_CTRL | 1 | Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948 | Controller/display | Exact PCB revision and GPIO crosswalk verify |
| U_NRF | 1 | nRF24L01+ PA/LNA breakout | Primary 2.4GHz RF | Module/revision and supply current validate |
| U_LORA | 1 | EBYTE E22-900T22S UART LoRa | Backup EU868 radio | Exact revision and legal settings verify |
| U_UART | 1 | SC16IS750 | I2C-to-UART for E22 | Candidate; clock and address verify |
| U_AXIS | 1 | ADS1115 | Four joystick axes | Address0x4A candidate |
| U_MON | 1 | ADS1115 | Battery/NTC monitoring | Address0x4B candidate |
| U_BUTTONS | 1 | MCP23017 | 16 physical buttons | Address0x20; inputs/pull-ups |
| U_RADIO_IO | 1 | MCP23017 | Radio/bridge I/O plus OK input | Address0x21; GPA4 is dedicated OK/confirm input |
| BTN_SW | 16 | Low-profile momentary switches | Two banks, 8 per side | Footprint/travel/ratings verify |
| BTN_OK | 1 | Momentary tactile switch | Separate release-confirm key | Wired to U_RADIO_IO GPA4, active-low |
| BTN_PWR | 1 | Power switch / power-button mechanism | Remote power | Mechanical/electrical architecture TBD |
| JOY | 2 | Dual-axis potentiometer joysticks | Left/right thumb control | Exact model and ADC range verify |
| CELL | 4 | Matched 18650 cells | 1S4P battery | Cell model/pack construction TBD |
| BMS | 1 | 1S protection | Pack protection | Current/rating and charging validation |
| F_MAIN | 1 | Main fuse | Pack short protection | Rating/coordination TBD |
| Q_REV | 1 | Reverse-polarity protection | Battery input | Candidate only |
| U_RADIO_PWR | 1 | TPS63070 | nRF 3.3V rail | Reference design/layout validation |
| U_LORA_PWR | 1 | TPS61023 | E22 5V rail | Reference design/layout validation |
| ENC | 1 | R5 front/rear 3D-printed enclosure | Handheld shell | 310×209×69.5mm prototype; fit unverified |

## R5 key assignment
- Left bank L1–L8: STOP, manual/autopilot, return-home, waypoint-save placeholder, waypoint-select placeholder, map full-screen, sonar full-screen, settings placeholder.
- Right bank R1–R4: hopper-left, hopper-right, hook-left, hook-right — each independent.
- Right bank R5–R8: status/lock/map-split/back-cancel UI-only.
- Separate OK button is used by the two-step release guard.

See `REMOTE_PHYSICAL_BUTTON_WIRING_R5.csv` and `MECHANICAL/PHYSICAL_BUTTON_MAP_R5.md`.
