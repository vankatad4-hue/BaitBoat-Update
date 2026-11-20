# Waveshare ESP32-S3-Touch-LCD-4.3 Standard — GPIO Resource Audit R1

**Status:** Preliminary peripheral interface plan based on manufacturer pinout. GPIO assignments to custom wiring are provisional until the exact board revision, PORT1 header pinout and electrical connections are checked against schematic and a test board.

## 1. Board GPIOs already occupied
- RGB display: GPIO0,1,2,3,5,7,10,14,17,18,21,38,39,40,41,42,45,46,47,48.
- Touch: GPIO4 IRQ, GPIO8 SDA, GPIO9 SCL; CH422G EXIO1 touch reset.
- I2C peripheral bus: GPIO8 SDA/GPIO9 SCL (touch, CH422G, onboard RTC and external I2C connector).
- TF card: GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO; SD_CS on CH422G EXIO4.
- RS485/CAN share GPIO15/16 functions and USB/CAN multiplexing; GPIO15/16 are not clean generic GPIO choices without checking attached circuitry.
- USB native pins GPIO19/20; CH343 UART pins GPIO43/44.
- Sensor AD interface uses GPIO6 as analog input.
The pinout table and board schematic must be checked for connector exposure and attached loads before final assignment.

## 2. Proposed expansion to avoid consuming scarce GPIOs
### Four joystick analog axes
Candidate ADC: ADS1115, 16-bit, four single-ended inputs, I2C, maximum 860 samples/s aggregate.
- Supply 3.3V and GND.
- Address 0x48 (ADDR to GND).
- SDA/SCL share the board's GPIO8/GPIO9 bus.
- AIN0–AIN3: joystick X/Y channels.
- Use PGA range suitable for 0–3.3V signals; inputs must not exceed ADC supply rails.
- Initial sample target: 50Hz control updates with an acquisition scan substantially faster; 860SPS aggregate is adequate for four channels if latency is measured.
- Candidate per-channel input filter: 1k series plus 1µF to ground (about 160Hz RC); confirm source impedance/settling and adjust. Software calibration, deadband, smoothing and fail-detection remain in firmware.
- ALERT/RDY may be left unused initially; use polling.
- 100nF local supply bypass.

### 16 physical buttons
Candidate expander: MCP23017 16-bit I2C GPIO.
- Supply 3.3V; address 0x20 with A0/A1/A2 low.
- All 16 buttons are normally-open contacts to GND, active-low inputs.
- Internal pullups enabled as starting point; add external pullups only if noise/board testing requires.
- Button numbering map: Bank A = 8 buttons, Bank B = 8 buttons; one button reserved as OK/CONFIRM per existing remote concept.
- Software debounce, stuck-button detection and safe-state behavior.
- INTA/INTB optional; initial design can poll over I2C at 50–100Hz to avoid consuming a scarce ESP32 GPIO.
- RESET pulled high to 3.3V; 100nF local bypass.

## 3. Battery measurement
Use GPIO6 sensor-AD input as BATTERY_ADC candidate:
- Protected pack→220kΩ→ADC node; ADC node→68kΩ→GND; 100nF at ADC node.
- 4.2V pack gives ≈0.98V ADC; 3.0V gives ≈0.70V.
- Verify board's GPIO6/Sensor-AD circuit, ADC attenuation, calibration and connector path before use. If it is already loaded/conditioned, revise.

## 4. nRF24L01+ PA/LNA SPI
- SPI SCK/MOSI/MISO candidate = GPIO12/GPIO11/GPIO13 (shared with TF card SPI).
- SD-card chip-select is via CH422G EXIO4; radio uses separate CSN and CE.
- Candidate CSN = GPIO15 and CE = GPIO43, subject to confirming PORT1 exposure, board circuitry and no conflicts. GPIO15 is also associated with RS485/CAN transmit; GPIO43 is UART0 TX to CH343. If those interfaces/debug are needed, revise GPIO assignment.
- Radio IRQ is omitted initially; poll STATUS/FIFO at a short interval. This saves an interrupt pin and avoids GPIO4 touch IRQ conflict.
- SPI bus must be transaction-locked and radio-priority. SD card accesses are chunked so they cannot block control packets for long; bus clock changes between SD and nRF devices as required.
- Radio module power remains on RADIO_3V3 from TPS63070; nRF sleep modes used for power saving.

## 5. I2C shared bus
GPIO8/GPIO9 is already used by touch and board expansion. Add ADS1115 and MCP23017 only after bus scan and pullup review.
- ADS1115 address candidate 0x48.
- MCP23017 address candidate 0x20.
- Confirm no address conflict with GT911, CH422G or RTC; inspect actual board and run scan.
- Do not blindly add module pullups in parallel with onboard pullups; total resistance and bus capacitance must be measured.

## 6. Bench/prototype gate
- [ ] Exact Standard board revision confirmed.
- [ ] PORT1 header mapping and available pins verified.
- [ ] GPIO15/43 attached circuits and reuse suitability checked.
- [ ] I2C bus scan, address confirmation and pullup resistance.
- [ ] ADS1115 axis sample latency/rate, noise and full joystick range.
- [ ] MCP23017 button states, debounce, interrupt/poll behavior.
- [ ] Shared SPI with SD and nRF tested under simultaneous map reads/radio activity.
- [ ] Native schematic and ERC before PCB.
