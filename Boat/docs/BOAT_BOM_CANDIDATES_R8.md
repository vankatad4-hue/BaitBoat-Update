# R8 active Boat BOM — Heltec integrated SX1262 / direct MDD20A
Date: 2026-11-21 | Candidate engineering BOM **not** purchasing or PCB authorization.

| Ref | Qty | Component | Selection and electrical gate |
|---|---:|---|---|
| U_CTRL | 1 | Heltec Wireless Tracker V1.2 ESP32-S3 + GNSS + onboard SX1262 | Exact revision, internal SX1262 pin map, antenna and legal EU868 SKU unverified |
| U_MOTOR | 1 | Cytron MDD20A dual PWM/DIR motor driver | Separate mechanical mount; high-current supply and driver terminal pinout must be measured |
| M_L, M_R | 2 | Existing brushed 550 DC12V motors | Measure stall/current/transient and actual direction |
| U_NRF | 1 | nRF24L01+ PA/LNA module | External primary SPI 2.4GHz, independently regulated RF supply |
| ANT_NRF | 1 | nRF24 compatible antenna | Check RF load, cable and band |
| ANT_LORA | 1 | Heltec native SX1262-compatible 868MHz antenna | Uses *Tracker's dedicated* LoRa connector; no separate E22 module |
| U_IMU | 1 | MPU9250/AK8963 + BMP280 module | Selected candidate, I2C pullups/address/range unverified |
| U_ADC | 1 | ADS1115 | Candidate measured battery/current sensing; actual input protection and calibration pending |
| U_5V | 1 | Protected 3S-to-5V regulator | Choose actual input range, output peak current, thermal margin and fuse |
| U_RF_3V3 | 0..1 | Dedicated 3.3V nRF radio rail | If Tracker 3V3 rail cannot handle nRF PA/LNA TX bursts |
| U_WD | 1 | TPS3431 watchdog | Candidate 150pF CWD; WDO must set independent fault latch |
| U_LATCH | TBD | Discrete safety fault latch + physical gate controller | Must default OFF, STOP/WDO/UVLO/current/temp trip OFF and only manually rearm |
| Q_GATE | TBD | Actuator power cutoff | Load/short/flyback rating after motor + coil measurement |
| U_IO | 0..1 | Non-programmable I2C GPIO expander | For four coil selection and slow measured feedback ONLY if V1.2 pin audit requires |
| SOL_LH, SOL_RH | 2 | FW0630 009 DC12V | Left/right hopper independent drivers; characterize |
| SOL_LK, SOL_RK | 2 | FW0630 001 DC12V | Left/right hook independent drivers; characterize |
| Q_SOL_1..4 | 4 | Logic-level low-side MOSFET + gate resistors/pulldowns | Device selection after coil and switching data; OFF when permit false |
| D_SOL_1..4 | 4 | Flyback/clamp | Independently sized; confirm release delay, transient and pulse energy |
| F_SOL_1..4 | 4 | Coordinated branch protection | Rates remain TBD |
| J_SOL_1..4 | 4 | Keyed independent two-wire solenoid connectors | Current/heat/sealing TBD |
| U_LIMIT | TBD | Independent one-hot and maximum pulse time logic | Separate from MCU firmware/I2C bus; physical hardware enforce |
| F_MAIN/F_MOTOR/F_LOGIC | 3+ | Coordinated fusing/protection and switch | Battery/BMS, wire gauge and load curves unverified |
| U_SOL12 | 0..1 | Dedicated 12V coil regulator | Only if coil tolerances are not met by 3S 9–12.6V pack |
| J_SONAR | 0 | No committed sonar circuit | Sonar not selected; **no hardware, protocol or supply assignment** |

**Removed from active Boat BOM:** E22/E227 module, Boat SC16IS750, external Boat LoRa UART wiring, E22 M0/M1/AUX level hardware, extra 5V E22 supply and separate E22 RF connector. Do not remove the Heltec's own native SX1262 antenna.

**Safety:** Keep 3S pack off the Tracker's onboard 1S battery charge connector. Carrier remains a separate low-voltage modular PCB; MDD20A mounts separately. Current, thermal, electrical STOP and watchdog cutoff acceptance are blocked until real measurements and tests. Remote E22/SC16IS750 are unchanged and not PHY-validated against onboard SX1262.

References: BOAT_INTEGRATED_SX1262_R8.md, BOAT_HARDWARE_PIN_CONNECTIONS_R8.csv, BOAT_MDD20A_ELECTRICAL_GATE_R7.md. Official releases remain held.

### R9 schematic parts are **not** an approved purchasing BOM
The new `Boat/hardware/Coil_Drivers_R9/BOAT_COIL_DRIVER_BOM_R9.csv` maps native schematic references F1–F4, Q1–Q4, D1–D4, R1–R8 and J1–J6. Labels use generic custom symbols, all PCB footprints remain blank and device ratings TBD. Neither this schematic nor zero ERC on its **standalone partial sheet** authorizes purchasing power components or fabrication. Actual motor current, coil current, fuse, MOSFET VGS/SOA, clamp energy and release dynamics are still unmeasured. Power cutoff and one-hot pulse limiter are NOT physically implemented.

### R10 BOM candidate additions — NOT purchasing authorization
High-side gate draft `Boat/hardware/Safety_Cutoff_R10/BOAT_SAFETY_GATE_BOM_R10.csv`: U1–U3 SN74HC08, U4 SN74HC74, U5 SN74HC14; Q1 P-channel high-side device TBD (MDD20A+coil peak, thermal SOA not measured), Q2 low-side gate driver TBD, F1–F3 and J1–J6 generic unqualified fuses/connectors, S1+S2 portions of a REQUIRED independently conditioned triple-linked NC mechanical STOP assembly, R2 R3 R4 R5 provisional gate/bleed/sense components, and R1 WDO_N **DNP if R7 R1 WDO pullup is installed**. Actual 3S-to-3V3 safety logic supply is unimplemented. The partial R10 schematic still has **two undriven supply ERC errors intentionally BLOCKING**. No rated component/PCB/thermal decision without real measurements.
