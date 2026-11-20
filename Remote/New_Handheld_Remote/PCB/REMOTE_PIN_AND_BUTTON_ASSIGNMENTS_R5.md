# Remote Pin/Expander Allocation R5

**Logical assignment only. Verify exact Waveshare PCB revision and connector crosswalk before wiring.**

## I2C
- ESP32-S3 I2C: GPIO8 SDA, GPIO9 SCL (provisional).
- MCP23017 buttons: 0x20.
- MCP23017 radio/confirm I/O: 0x21.
- ADS1115 joystick: 0x4A.
- ADS1115 monitor: 0x4B.
- SC16IS750: 0x48.

## MCP23017 @ 0x20 — physical buttons
| Port | Bit | Physical | Active |
|---|---:|---|---|
| GPA0–GPA7 | 0–7 | L1–L8, left bank | Low |
| GPB0–GPB7 | 8–15 | R1–R8, right bank | Low |

All key inputs use pull-ups; switch closure connects input to ground. Software debounce 25ms. Physical scanner polls and dispatches press/release transitions.

## MCP23017 @ 0x21 — radio I/O + OK
| Port | Bit | Function | Direction |
|---|---:|---|---|
| GPA0 | 0 | Spare | Reserved |
| GPA1 | 1 | nRF IRQ | Input |
| GPA2 | 2 | SC16 bridge IRQ | Input |
| GPA3 | 3 | RADIO_EN | Output candidate |
| GPA4 | 4 | Separate OK/confirm key | Input, pull-up, active-low |
| GPA5–GPA7 | 5–7 | Spare | Reserved |
| GPB0–GPB7 | 8–15 | Spare | Reserved |

Scanner configures GPA4 as input and enables its pull-up while preserving other direction/pull-up bits. Confirm is independent of the 16-key bank.

## Release event mapping
R1 GPIOB0 = left hopper; R2 GPIOB1 = right hopper; R3 GPIOB2 = left hook; R4 GPIOB3 = right hook. Each release requires 1.2s key hold, release, then separate OK 0.8s hold within 4s. No release is allowed on backup LoRa or while boat interlock status is unknown.

The scanner source and command guard are host-syntax/test checked. Physical I2C behavior, pin loading, expander conflict and actual PCB wiring are not verified.
