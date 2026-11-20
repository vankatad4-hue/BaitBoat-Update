# Remote R6.4 — Wiring Plan (Provisional)

This is a logical wiring plan, not a released schematic. Connector families and pin numbering are not frozen until component footprints and current ratings are confirmed.

## I2C expansion board

Bus from Waveshare I2C header:
- VCC: 3.3 V only (confirm expander/ADC module supports it)
- GND
- SDA: GPIO8
- SCL: GPIO9

Peripherals:
- MCP23017 button/radio GPIO expander, address straps provisionally A0=A1=A2=0 → 0x20.
- ADS1015 joystick ADC, address provisionally 0x48 (ADDR→GND).
- Verify I2C addresses by scan; account for GT911, CH422G and PCF85063A.

## MCP23017 logical pin assignment

| MCP pin | Role | Direction | Notes |
|---|---|---|---|
| GPA0 | Button matrix COL0 | input | pull-up |
| GPA1 | Button matrix COL1 | input | pull-up |
| GPA2 | Button matrix COL2 | input | pull-up |
| GPA3 | Button matrix COL3 | input | pull-up |
| GPA4 | Joystick L push | input | pull-up |
| GPA5 | Joystick R push | input | pull-up |
| GPA6 | nRF IRQ | input | interrupt |
| GPA7 | spare / arm sense | input | pull-up, reserved |
| GPB0 | Matrix ROW0 | output | scan active-low |
| GPB1 | Matrix ROW1 | output | scan active-low |
| GPB2 | Matrix ROW2 | output | scan active-low |
| GPB3 | Matrix ROW3 | output | scan active-low |
| GPB4 | nRF CE | output | radio state control |
| GPB5 | status LED / buzzer enable | output | optional, driver if needed |
| GPB6 | spare | output | reserved |
| GPB7 | spare | output | reserved |

MCP23017 INT output is open-drain; if used to signal button changes to the ESP32, route to a verified available GPIO. The current plan does not yet choose a physical GPIO for INT because the board has few unoccupied pins. Radio IRQ can be polled through the expander during the first prototype; confirm polling cadence is fast enough.

## Button matrix

The 16 physical button positions form a 4×4 matrix:
- Rows: R0–R3, driven one row low at a time; other rows high-impedance.
- Columns: C0–C3, inputs with pull-ups.
- Each key includes a diode to prevent ghosting when multiple keys are held.
- Diode direction: column side anode, row side cathode, for the active-low scan convention.
- Release-key locations map to distinct matrix coordinates and are validated in firmware; guarded release buttons require a second confirmation action.
- Debounce target: 10–25 ms, configurable.
- Power button is outside the matrix and operates the hardware latch; it is not included in the 16-key matrix.

## ADS1015 joystick ADC

| Channel | Signal |
|---|---|
| A0 | Left stick X |
| A1 | Left stick Y |
| A2 | Right stick X |
| A3 | Right stick Y |

- ADC supply/reference: 3.3 V.
- Stick potentiometer supply: 3.3 V; common ground.
- Add optional RC filtering per channel after oscilloscope/noise tests; do not choose capacitor values until sampling rate and control latency are tested.
- Use single-ended inputs; configure full-scale range for 0–3.3V (ADS1015 PGA range selection must ensure input not beyond supply).
- Calibrate center, min and max; deadband and curve are software parameters.

## nRF24 primary radio

**Provisional signals:**
- SCK/MOSI/MISO: candidate GPIO11/12/13 (currently TF-card interface; TF use disabled).
- CSN: GPIO6 candidate (Sensor AD terminal; verify circuit and boot implications).
- CE: MCP23017 GPB4.
- IRQ: MCP23017 GPA6 (poll/interrupt handling; verify timing).
- 3.3V rail: dedicated radio regulator branch, with local 100nF + 10–47µF decoupling.

The SPI pins and GPIO6 are candidate reuse only; the board schematic and physical header access must be checked. Avoid radio transmit while display update or power converter load causes rail droop.

## E22 backup radio UART

- E22 VCC: regulated 5 V branch only if exact module variant and manual support this; UART signals remain 3.3V logic.
- GND: shared signal ground.
- TXD/RXD: candidate GPIO43/44 after an isolation/selector arrangement disconnects onboard CH343 UART when radio is connected.
- AUX: optional status input, use level-safe GPIO/expander.
- Add keyed connector and labeled selector (USB-debug / E22 / isolated) or design a proper mux. Do not wire directly and allow two drivers to contend.

## Battery and hardware power key

- Pack output → pack fuse → protected power distribution.
- Battery voltage sense: divider to an ADC input; exact GPIO and divider values TBD.
- Power key: momentary front switch controls a hardware latch or board power-enable path; long-press firmware shutdown is optional but must not be the sole way to remove power.
- Ensure screen board battery connector charging circuit is not paralleled with a second charger until verified.

## Bring-up order

1. Power board and verify I2C scan.
2. Add MCP23017 only; test all 16 matrix positions and diode polarity.
3. Add ADS1015 and sweep joystick axes; verify readings do not exceed 3.3V.
4. Test nRF SPI at low TX power with radio disconnected from antenna only if permitted by module design (prefer a dummy load/antenna; never transmit PA/LNA without proper load).
5. Validate CE/CSN/IRQ behavior and command acknowledgements.
6. Isolate USB-UART and test E22 UART.
7. Run simultaneous display, button scan, ADC sampling and radio traffic; log timing, resets, rail droop and errors.

