> **R15 HISTORICAL ONLY:** Previous custom Boat carrier PCB plan canceled. Use ready-made Heltec/MDD20A/EKM014/RECOM/Pololu modules and harnesses. See [new Boat module plan](BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md). No board fabrication permitted.

# Flytec V080 Boat — Carrier PCB Connection Plan (ver004)

**Status:** approved modular architecture; logical connector plan, not a released PCB pinout.

## 1. Architecture

Use one central low-voltage carrier PCB with removable, keyed/locking cable harnesses. The carrier is the interconnect and signal-distribution hub; it is not the motor power board. Major modules remain physically separate and replaceable:

- Main controller/GNSS: Heltec Wireless Tracker V1.2 (ESP32-S3FN8 + UC6580).
- Heading/motion: GY-91-style MPU-9250 + BMP280 breakout.
- Command radio: external nRF24L01+ PA/LNA module.
- Motor driver: **Cytron MDD20A dual-channel brushed-DC driver**, separate board, one channel per 550 motor.
- Solenoid driver: separate protected MOSFET driver board for bait/hook coils.
- Power module: battery input protection, branch fusing, logic regulators and monitoring.
- Sonar: separate CHIRP module; exact model/interface remains unselected.

The carrier PCB envelope remains approximately 77 × 79 × 14 mm, with approximately 30 mm compartment height. The MDD20A is not to be mounted on this carrier PCB; mount it separately with cooling, cable strain relief and access.

## 2. Power domains and routing

| Domain | Intended contents | Rule |
|---|---|---|
| BAT_RAW | Battery/BMS output, motor driver supply, solenoid power branch | High current stays off the logic carrier except for protected sensing. Fuse branches near source. |
| MOTOR_PWR | MDD20A supply and motor outputs | Direct appropriately sized harness from protected battery branch; never route motor current through carrier PCB. |
| SOLENOID_PWR | Solenoid driver supply and coil wiring | Separate fused branch; flyback/clamp protection at driver/coil as designed. |
| LOGIC_5V | Controller/peripheral supply, if required by verified board input | Regulated and fused; confirm Heltec V1.2 permitted input before connecting. |
| LOGIC_3V3 | GY-91 and logic signals | 3.3 V logic; do not feed 5 V into ESP32 GPIO. |
| RF_3V3 | nRF24L01+ PA/LNA | Dedicated low-noise 3.3 V regulator with local bulk + high-frequency decoupling; not assumed to be supplied by dev-board regulator. |

All grounds require a deliberate star/low-impedance return strategy. Keep motor and solenoid return currents out of sensor/radio ground paths. Join power and logic grounds at the designed reference point required by the driver interfaces; do not create uncontrolled parallel return paths.

## 3. Proposed carrier connector schedule

Connector family is a preliminary recommendation only. Select exact series, pitch, current rating, wire gauge, retention and mating parts after mechanical layout and current measurements. Use keyed/locking parts; do not use identical housings for different voltage domains where misconnection is possible.

| Ref | Module / function | Logical pin order (pin 1 onward) | Preliminary connector |
|---|---|---|---|
| J1 | Protected logic power input | 1: LOGIC_5V (or verified controller-input rail); 2: GND_LOGIC | JST-GH/XH, 2-pin, final current rating TBD |
| J2 | GY-91 I²C sensor | 1: 3V3_SENSOR; 2: GND; 3: SDA; 4: SCL | JST-GH, 4-pin |
| J3 | nRF24L01+ PA/LNA | 1: RF_3V3; 2: GND_RF; 3: SCK; 4: MOSI; 5: MISO; 6: CSN; 7: CE; 8: IRQ (optional) | Locking 8-pin; short cable; RF board mounted away from noise |
| J4 | MDD20A control harness | 1: GND_SIGNAL; 2: PWM1; 3: DIR1; 4: PWM2; 5: DIR2 | Locking 5-pin signal connector |
| J5 | Solenoid-driver control harness | 1: GND_SIGNAL; 2: BAIT_CMD; 3: HOOK_CMD; 4: AUX_CMD (reserved) | Locking 4-pin signal connector |
| J6 | Solenoid-driver status / interlock return | 1: GND_SIGNAL; 2: DRIVER_FAULT (optional); 3: SUPPLY_OK (optional); 4: SPARE | Locking 4-pin; populate only if driver supports these signals |
| J7 | Battery-voltage monitor input | 1: BAT_SENSE (conditioned/divided and protected); 2: GND_SENSE | Distinct keyed 2-pin; voltage divider/protection on carrier or power module |
| J8 | Hardware stop/interlock loop | 1: STOP_LOOP_IN; 2: STOP_LOOP_RETURN / status (architecture TBD) | Distinct locking 2-pin; must be designed as a real hardware inhibit, not merely a UI command |
| J9 | Sonar expansion (future) | Reserved; pinout, supply and interface intentionally undefined until a specific CHIRP module is selected | Do not populate or connect yet |
| J10 | Expansion / service | Reserved low-voltage GPIO/UART/I²C only after pin audit | Keyed, labelled, no power pins until assigned |

**Important:** J4 carries only logic-level MDD20A inputs. MDD20A battery input and motor outputs use their own appropriately rated power terminals and cables, never J4 or the carrier PCB. Confirm the exact MDD20A terminal names and control logic against the purchased board/datasheet before wiring.

## 4. Module-side wiring notes

### Heltec Wireless Tracker V1.2
- Mount on a removable carrier/socket arrangement with mechanical support; do not rely on header pins alone under vibration.
- The exact V1.2 schematic, pin map, GNSS UART and GNSS power control must be checked before allocating GPIOs.
- The onboard SX1262 LoRa is not used for the selected command link.
- GNSS antenna must have a clear sky view and be separated from motor/solenoid wiring.

### GY-91
- Initial interface: I²C (SDA/SCL), 3.3 V logic.
- Verify the actual breakout's regulator, level shifting, supply limits, chip markings and I²C address before connection.
- Keep sensor and cable away from magnetic/high-current sources; calibrate with the boat fully assembled.

### nRF24L01+ PA/LNA
- SPI + CE + CSN; IRQ optional.
- Dedicated RF_3V3 supply and local decoupling; confirm actual module peak current and RF front end.
- Short supply/ground wiring; antenna outside metal enclosure and away from switching regulators, display/data cables and motor leads.
- Check Bulgarian/EU EIRP limits for the exact module and antenna.

### Cytron MDD20A
- One channel per motor; motor driver is a separate replaceable board.
- Control harness uses PWM1/DIR1 and PWM2/DIR2 plus signal ground. No assumed enable/fault pin is added unless the exact board provides it.
- Battery and motor wiring are independent of the carrier PCB. Fit source-side branch protection and appropriately rated wiring/terminals.
- Driver rating does not increase the battery/BMS capability. Existing battery is reported as 3S4P with a 20 A BMS; verify actual pack/BMS and measure each motor's running, startup and stall current before selecting fuse, wire, connectors or operating limits.
- MDD20A has no assumed reverse-polarity protection; provide correct upstream protection and polarity-proof connectors.

### Solenoid driver
- Separate board with one independently controlled output per fitted coil/channel. BAIT_CMD and HOOK_CMD are logic-level commands only.
- Determine actual number of hook coils before final channel count. AUX_CMD is a reserve, not an instruction to fit another coil.
- Measure each coil resistance/current and permitted pulse duration. Select MOSFET, gate drive, flyback/clamp, fuse, connector and thermal design from measurements and coil data.
- Hardware interlocks must prevent unintended simultaneous/continuous energization; firmware timing alone is not the only protection.

## 5. GPIO allocation policy

No ESP32 GPIO numbers are approved in ver004. Build a pin-allocation spreadsheet only after:
1. confirming the exact Heltec Wireless Tracker V1.2 revision and matching schematic;
2. identifying onboard GNSS, display/debug, USB, LoRa and boot/strapping pin conflicts;
3. checking all selected interfaces and boot-safe states;
4. verifying the GY-91 breakout and nRF24 module wiring.

The connector pin order above is a **logical harness pinout**, not a statement about ESP32 GPIO numbers.

## 6. Harness and PCB rules

- Label both ends of every harness with connector reference, pin 1 and cable ID.
- Use keyed/locking connectors and distinct families for different power domains.
- Add test pads for GND, LOGIC_5V, LOGIC_3V3, RF_3V3, reset and selected bus signals.
- Keep radio, GNSS and IMU physically separated from motor/solenoid drivers and high-current loops.
- Provide strain relief, vibration-resistant mounting and drip protection; no connector should carry cable weight.
- Motor and solenoid outputs must never be hot-plugged. Power down and isolate battery before module replacement.
- Reserve board edge/area for service access and expansion; do not populate undefined connectors.

## 7. Measurements / gates before PCB release

- Exact Heltec V1.2 schematic and connector/pin mapping.
- Exact GY-91 breakout revision and voltage/I²C details.
- Exact nRF24 PA/LNA module, supply peak current, antenna and regulatory EIRP.
- Motor running/start/stall current for both motors; battery/BMS limits; fuse and wire calculations.
- Each solenoid coil resistance/current, number of coils, duty/pulse limits and thermal data.
- Exact MDD20A version, terminal layout, control truth table, mounting and cooling.
- Battery voltage measurement range and divider/protection values.
- Stop/interlock architecture and power-off behavior.
- Sonar module and interface (still unselected).
- Actual carrier PCB mounting holes, connector clearances and compartment vibration/water ingress.

## 8. Implementation status

This document is a hardware connection plan only. No new GPIO assignments, PCB layout, harnesses, drivers, radio implementation, GNSS/IMU drivers, sonar interface or actuator firmware are claimed. Boat firmware remains a non-actuating scaffold. Do not connect powered actuators or operate the boat using this software release.
