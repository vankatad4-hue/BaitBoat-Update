# Remote R6.4 — I/O Architecture and Pin Allocation Strategy

**Status:** preliminary architecture; exact GPIO assignment is intentionally not frozen.

## Critical constraint

The selected Waveshare ESP32-S3-Touch-LCD-4.3 board dedicates many ESP32 GPIOs to its RGB display, touch I2C, USB, TF card and onboard CAN/RS485. It is not a bare ESP32-S3 carrier with every GPIO available. The project has 16 physical button positions, two analog joysticks (four analog axes), joystick push-switches, two radio interfaces, power-status signals and other future controls. Direct GPIO allocation is therefore likely to be insufficient or unnecessarily fragile.

Official pin mapping/reference:
https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3

## Verified board connections (Waveshare 4.3 touch model)

The official documentation and schematic confirm the following:
- GPIO8/9 are the shared I2C SDA/SCL bus and appear at the external I2C connector; touch uses the same bus.
- GPIO4 is touch IRQ; avoid assigning it to an external device.
- GPIO0/1/2/3/5/7/10/14/17/18/21/38/39/40/41/42/45/46/47/48 are used by RGB LCD interface. Treat them as unavailable.
- GPIO11/12/13 are routed to TF-card SPI (MOSI/SCK/MISO); can potentially be repurposed only if TF storage is not used, with the SD-card circuit and CS behavior checked.
- GPIO15/16 are shared with onboard RS485/CAN hardware; do not assume clean GPIO/UART without checking mux and board circuitry.
- GPIO19/20 are USB data; reserve them.
- GPIO43/44 connect to onboard USB-UART interface; do not assign a radio UART there without analyzing the CH343 connection and conflict when USB serial is used.
- GPIO6 is the Sensor AD connector signal and is a candidate for one digital control line only after confirming its board circuitry.
- GPIO8/9 I2C bus has onboard pull-ups and the CH422G expander; external expander/ADC are bus additions, not a new bus.

Official board resource page and schematic: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3/Resources-And-Documents

## Proposed architecture

### A. External button I/O expander
Candidate: MCP23017 (16 GPIO over I2C) or equivalent.
- Use a 4×4 button matrix to read the 16 front button positions with 8 expander pins; add one diode per button to prevent ghosting when simultaneous presses are possible.
- Allocate remaining expander pins to the two joystick push-switches and safety/arm/status signals, with pull-ups and debounce.
- Matrix row/column wiring uses two keyed harnesses and is documented in the final pinout.
- The physical POWER key is separate and handled by the hardware power-latch circuit, with a logic sense input if required.
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
| Button matrix/inputs | 16-position 4×4 matrix (8 lines) + joystick switches + safety/arm | MCP23017 GPIO expander |
| Joystick analog | L-X, L-Y, R-X, R-Y | ADS1015 4-channel ADC or exposed ADC pins |
| Joystick switches | L-SW, R-SW | Expander inputs |
| nRF radio | SCK, MOSI, MISO, CE, CSN, optional IRQ | SPI + GPIO |
| E22 radio | UART TX, RX, optional AUX | UART |
| Battery monitor | VBAT sense | ADC input via divider |
| Power control | POWER_KEY / ENABLE | Hardware power-latch circuit plus logic input |
| Status | buzzer/LED or fault | GPIO / expander |

## Pin assignment strategy — no frozen numbers yet

The schematic audit shows that the previously suggested “free GPIO” assignment cannot be made casually: most pins are dedicated, and several apparently available pins are connected to onboard interface circuitry. Keep all external devices on I2C where possible; use a radio interface daughterboard or verified breakout pins for the SPI/UART signals. The final pin map must document each signal, connector pin, onboard loading, boot/strapping constraints, and whether it remains available during USB debugging.

## Addressing and shared bus

- The board's I2C bus is GPIO8 SDA / GPIO9 SCL, shared by touch, CH422G, and the external I2C connector.
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
