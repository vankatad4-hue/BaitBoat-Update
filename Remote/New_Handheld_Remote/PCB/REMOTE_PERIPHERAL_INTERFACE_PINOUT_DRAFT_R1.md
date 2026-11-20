# Remote Peripheral Interface Pinout Draft R1

**Status:** Interface candidates, not frozen. Confirm board revision, GPIO exposure, connector pinout, I2C address scan, schematic and ERC.

## I2C expansion (Waveshare GPIO8 SDA / GPIO9 SCL)
Shared with onboard touch/CH422G/RTC. External devices:
- ADS1115 four-channel ADC, address 0x48 (ADDR→GND), 3.3V, 100nF local bypass. AIN0=Left X; AIN1=Left Y; AIN2=Right X; AIN3=Right Y. Input ranges set for 0–3.3V axes; verify absolute input limits. Use 860SPS aggregate for initial scans; four channels provide about 215SPS each, subject to conversion/settling.
- MCP23017 16-bit I/O expander, address 0x20 (A0/A1/A2→GND), 3.3V. GPA0–7 = button bank A; GPB0–7 = bank B. Buttons connect input to GND, active low, internal pullups enabled. Poll at 50–100Hz and debounce in firmware. INTA/INTB optional and unconnected initially. RESET pulled high.

I2C pullups: board already has pullups; do not add module pullups blindly. Confirm total resistance, bus capacitance and addresses with actual board.

## Joystick connectors
Two 5-pin connectors (exact part TBD):
- 3V3
- GND
- X axis signal
- Y axis signal
- NC / optional unused stick switch
Do not connect optional joystick switch unless button count/mapping is revised. Each axis is a 10k potentiometer candidate between 3V3 and GND, wiper to ADS1115. Add 1k series and 1µF shunt-to-ground filter candidate per channel; confirm settling and sampling. Software calibration/deadband/filtering is required.

## Buttons
16 separate button contacts, normally open to GND, MCP23017 input pullups; assign 8 to each bank. One of the 16 is the confirm/OK button per remote design. Mapping is a firmware/UI decision; connector part and physical wiring harness TBD.

## Battery monitor
GPIO6/Sensor-AD candidate. Protected pack divider: 220k BAT_PROT→ADC, 68k ADC→GND, 100nF filter. Verify GPIO6 board circuit and ADC calibration/attenuation.

## nRF24 radio
Shared SPI bus candidate GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO (TF-card bus). Separate chip selects required. Candidate CSN GPIO15, CE GPIO43; both are reused-function pins and require board schematic/PORT1 exposure verification. Radio IRQ not connected initially; poll STATUS/FIFO. If board CAN/RS485 or UART debug is needed, reconsider GPIOs. SPI bus access must be serialized; prioritize radio transactions and chunk SD access.

## Connector draft
- J_RADIO 8-way: GND, RADIO_3V3, SCK, MOSI, MISO, CSN, CE, IRQ_N (IRQ_N exposed but unconnected on controller initially).
- J_JOY_L/R 5-way: 3V3, GND, X, Y, NC.
- J_BUTTONS: 16 normally-open switches to MCP23017 GPIOs; harness/connector architecture TBD.
- J_I2C_EXPANSION: 3V3, GND, SDA, SCL; ADS1115 and MCP23017 on shared bus.
- J_BAT_BOARD: BAT_PROT and GND to Waveshare battery connector, polarity verified before assembly.

## Firmware sampling plan
- Joysticks: scan 4 ADC channels; target 50Hz control frame, maintain latest calibrated axis values and timestamp. ADC conversion latency and inter-axis skew must be measured.
- Buttons: read 16-bit MCP23017 state at 50–100Hz; debounce 10–20ms starting values, tune; active-low normalized logical state.
- Radio: SPI polling/STATUS at a fast interval; packet control output at 20Hz initial baseline. SD map IO must not block radio longer than latency budget.
