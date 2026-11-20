# Remote R6.4 — I/O Architecture and Pin Allocation Strategy

**Status:** preliminary architecture; exact GPIO assignment is intentionally not frozen.

## Critical constraint

The selected Waveshare ESP32-S3-Touch-LCD-4.3 board dedicates many ESP32 GPIOs to its RGB display, touch I2C, USB, TF card and onboard CAN/RS485. It is not a bare ESP32-S3 carrier with every GPIO available. The project has 16 physical button positions, two analog joysticks (four analog axes), joystick push-switches, two radio interfaces, power-status signals and other future controls. Direct GPIO allocation is therefore likely to be insufficient or unnecessarily fragile.

Official pin mapping/reference:
https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3

## Proposed architecture

### A. External button I/O expander
Candidate: MCP23017 (16 GPIO over I2C) or equivalent.
- Connect all 16 front button positions as active-low inputs with pull-ups.
- Button bank wiring as two keyed 8-position harnesses; use a ground return and avoid long unprotected parallel runs.
- Use hardware/software debounce.
- If OK and power are physical dedicated controls outside the 16-position bank, reserve extra expander GPIOs or use a dedicated MCU GPIO only after pin mapping.
- Release controls (bait/hook) must have independent inputs and guarded physical switches. Firmware must require explicit confirmation/arming, and a link-loss failsafe must never trigger release.

### B. Joystick analog acquisition
Candidate: ADS1015 four-channel ADC over I2C (or a verified board-accessible set of ADC1 pins if four channels can be exposed cleanly).
- 4 analog inputs for X/Y from two sticks.
- Power joystick potentiometers from 3.3V and reference the ADC to the same clean rail.
- Calibrate each axis center/min/max; apply deadband and smoothing.
- 100–200 Hz control sampling target is a preliminary requirement; validate ADC choice and I2C traffic.
- Joystick push switches can be routed through the button expander.

### C. Primary radio interface
nRF24L01+ PA/LNA uses SPI plus CE and CSN; IRQ is optional but recommended if pin budget permits.
- SPI lines may be shared with other SPI devices only with separate chip selects and verified bus timing.
- Keep radio SPI wiring short and physically separated from switching regulator and display flex.
- Avoid using GPIOs occupied by LCD, touch, USB, TF, onboard CAN/RS485 without verifying the schematic and firmware configuration.

### D. Backup E22 UART interface
Reserve one hardware UART RX/TX pair for the E22. UART logic is 3.3V. Add a connector with power, ground, TX, RX and optional AUX. Configure radio as backup/safety path and test radio switching behavior independently.

### E. Power/safety inputs
Reserve lines for:
- physical power key / power latch, if a latching scheme is used;
- battery voltage monitor (through a safe divider);
- charge-state/USB presence (optional, only if documented signals are accessible);
- watchdog/fault indication (optional);
- release-control arm state (if used).

## Proposed logical net list (not physical GPIO numbers)

| Signal group | Signals | Proposed interface |
|---|---|---|
| Button matrix/inputs | 16 buttons + OK/joystick press + safety/arm | MCP23017 GPIO expander / spare GPIO |
| Joystick analog | L-X, L-Y, R-X, R-Y | ADS1015 4-channel ADC or exposed ADC pins |
| Joystick switches | L-SW, R-SW | Expander inputs |
| nRF radio | SCK, MOSI, MISO, CE, CSN, optional IRQ | SPI + GPIO |
| E22 radio | UART TX, RX, optional AUX | UART |
| Battery monitor | VBAT sense | ADC input via divider |
| Power control | POWER_KEY / ENABLE | Hardware power-latch circuit plus logic input |
| Status | buzzer/LED or fault | GPIO / expander |

## Addressing and shared bus

- The board's I2C bus is GPIO8 SDA / GPIO9 SCL and is already used by touch and onboard I/O expansion.
- Verify the exact I2C address of the touch controller and board I/O expander before adding peripherals. Check address conflicts, pull-up strength and bus capacitance.
- ADS1015 and MCP23017 candidates have configurable addresses, but address straps must be defined in schematic.
- No pinout is final until exact Waveshare board revision, schematic, connector-accessible GPIO list, and all onboard peripherals are reconciled.

## Firmware safety requirements

- Button events must be debounced and edge-detected.
- Release buttons are guarded and require a deliberate confirmation workflow.
- A joystick ADC failure, out-of-range reading, or lost radio link must produce neutral motor commands; autopilot failover behavior is handled separately from manual control.
- Primary/backup radio switching must not permit duplicate release commands.
- Establish a command sequence counter, acknowledgement and duplicate suppression for release commands.

## Next design gate

Review the exact Waveshare schematic and determine accessible GPIOs. Then produce the physical pin map, connector pinout, expander/ADC schematic, and bring-up test list. Keep this architecture separate from the final PCB design until verified.
