# Boat Electrical Schematic Capture Sheets R2

**Historical reference:** use [R5 board integration](BOAT_BOARD_INTEGRATION_R5.md) and [R5 BOM](BOAT_BOM_CANDIDATES_R5.md) for the active architecture. Earlier two-coil quantities and simultaneous-coil sizing assumptions are superseded by four independent channels, maximum one active, separate hardware cutoff, and direct Tracker PWM/DIR.


## Sheet 01 — pack, BMS, fuse and master switch
J_PACK keyed connector, reported3S4P battery/BMS20A (confirm). PACK+→BMS output→F_MAIN candidate20A (final after current/wire coordination)→retained switch→BUS_3S. PACK−→star high-current return. Retain battery indicator; verify range/polarity. Add test pads BUS_3S,PACK−,BMS output. Main fuse close to pack.

## Sheet 02 — electronics 5V converter
BUS_3S→electronics buck candidate 5V,≥2A peak; branch fuse candidate2A. Output5V→Tracker approved power input and E22 VCC. Converter component-level circuit/footprint chosen later. Add input/output caps per reference design, test pads5V/GND. E22 local100µF+10µF+100nF. Logic UART/M0/M1/AUX is3.3V; never5V TTL.

## Sheet 03 — nRF24 primary
nRF VCC from verified3V3 rail or dedicated buck, local100nF+10µF+22–47µF candidate. SPI MOSI/SCK/MISO, direct CE/CSN, IRQ. Pin map TBD from exact Tracker schematic. Radio channel76,250kbps,fixed32,CRC16,autoack/retries,PA_LOW initial. Test rail droop and coexistence with integrated radios/Wi-Fi.

## Sheet 04 — E22 backup
Matching E22 UART module VCC=5V_ELECTRONICS, UART TX→MCU RX, MCU TX→UART RX at3.3V, M0/M1 defined low normal, AUX input, reset access. 50Ω 868MHz antenna. Module config/readback same as remote. Local bypass and test pads. Boat pins TBD.

## Sheet 05 — motor driver and harness
Cytron MDD20A candidate separate mount. BUS_3S→driver VB+ via motor branch fuse selected after startup/stall current tests; BUS return heavy GND. M1A/M1B→left550, M2A/M2B→right550. Controller DIR1/PWM1/DIR2/PWM2/GND harness. PWM reset low; DIR defined. Driver has no reverse protection. Main fuse/BMS/wire/driver/current coordination required.

## Sheet 06 — bait/hook solenoid drivers
Bait FW0630 009 and hook FW0630 001 nominal12V. Each coil supply from SOL_12V; raw BUS_3S only if coil voltage tolerance confirmed (pack9–12.6V).
Each low-side MOSFET: Drain coil−,Source solenoid GND, Gate MCU through100Ω candidate, Gate-Source100k pulldown. Flyback diode cathode coil+,anode coil−. Individual branch fuse. MOSFET/diode ratings based on measured coil current and pulse. Outputs OFF by hardware default. Pulse-limit/lockout in software.
If coil range requires12V regulated, use buck-boost converter sized for simultaneous coil peaks; not selected yet.

## Sheet 07 — battery monitor and sensors
Battery divider into ADC after exact Tracker ADC range/pin map. Heading/compass I2C; GNSS integrated. Sonar connector/voltage/interface TBD after sonar module selection. Sensor grounds return to logic star, away from motor-current return.

## Sheet 08 — connectors/test points
J_PACK, J_MOTOR_PWR, J_MOTOR1/2, J_CTRL_MOTOR, J_NRF, J_E22, J_BAIT, J_HOOK, J_SENSORS, J_GNSS/SONAR, battery indicator and main switch. Keyed/locking, ratings and pin numbering to be frozen.
Test pads PACK/BUS_3S,5V,3V3,12V_SOL,logic GND,UART,SPI,PWM/DIR,ADC.

## Layout
Carrier77×79×14mm measured preliminary. 4-layer control carrier, 2oz power copper candidate, solid ground plane; motor driver separate. Keep RF/ADC away from motor switching. E22/nRF antenna feeds separate; GNSS antenna unobstructed. Waterproof and thermal fit validation.
