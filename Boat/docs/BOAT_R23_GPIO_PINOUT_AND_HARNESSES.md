# Boat R23 — ESP32-S3 GPIO pin freeze candidate and ready-made module harnesses

> **CURRENT ASSEMBLY OVERRIDE — R30A (2026-11-22):** This R23 document is the original manufacturer-header *candidate* reference only. Do **NOT** assemble ADS1115 battery nets or separately switched motor/solenoid supplies from its historical CSV. The current [R30A effective netlist](BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv) merges all still-relevant R23 signals with the R27 INA231 BUS-only wiring and R29 **single common 3S** VM/VH supply. It retains unsigned GPIO and connector checks; no actuator power is approved. See [R30A handoff](BOAT_R30A_PROTOTYPE_ASSEMBLY_HANDOFF.md).


**Date:** 2026-11-22 | **Branch:** `hardware-ready-dual-radio-r1`  
**Scope:** actual R20-selected DevKitC-1-N32R16V + M100 PRO + GY-91 + ADS1115 + nRF24 PA/LNA + E22-900T22S + Waveshare MCP23017 + Cytron MDD20A + EKM014.  
**Result:** **manufacturer-header-verified pin allocation and complete logical connection schedule**; final board-revision, carrier connector, logic-level and physical safety tests remain **unsigned**. **No physical motor or solenoid energization authorized.** R30H-124W supersedes the optional GPIO21 nRF IRQ assignment with the candidate sonar UART0 RX route; the radio remains polled.

The authoritative machine-readable 64-net schedule is [BOAT_R23_MODULE_CONNECTION_MATRIX.csv](BOAT_R23_MODULE_CONNECTION_MATRIX.csv). This document and CSV supersede R20's prose **only where extra electrical conditions are made explicit**, not the R20 module selection or the R21 battery divider.

## 1. ESP32-S3-DevKitC-1-N32R16V J1 and J3 allocation

Espressif's **DevKitC-1 v1.1 J1/J3 header table** matches all 18 proposed MCU GPIO positions below. Header positions refer to Espressif's silk/header numbering **not Arduino GPIO numbering**. Both original and v1.1 DevKitC-1 revisions are sold; the actual PCB must still be identified. The module must be **ESP32-S3-WROOM-2-N32R16V** (not an arbitrary WROOM-1 clone).

| ESP32 GPIO | Espressif header | Assigned function | Connected module / pin |
|---:|---|---|---|
| 4 | J1-4 | UART TX (GNSS) | HGLRC M100 PRO RX |
| 5 | J1-5 | UART RX (GNSS) | HGLRC M100 PRO TX |
| 6 | J1-6 | E22 hardware reset | E22-900T22S pad 1 NRST |
| 7 | J1-7 | Shared I²C SCL | M100/QMC, GY-91, ADS1115, MCP23017 |
| 8 | J1-12 | Shared I²C SDA | Same four physical modules |
| 9 | J1-15 | nRF24 CSN | nRF24 CSN |
| 10 | J1-16 | nRF24 CE | nRF24 CE |
| 11 | J1-17 | nRF24 SPI MOSI | nRF24 MOSI |
| 12 | J1-18 | nRF24 SPI CLK | nRF24 SCK |
| 13 | J1-19 | nRF24 SPI MISO | nRF24 MISO |
| 14 | J1-20 | E22 status input | E22 pad 24 AUX |
| 15 | J1-8 | E22 mode 0 | E22 pad 20 M0 |
| 16 | J1-9 | E22 mode 1 | E22 pad 21 M1 |
| 17 | J1-10 | UART TX (E22) | E22 pad 22 RXD |
| 18 | J1-11 | UART RX (E22) | E22 pad 23 TXD |
| 21 | J3-18 | Open Echo UART RX | Sonar coprocessor TX; UART0 RX with native USB CDC debug console |
| 39 | J3-9 | MDD20A left PWM | MDD20A PWM1 (JTAG MTCK repurposed) |
| 40 | J3-8 | MDD20A right PWM | MDD20A PWM2 (JTAG MTDO repurposed) |

All assigned ESP pins use 3.3V logic; **never drive ESP or MCP23017 GPIO with 5V**. GPIO39/40 are usable as PWM but sacrifice those JTAG signals; perform debugging through USB and verify no unwanted JTAG behavior during reset. **The CSV and static code checks match this exact table.**

### Reserved; do not connect

- **GPIO0/3/45/46:** strapping/reset; no external wiring.
- **GPIO35/36/37:** N32R16V octal flash/PSRAM interface.
- **GPIO38/48:** board-version-dependent RGB LED; reserved.
- **GPIO47/48:** critically, **1.8V operation on ESP32-S3-WROOM-2 'V' variants**; never connect them to 3.3V wiring.
- **GPIO19/20:** native USB. **GPIO43/44:** UART0 USB bridge.
- **GPIO1/2/41/42:** reserved, currently **NO WIRE**. Potential physical STOP/watchdog/contactor/current-zero connections remain unassigned until an actual safe electrical interface is selected. Four spare GPIO alone do not implement an independent safety supervisor.

Source: [Espressif DevKitC-1 v1.1 pin headers](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html) and [ESP32-S3-WROOM-2 datasheet](https://documentation.espressif.com/esp32_s3-wroom-2_datasheet_en.html).

## 2. Shared I²C: GPIO7 SCL and GPIO8 SDA

| Device | Planned 7-bit address | Supply | Isolation check |
|---|---|---|---|
| M100 PRO integrated QMC5883L | 0x0D | Module main input 5V | **HOLD** until manufacturer connector order and SCL/SDA idle/pull-up levels are verified 3.3V-safe |
| GY-91 MPU-9250 | 0x68 (or 0x69 if AD0 high) | 3.3V sensor rail | Verify genuine chip and its breakout power/AD0 routing |
| GY-91 BMP280 | 0x76 or 0x77 | Same 3.3V sensor rail | Read chip ID; no assumption of fixed strap |
| ADS1115 | 0x48 (ADDR=GND) | 3.3V sensor rail | Confirm it is ADS1115 rather than ADS1015; R21 divider; VDD/analog ramp safety |
| Waveshare MCP23017 IO Expansion Board | **0x27 when A0 A1 A2 are left open** on the Waveshare board | 3.3V sensor rail | Manufacturer wiki: jumper **open=HIGH / short=LOW**. Verify PCB jumper states, RESET high, and I²C scan |

All peripheral grounds share the *low-current* sensor reference at a defined ground distribution point. Do not send motor/coil return current through I²C harnesses. I²C pull-ups must terminate at **3.3V only**, never 5V. Keep wiring short and check combined pull-up resistance, edge timing and address conflicts with an actual logic analyzer.

**M100 PRO wiring:** its supplied six-pin SH1.0 harness comprises function labels VCC, GND, TX, RX, SDA, SCL, but this document **does not assign any physical contact order**; only the exact purchased board cable and silk/datasheet can confirm the cavity order. The M100 PRO manufacturer lists 3.6–5.5V input, UBX at 115200 baud and integrated QMC5883L. The 5V supply rating alone does **not** prove TX or I²C output HIGH is 3.3V-safe. First power M100 independently and measure TX/SDA/SCL levels. If any exceeds ESP32 limits, use a compatible **premade** level-shifting interface rather than attaching directly.

Manufacturer references: [HGLRC M100 PRO](https://www.hglrc.com/products/hglrc-m100-pro-gps); [Waveshare MCP23017 wiki](https://www.waveshare.com/wiki/MCP23017_IO_Expansion_Board); [Microchip MCP23017 datasheet](https://www.microchip.com/en-us/product/MCP23017); [TI ADS1115](https://www.ti.com/product/ADS1115).

## 3. Primary 2.4GHz radio: nRF24L01+ PA/LNA

Selected wiring uses the single 3.3V SPI bus on GPIO11/MOSI, GPIO12/SCK, GPIO13/MISO, GPIO9/CSN and GPIO10/CE. GPIO21 is reassigned to sonar UART0 RX; leave the optional nRF IRQ disconnected because `NRF24Link` polls the radio. Power is **Pololu D24V10F3 #2830 unit #1**, not the ESP32 board's onboard 3.3V pin and never a 5V socket feed. Provide proper local supply decoupling and measure peak burst/ripple.

**Critical connector condition:** nRF24 PA/LNA carriers typically expose the labelled signals GND/VCC/CE/CSN/SCK/MOSI/MISO/IRQ but **the 2×4 physical header orientation remains unknown**. Crimp/key the final adapter harness after inspecting the actual purchased module. An Ardboard nRF24 adapter may be used *only after* its regulator input/output/ripple are qualified; the currently selected power path is the separate Pololu 3.3V regulator, so do not purchase a second radio voltage converter automatically. Antenna matching and RF coverage must be tested.

## 4. Backup EU868 radio: EBYTE E22-900T22S UART **26-pad**, not E220 or S1C

| E22 manufacturer pad | Function | ESP32-S3 signal |
|---:|---|---|
| 1 | NRST (active low) | GPIO6 |
| 20 | M0 | GPIO15 |
| 21 | M1 | GPIO16 |
| 22 | RXD | GPIO17 TX |
| 23 | TXD | GPIO18 RX |
| 24 | AUX | GPIO14 input |
| 25 | VCC | Regulated **5V main DCDC6-16-TO5** |
| 12 | ANT | Appropriate 50-ohm 868MHz antenna line |
| GND pads 2/3/8/11/13/14/15/16/19/26 | Module ground | RF ground reference through manufactured carrier |

Manufacturer confirms TTL UART compatible with 3.3V and 5V controllers and 2.3–5.2V VCC on this suffix; **actual TXD/AUX high levels with its VCC at 5V still need bench measurement**. Verify factory SMD-to-header carrier; do not wire a different E22 suffix on pad numbers above. Firmware `BOAT_E22_UART_PIN_AUDIT_SIGNED=0`, `BOAT_E22_UART_LOGIC_LEVELS_VERIFIED=0` and `BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED=0` are deliberately unchanged. The transmitter must not operate without a matched antenna or approved EU868 power/channel settings.

Source: [EBYTE E22-900T22S manufacturer pin definition](https://www.cdebyte.com/products/E22-900T22S/2).

## 5. Motor driver wiring: Cytron MDD20A

| Path | Origin | MDD20A connection |
|---|---|---|
| PWM motor left | GPIO39 / J3-9 | PWM1 |
| PWM motor right | GPIO40 / J3-8 | PWM2 |
| Direction motor left | MCP23017 **GPB0** | DIR1 |
| Direction motor right | MCP23017 **GPB1** | DIR2 |
| Low-current reference | logic GND | control GND |
| Motor power | protected 3S battery via uncommissioned independent hard STOP/power cutoff and separately fused rated conductors | MDD20A motor supply **DISCONNECTED** |
| Outputs | MDD20A output M1 / M2 | original two brushed 550 motors **DISCONNECTED** |

Cytron specifies 6–30V motor supply, PWM/DIR input logic including 3.3V, max **20kHz PWM**, **no reverse-polarity protection**. No software GPIO/PWM initialization may bypass the present independent safety gate. The two MCP expander DIR outputs must be initialized LOW before becoming outputs; changing direction requires PWM zero and isolated motor power. A successful I²C output-latch readback is **not** proof of an independent motor cutoff. Motor polarity, left/right orientation, startup/stall currents and the 20A BMS envelope require physical dummy-load/electrical validation.

Source: [Cytron MDD20A](https://my.cytron.io/sumo-robot/c-motor-driver/p-20amp-6v-30v-dc-motor-driver-2-channels).

## 6. Solenoid driver wiring: Electrokit EKM014

| Command from MCP23017 at 3.3V | EKM014 input | Load (output +/− pair) |
|---|---|---|
| GPA0 | IN1 | Left bait hopper |
| GPA1 | IN2 | Right bait hopper |
| GPA2 | IN3 | Left hook release |
| GPA3 | IN4 | Right hook release |

EKM014 **VL** connects only to the chosen **TRACO TSR 2-2450 regulated 5V logic rail**; EKM014 **VH** connects only to a separately protected/fused 3S solenoid power branch **after physical interlock sign-off**. EKM014 GND shares the required reference with the 3.3V GPIO signals. The manufacturer explicitly supports **3.3V control HIGH with VL=5V** and requires **VH ≥ VL**. **Do not short VH and VL together:** a full 3S battery is 12.6V, exceeding the manufacturer's 12V logic supply recommendation.

Each load's **positive lead** connects to its dedicated EKM014 channel **+** (supply VH) and negative lead to that channel **−** (low-side MOSFET switching). The onboard nominal **10A slow-blow fuse is not a 6A instantaneous pulse limiter**. Only **one of the four 6A-class coils may be on at a time**. GPIO outputs, I²C initialization and software one-hot control do not replace a physically independent current-zero measurement, non-retriggerable pulse limiter or STOP/cutoff. **All coil connections remain DISCONNECTED** until those are resolved.

Source: [Electrokit EKM014 manufacturer documentation](https://docs.electrokit.com/en/modules/EKM014/).

## 7. Power-rail integration — no custom PCB

| Rail | Selected module | Connected loads |
|---|---|---|
| `5V_MAIN` | Olimex DCDC6-16-TO5, 5V / 2A | DevKitC-1 5V input, HGLRC M100 PRO, E22 VCC pad25 |
| `5V_EKM_LOGIC` | TRACO TSR 2-2450, 5V / 2A | EKM014 VL only |
| `3V3_NRF` | Pololu D24V10F3 #2830 **#1**, 3.3V | nRF24 PA/LNA |
| `3V3_SENSORS` | Pololu D24V10F3 #2830 **#2**, 3.3V | GY-91, ADS1115, Waveshare MCP23017 |
| `PACK_SENSE_SWITCHED` | protected/switched 3S output | R21 150kΩ → ADS1115 AIN0, 33kΩ and 100nF to sensor ground |
| `3S_MOTOR` | BMS+protected distribution, **safety cutoff unselected** | MDD20A VIN **NOT CONNECTED** |
| `3S_SOLENOID` | BMS+protected distribution, **safety cutoff unselected** | EKM014 VH **NOT CONNECTED** |

Never parallel regulator outputs. Keep power grounds common where required for logic but route high-current returns separately to the battery-negative distribution. Prevent external 5V from back-feeding USB or vice versa. Dimension fuses, wire gauge and connectors only after actual startup/stall/coil current and temperature measurements. The current 20A BMS nominal does **not** mean 20A per motor can safely be delivered.

## 8. Connection and commissioning sequence

1. **Before applying voltage:** identify exact ESP32-S3-WROOM-2-N32R16V shield and DevKitC-1 PCB revision; inspect J1/J3 positions and physical M100/E22/nRF/MCP/driver header orientation. Record actual labels and photographs. Do not infer a 6-pin connector's cavity numbering from these logical nets.
2. **Power only:** test the main 5V, logic 5V, sensor 3.3V and RF 3.3V rails separately using a current-limited bench supply. Do not connect motors or any solenoid load. Verify physical grounds and prevent USB backfeed.
3. **Isolated I²C modules:** scan individual GY-91, ADS1115 and MCP23017 at **3.3V**; verify MCP A0–A2 jumpers open for 0x27. Independently measure M100 PRO 5V-powered TX/SDA/SCL before adding its harness to the shared bus.
4. **Battery sensing:** run the GPIO-locked [R22 ADS1115 diagnostic](BOAT_R22_ADS1115_SENSOR_ONLY_BRINGUP.md) only after physical pin and ADC supply-versus-AIN0 ramp checks; record 9.0V–12.6V DMM and raw counts in [R21 worksheet](BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv).
5. **Radio harness without propulsion:** verify exact nRF 2×4 signal orientation and E22 carrier-pad mapping. Measure voltage levels, current and RF readiness with antennas attached and approved RF parameters. Neither radio is currently physically accepted.
6. **Actuator controls with NO HIGH-CURRENT INPUT:** check GPIO39/40 PWM outputs remain off during boot/reset, expander GPB0/1 and GPA0–3 are initialized low, and failure to address MCP23017 latches output **OFF**. Do not assume high current is disabled because a software test passes.
7. **Independent safety blocker:** select/verify real STOP/watchdog/cutoff/current-zero/pulse limiting and feedback wiring; test with load disconnected and physical contacts. The R23 matrix intentionally reserves GPIO1/2/41/42 but connects **none** to fictional safety equipment.
8. **Dummy loads, then physical testing:** only after safety hardware has been commissioned, record MDD20A direction and both motor current under safe limits and each EKM014 6A-class coil pulse with independent one-at-a-time enforcement. No water testing before these stages.

## 9. Software, tests, and acceptance

- `Boat/source/boat_r23_gpio_connection_contract.h` records 18 unique GPIO/header assignments, reserved pins, I²C addresses, MCP bank masks and **false** physical-commissioning flags.
- `Boat/tools/verify_gpio_connections_r23.py` cross-checks the R23 64-net CSV against the complete 44-position R20 J1/J3 crosswalk and selected radio macros; rejects unsafe pin use, unverified physical status and active outputs.
- `Boat/tests/boat_r23_gpio_contract_test.cpp` checks collisions, address/masks, JTAG assignment and radio macro consistency on host.
- `Boat/source/boat_radio_hw_config.h` stays R20 candidate with all radio GPIO/RF/logic-level flags **0**.
- `Boat/source/tracker_sensor_profile.example.h` stays with `pinAuditSigned=false` and ADC `commissioned=false`.
- `Boat/source/firmware/Flytec_Boat.ino` remains **non-actuating diagnostic** only.
- No physical measurements or output-driving hardware test has been performed. **Official Boat VER014 and Remote VER028 unchanged**. Sonar still deferred.

## 10. Manufacturer reference set

- [Espressif DevKitC-1 v1.1 J1/J3 header table](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html)
- [Espressif WROOM-2 N32R16V low-voltage GPIO47/48](https://documentation.espressif.com/esp32_s3-wroom-2_datasheet_en.html)
- [EBYTE E22-900T22S UART 26-pad reference](https://www.cdebyte.com/products/E22-900T22S/2)
- [Waveshare MCP23017 A0–A2 configuration](https://www.waveshare.com/wiki/MCP23017_IO_Expansion_Board)
- [HGLRC M100 PRO 5V/UBX/compass](https://www.hglrc.com/products/hglrc-m100-pro-gps)
- [Cytron MDD20A motor logic and power constraints](https://my.cytron.io/sumo-robot/c-motor-driver/p-20amp-6v-30v-dc-motor-driver-2-channels)
- [Electrokit EKM014 input logic/output connections](https://docs.electrokit.com/en/modules/EKM014/)

## R23 physical acceptance record — unsigned

A separate [R23 pin-by-pin electrical sign-off worksheet](BOAT_R23_GPIO_AND_LEVEL_SIGNOFF.csv) contains **18 MCU GPIO checks plus 14 module voltage/connector/safety checks**, all initially **NOT_RUN**. Record the actual DevKitC PCB revision, module/carrier revision, continuity and on/off logic levels in this sheet. An unfilled sheet is not proof of pin safety; it must be completed before a production GPIO audit can be signed.

### R23 CI closure (2026-11-22)

- [Boat host tests #45033370079](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45033370079): **PASSED** for R23 pin collision, 64-net wiring consistency, radio GPIO flags, historical-power/R23 current purchase register alignment, and the existing Boat safety tests.
- [Boat ESP32-S3 diagnostic target build #45033247797](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45033247797): **PASSED**; output remains locked/non-actuating.
- [Module-only and fail-closed hardware gate #45033496173](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45033496173): **PASSED** for current R23 README/procurement documentation.
- Physical acceptance still **NOT_RUN** for every [R23 GPIO and level worksheet](BOAT_R23_GPIO_AND_LEVEL_SIGNOFF.csv) row. No GPIO sign-off flag or power-output hardware acceptance was changed.

## R25 successor note — battery sense hold during power transitions

R23's 150k/33k ADC connection is **logical only**. R25 identified the unpowered-ADS1115 VDD-relative input limit as an explicit release blocker. Do not connect the switched 3S sense feed until safe sequencing/conditioning has been designed and accepted. See [R25 power/logic preflight](BOAT_R25_POWER_LEVEL_PREFLIGHT.md) for the 28-item unsigned test sheet and isolated module measurement order. R23 manufacturer GPIO allocation, 32 NOT_RUN physical checks, module selection and locked outputs remain unchanged.

## R27 successor — ADS1115 no longer active battery module (2026-11-22)

R23's GPIO7 SCL / GPIO8 SDA and sensor 3.3V rail are retained; however **R27 selected Ardboard INA231 product 1491** for voltage-only BUS measurement in place of ADS1115 0x48. Treat all R23 ADS1115 AIN0/wiring nets as **HISTORICAL, NOT CONNECTED**. Proposed INA231 address 0x40 depends on physical A1/A0 straps, and no final connector contact numbering has been signed. See [R27 active wiring requirements](BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md). All actual R23 physical signoffs stay NOT_RUN; do not energize any actuator.

## R29 power-only supersession — single common 3S bus (2026-11-22)

Per owner, **no separately switched motor and solenoid high-current supply rails**. Treat historic R23 MDD `MDD_MOTOR_SUPPLY` and EKM014 `EKM_VH` two independent protected-gate wording as historical. The [R29 current power plan](BOAT_R29_SHARED_BUS_FAULT_SIMULATION.md) supersedes these with **the SAME shared 3S4P battery bus / common return** feeding MDD20A VM and EKM014 VH in parallel, with no output power wired or commissioned. The existing EKM014 5V VL *logic* regulator is unchanged; all radio/sensor/GPIO/other R23 signal nets remain. Full current load/connector/fuse/reverse-polarity tests are still required. Never claim independent contactor feedback where no contactor is fitted.
