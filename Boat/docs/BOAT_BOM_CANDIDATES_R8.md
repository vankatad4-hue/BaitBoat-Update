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


## R11 physical-timing candidate and verified KiCad continuation — 2026-11-21

- Created an actual editable KiCad schematic at `Boat/hardware/Pulse_Limiter_R11/Boat_Pulse_Limiter_R11.kicad_sch` with 17 components: CD74HC221 monostable, SN74HC175 channel latch, six SN74HC08 quad AND ICs, two SN74HC32 quad OR ICs, one SN74HC14 Schmitt inverter IC, three R/C passives, and three harness connectors. All footprints and timer R/C ratings remain unassigned.
- Native KiCad **PASS** run https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201259 on `cb8ed8546bff1c5555ddecc667c052365cbe61d1`: 54 named exported XML nets checked against exact pin numbers, 9 explicit no-connect pins, 1,024 static one-hot/window combinations and cross-sheet logical-name checks passed. The isolated R11 sheet has **exactly two outstanding power_pin_not_driven errors**: the external unqualified 3V3 safety rail and return. No additional ERC errors; neither 2 ERC errors nor isolated ERC proves system safety.
- Existing Boat host CI PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201349; ZIP packaging workflow PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201366. Previous R11 KiCad run failed a test assertion pin number (pin 13 vs real pin 12), corrected before passing.
- The new interface contract `Boat/docs/BOAT_R7_R9_R10_R11_INTERFACE_CONTRACT.csv` maps R7 raw release, R9 MOSFET gates and R10 safety, but **all physical inter-sheet connections remain UNBUILT**. R10 internal LATCH_Q is not wired out to an approved connector.
- **Blocking unsafe failure mode:** The external `RELEASE_EDGE_TOKEN_TBD` is **not implemented**. If this input remains HIGH, a held release request can trigger a second pulse after the 74HC221 window ends. Analog motor-/coil-current-zero comparators, independent 3V3/POR and rail-good logic, R/C pulse bounds, input filtering/setup/hold and actual actuator STOP/current-off testing are also missing. **Never connect/energize motors or solenoids from this schematic.**
- Remaining standalone ERC blockers: R7 **15**, R9 **0** (isolated low-side stages), R10 **2** (external power), R11 **2** (external power). No combined ERC, PCB DRC, validated high-current path, physical acceptance or release.
- Next: capture the *consumed* hardware release/zero-current rearm token (no autorepeat), independently qualify current-zero and logic supply, analyze race/glitch/fault timing, establish manufacturer pulse width/fuse/MOSFET ratings from measured hardware, integrate four sheets into native KiCad hierarchy and run system ERC. Sonar remains **planned only**; no sonar module selected. Single Heltec Boat controller with MDD20A differential motors/no rudder, no RP2040 and no Boat E22. Official Remote VER028 / Boat VER014 unchanged; 029/015 withheld.

## R12 physical consumed-release token — verified isolated KiCad candidate (2026-11-21)

- New native **editable** schematic `Boat/hardware/Release_Token_R12/Boat_Release_Token_R12.kicad_sch` (23 generic components) and project-local symbol library. Two flip-flops in SN74HC74 implement **RELEASE_ARMED** (set only after a separately RC/Schmitt-qualified all-requests-released and current-zero condition) and **FIRED_Q** (held during a pressed request, consuming RELEASE_ARMED asynchronously). CD74HC221 supplies a *separate short token* to existing R11 `RELEASE_EDGE_TOKEN_TBD`; six pairwise overlap terms reject multi-request patterns. R1-C4 timing/filters unspecified, all footprints blank. R12 output connector J3 is **NOT physically wired to R11 J2** yet.
- Actual KiCad CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745446 on `ee9ff4fbb122ce59ce2628007c3ad2f05d24ace8`: **53 precisely matched electrical nets, 11 explicit no-connect output pins, 256 static mask/ready tests and 7 synthetic temporal-model tests**. Only **2 unresolved isolated-sheet `power_pin_not_driven` ERC violations** remain for the unapproved 3V3/ground source; this is NOT zero ERC on full Boat. Boat portable host CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745373; ZIP packaging CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745305.
- Detailed interface mapping: `Boat/docs/BOAT_R7_TO_R12_INTERFACE_CONTRACT.csv`, with every unbuilt harness/status explicit. R7 SOL_REQ_x to R12 J1; R12 J3 pin1 to R11 J2 pin6; R11 J3 4 gates to R9 J2; R10 internal LATCH_Q is NOT exposed on an approved connector. R10 feeds R9 coil rail only by a proposed independent fused power connector.
- Critical remaining hazards listed in `Boat/docs/BOAT_R12_HAZARD_REVIEW.csv`: external POR may assert before 3 separate RC filter nodes settle, request bounce/race, non-zero coil current or failed current sensor, R12 token width longer than R11 main time, stuck-high token that can retrigger R11, U2 or high-side MOSFET failure, motor and coil real flyback, unverified 3S high current. **No actuator power may be connected.**
- Component gate: qualify *actual* 3S-to-logic regulator/POR, R/C release/debounce/reset-to-CLK and token-vs-main timing over tolerance+temperature, independent motor/coil current-zero comparators, physical R10 STOP latch and output connector, and a power-rated high-side actuator disconnect for both motor and solenoid supply. Integrate actual combined KiCad hierarchy/ERC/PCB/DRC and perform dummy-load fault acceptance.
- Standalone ERC status: **R7 15, R9 0 (partial low-side only), R10 2, R11 2, R12 2**. No system-wide zero ERC or fab authorization. Existing Boat and Remote pin maps remain uncommissioned; no RP2040, no rudder, no Boat E22. Sonar has no selected model and remains **planning only**. Official versions Remote VER028 / Boat VER014 unchanged; VER029/VER015 withheld.

## R13 RECOM ready-made 3.3 V safety supply (2026-11-21)

**User choice: use a premade regulator module instead of a custom 3.3 V feedback/inductor buck design.** Selected candidate **RECOM R-78E3.3-0.5** SIP3, datasheet REV 9/2025 https://recom-power.com/pdf/Innoline/R-78E-0.5.pdf. Vendor pinout **pin1 VIN, pin2 GND, pin3 VOUT**; input **6–28 V**, fixed output **3.3 V**, max **0.5 A**, **11.6 × 8.5 × 10.4 mm**, non-isolated. 3S 12.6 V maximum nominal charge is in range assuming measured input spikes stay within 6–28 V.

Created native **R13 KiCad** power interface `Boat/hardware/Safety_Power_R13/Boat_Safety_Power_R13.kicad_sch` with 9 components: fused protected battery-to-module VIN, supply/common-return input, VLOGIC_3V3/GND outputs to R10-R12, test interface, optional 10uF/35V input and 10uF/10V + 100nF output decoupling, optional 300 ohm/0.125 W preload (~11mA; RECOM recommends >2% min load for specified ripple). No fuse value or footprint approved; optional parts may be omitted only after ripple and EMC tests. Vendor 220uF maximum output capacitive-load condition includes all downstream decoupling. `Boat/docs/BOAT_R13_READYMADE_REGULATOR_BOM.csv` lists choices. Native exported KiCad netlist / ERC CI is `boat-kicad-power-r13.yml`.

**Safety limits:** this preassembled non-isolated regulator is **only for independent safety logic**, not nRF24 PA/LNA burst supply, not Heltec ESP32 input (which needs separately approved 5V), and never motor/solenoid load. Source is MASTER-switched pack protected by BMS/fuse, electrically **before** the R10 actuator high-side cutoff. Its output ground shares a low-current star reference with the battery; prevent high-current motor/coil return in safety harness. The RECOM device has **no separate PG pin** and short-circuit protection **automatically restarts**. A separate physical POR/brownout supervisor must assert reset on loss and require manual physical fault-latch rearm after recovery. The regulator alone is **not** an independent safety interlock. Input 3S pack transient, fuse, mounting/socket retention, load/thermal, EMI/RF noise, real 3.3V ramp/current and POR/UVLO timing are **unverified**.

The new R13 power sheet does NOT automatically clear older separate-sheet ERC: R7=15; R9=0 (partial low-side); R10=2, R11=2, R12=2 (external supply); combined KiCad hierarchy, PCB, rated high-side cutoff and physical loads remain unfinished. No sonar implementation, no RP2040/rudder/Boat E22. Keep official Remote VER028/Boat VER014, no physical actuation permission.


## R14 ready-made four-coil module feasibility (2026-11-21)
User asked whether an NCD 4-channel solenoid I²C board or equivalent can replace our new custom PCB designs. **Yes, for the four power MOSFET channels**, with important constraints. NCD PR22-5/PR22-4 list 5.5A/channel, below the project's *provisional 6A-per-coil design allowance*, and the advertised 8W is transistor heat dissipation—not an 8W coil limit. Preferred **candidate** is Electrokit **EKM014** 4-channel MOSFET board: listed 10A/channel, load 5–50V, inputs accept ESP32 3.3V GPIO, built-in flyback, and 4.5–12V driver supply **VL from separate regulated 5V**, not RECOM 3.3V or raw 12.6V charged 3S. Supplier states a single 10A slow-blow fuse in shared VH. Actual coil current/thermal/timing and board mechanical dimensions (conflicting supplier listings) must be verified.

**No component purchase/design freeze yet**. New source: `Boat/docs/BOAT_R14_PREMADE_SOLENOID_DRIVER_EVALUATION.md` and proposed interface: `Boat/docs/BOAT_R14_EKM014_HARNESS_CANDIDATE.csv`. Keep the earlier R9 KiCad driver board as unused engineering fallback; do not fabricate it. The off-the-shelf FET module **does not replace independent hardware one-hot/nonretriggerable pulse limiter, power-cutoff latch, STOP, current-zero sensing and POR**. Must choose/qualify off-the-shelf modules for those, or retain a small safety interlock design. MDD20A continues driving two motors; no rudder/RP2040. Sonar unselected. Remote VER028 / Boat VER014 held.
