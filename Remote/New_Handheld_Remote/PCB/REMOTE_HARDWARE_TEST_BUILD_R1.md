# Remote Hardware Test-Build Package R1

This package is intended to take the remote hardware from documented architecture to a safe, bench-testable prototype. **It is not yet a fabrication-ready PCB**: exact board revision/pin crosswalk, module footprints, electrical ratings and KiCad/ERC/layout are outstanding.

## Approved architecture
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948 remains the display/controller.
- 1S4P matched 18650 pack. Cell selection, pack construction, protection/BMS and connector remain to be approved.
- Primary: nRF24L01+ PA/LNA module, 2.4GHz.
- Backup: EBYTE E22-900T22S (SX1262 UART module) candidate, EU868.
- LEFT antenna = 2.4GHz; RIGHT antenna = 868MHz.
- ESP32 Wi-Fi/BLE uses onboard antenna.
- 16 buttons through two MCP23017; 4 joystick axes through ADS1115; monitoring through second ADS1115.
- I2C GPIO8/9; shared SPI GPIO11/12/13 with TF-card; UART/CSN/CE/IRQ physical pins are not frozen until connector mapping is validated.

## Supply/protection architecture
### Battery input
J_PACK keyed connector → F_MAIN → reverse-protection stage → BAT_PROT. Fuse must be close to pack-positive. Fuse and MOSFET must be selected based on measured load, pack short-circuit/BMS capability, DC interrupt ratings, thermal and I²t coordination. Do not rely on the pack BMS as the only fuse.

### Controller/display rail
BAT_PROT → Waveshare board PH2.0 single-cell battery input (polarity confirmed from board marking/schematic). Its onboard boost/charge path powers controller/display. USB charging behavior and backfeed with battery attached must be tested. External boost is not in parallel.

### nRF rail
BAT_PROT → TPS63070 buck-boost, 3.3V setpoint → nRF module. Datasheet-based design requires its reference application topology and component/layout values, not merely nominal divider values. Radio current bursts are validated by scope.

### LoRa rail
Candidate module supply 2.7–5.5V; 5V recommended to guarantee output power in the manual. Provisional filtered 5V from Waveshare 5V boost, subject to load test; otherwise separate converter. Never parallel boost outputs. Module UART is 3.3V logic; do not connect 5V UART.

## Test-stage wiring (no actuator connection)
### Controller
- Battery connector: protected 1S pack to board battery input after polarity test.
- I2C: SDA GPIO8, SCL GPIO9; devices 0x20,0x21,0x48,0x49, onboard touch/expander/RTC as discovered. Address scan and pullup measurement first.
- SPI radio: MOSI GPIO11, SCK GPIO12, MISO GPIO13; TF-card CS via CH422G EXIO4. Radio CSN, CE and IRQ must be direct GPIO assignments confirmed from PORT1 crosswalk. Shared bus lock; initial SPI 1MHz.
- Joystick ADC: ADS1115 @0x48, four single-ended channels; joystick pots from measured safe 3.3V supply, common ground, 1k series + 1µF RC only if source impedance/filter review supports.
- Buttons: MCP23017 @0x20; all 16 normally-open buttons to GND; use internal pullups; external reset bias and local 100nF.
- Battery/temperature monitor: ADS1115 @0x49. Battery divider 220k/68k and 100nF (max 4.2V maps under 1V); NTC 10k B3950 candidate; ADC channel input limits and sensor-open/short handling verify.
- LoRa UART: direct ESP UART TX/RX, AUX, M0/M1, NRST. Exact GPIO and board pin mapping TBD; do not connect until serial coexistence audit. UART start 9600 or 19200 after module config review.
- nRF: PA/LNA module local 10–47µF plus 100nF. Use a clean 3.3V rail; keep RF module supply traces short and low impedance.
- LoRa module: local bulk 100µF + 10µF + 100nF candidates, all close to VCC; scope during transmit.
- Antennas: separate bulkhead coax ports and matching antennas, labelled LEFT 2.4GHz and RIGHT 868MHz.

## Bench bring-up order
1. Visual inspection, board revision, connector orientation and continuity/polarity. No pack attached.
2. Current-limited supply to board battery input at 3.7V; confirm rails, charging/backfeed behavior; check display boot.
3. I2C scan; identify all onboard devices, pullups and bus voltage. Then add one peripheral at a time.
4. ADS1115 readings with known voltages; calibrate and test joystick endpoints.
5. MCP23017 input test using button pullups and debounce.
6. SPI test with TF-card idle; nRF module installed but RF output disabled; validate SPI read registers and pin levels.
7. nRF pair test in shielded/low-power bench arrangement; packet sequence/ACK/retry and burst droop.
8. LoRa module configuration over UART, readback of band/channel/rate/power; pair and exchange low-rate packets; verify AUX nonblocking.
9. Dual-radio failover test with RF attenuation/controlled disconnect; no motors/solenoids.
10. Simultaneous display, Wi-Fi, nRF and LoRa TX load test: rail droop, reset, current, temperature; record scope waveforms.
11. Antenna/cable/connector continuity and frequency matching; then controlled outdoor range/latency test.
12. Only after remote/boat protocol, watchdog and safety review may powered boat tests begin.

## Release gates
- Exact board schematic crosswalk reconciled with PCB revision.
- All connector pinouts checked with meter; keyed connectors selected and labels.
- Native KiCad schematic/PCB, ERC, DRC and footprint audit completed.
- Converter component calculations checked against current datasheets; thermal/current measurements.
- Battery/BMS/fuse/connector and cell assembly reviewed for short-circuit/thermal safety.
- Radio modules configured legally for EU/Bulgaria and antenna EIRP/duty cycle.
- Firmware compiles with pinned library versions, test vectors pass, authenticated protocol implemented and failsafe tested.
- Enclosure fit, RF detuning, antenna separation, hand/joystick/button reach and splash protection tested.

## Not yet final
Do not buy PCB assembly or create production harness from this R1 document. It's a safe test-build plan with unresolved GPIO and footprint items, not a fabrication package.
