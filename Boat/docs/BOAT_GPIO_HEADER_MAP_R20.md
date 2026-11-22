# Boat R20 — ESP32-S3-DevKitC-1-N32R16V GPIO/header and wiring map

Date: 2026-11-22  
Branch: `hardware-ready-dual-radio-r1`  
Status: **candidate map only; not signed or commissioned.** The board has not been purchased, so its PCB revision and exact module marking must be checked before wiring. Firmware pin-audit flags remain 0 and the sensor profile remains `pinAuditSigned=false`.

## Board constraints

The selected N32R16V option uses the ESP32-S3-WROOM-2-N32R16V module. Follow the Espressif DevKitC-1 header tables and the N32R16V module-specific memory restrictions.

- Reserve GPIO35–37 for the N32R16V octal flash/PSRAM interface.
- Reserve GPIO38 and GPIO48 because the DevKitC-1 RGB LED uses GPIO38 on v1.1 and GPIO48 on the initial board revision.
- Reserve GPIO0, GPIO3, GPIO45 and GPIO46 for boot/strapping behavior; do not connect external circuitry that can alter their reset levels.
- Reserve GPIO19/20 for native USB D−/D+ and GPIO43/44 for the USB-UART bridge.
- GPIO39–42 are JTAG pins. The candidate uses GPIO39/40 for motor PWM; use USB serial for debug and do not rely on JTAG while those pins are assigned.
- Do not apply 5V logic to ESP32-S3 or MCP23017 pins. All control and I²C signals are 3.3V logic.

## Candidate GPIO allocation

| Function | ESP32-S3 GPIO | DevKitC-1 header | Module connection | Notes |
|---|---:|---|---|---|
| M100 PRO UART RX | GPIO5 | J1-5 | M100 TX → ESP RX | UART2 candidate; 115200 baud |
| M100 PRO UART TX | GPIO4 | J1-4 | ESP TX → M100 RX | UART2 candidate |
| Shared I²C SCL | GPIO7 | J1-7 | GY-91, QMC5883L, ADS1115, MCP23017 SCL | 3.3V pull-ups only |
| Shared I²C SDA | GPIO8 | J1-12 | GY-91, QMC5883L, ADS1115, MCP23017 SDA | 3.3V pull-ups only |
| nRF24 CSN | GPIO9 | J1-15 | nRF24 CSN | SPI |
| nRF24 CE | GPIO10 | J1-16 | nRF24 CE | Radio control |
| nRF24 MOSI | GPIO11 | J1-17 | nRF24 MOSI | SPI |
| nRF24 SCK | GPIO12 | J1-18 | nRF24 SCK | SPI |
| nRF24 MISO | GPIO13 | J1-19 | nRF24 MISO | SPI |
| nRF24 IRQ | GPIO21 | J3-18 | nRF24 IRQ | Optional interrupt; polling remains possible |
| E22 NRST | GPIO6 | J1-6 | E22 pad 1 NRST | Active-low reset |
| E22 M0 | GPIO15 | J1-8 | E22 pad 20 M0 | Set low for normal mode |
| E22 M1 | GPIO16 | J1-9 | E22 pad 21 M1 | Set low for normal mode |
| E22 UART TX | GPIO17 | J1-10 | ESP TX → E22 pad 22 RXD | UART1 candidate; 3.3V TTL |
| E22 UART RX | GPIO18 | J1-11 | ESP RX ← E22 pad 23 TXD | UART1 candidate; 3.3V TTL |
| E22 AUX | GPIO14 | J1-20 | E22 pad 24 AUX | Input |
| MDD20A PWM1 | GPIO39 | J3-9 | MDD20A PWM1 | 3.3V logic; JTAG pin |
| MDD20A PWM2 | GPIO40 | J3-8 | MDD20A PWM2 | 3.3V logic; JTAG pin |
| MDD20A DIR1 | MCP23017 GPB0 | I²C expander | MDD20A DIR1 | R20 expander adapter uses bank B |
| MDD20A DIR2 | MCP23017 GPB1 | I²C expander | MDD20A DIR2 | Change only with PWM zero and motor power isolated |
| Left bait hopper | MCP23017 GPA0 | I²C expander | EKM014 IN1 | Active-high logical request |
| Right bait hopper | MCP23017 GPA1 | I²C expander | EKM014 IN2 | Active-high logical request |
| Left hook | MCP23017 GPA2 | I²C expander | EKM014 IN3 | Active-high logical request |
| Right hook | MCP23017 GPA3 | I²C expander | EKM014 IN4 | Active-high logical request |

### I²C address plan

| Module | Expected address | Bus |
|---|---:|---|
| QMC5883L compass on M100 PRO | 0x0D | GPIO7/8 |
| GY-91 MPU-9250 | 0x68 | GPIO7/8 |
| GY-91 BMP280 | 0x76 (verify scan; some boards strap 0x77) | GPIO7/8 |
| ADS1115 battery ADC | 0x48 (ADDR tied GND) | GPIO7/8 |
| Waveshare MCP23017 | 0x27 (verify A0/A1/A2 straps on purchased module) | GPIO7/8 |

Run an I²C scan before commissioning. No expected address proves the module is wired correctly.

## E22 module pad wiring and power

The selected E22-900T22S is the 26-pad UART module. Verify the exact suffix and pad numbering on the purchased module before soldering or using a premade carrier:

- Pad 1 NRST → GPIO6.
- Pad 20 M0 → GPIO15.
- Pad 21 M1 → GPIO16.
- Pad 22 RXD ← ESP32 GPIO17 TX.
- Pad 23 TXD → ESP32 GPIO18 RX.
- Pad 24 AUX → GPIO14.
- Pad 25 VCC → regulated 5V.
- Pad 12 ANT → 50Ω 868MHz antenna feed.
- Connect all module GND pads to the RF/power ground. Do not use the antenna shield as the only ground return.

**Power correction:** The selected E22-900T22S supply is the Boat 5V main rail. EBYTE specifies a range including 3.3V and 5V (typical 5V); 5V is the chosen supply allocation, not an absolute requirement for basic operation. UART/control logic remains 3.3V TTL. The DCDC6-16-TO5 5V/2A rail is the candidate source, subject to measured peak current, ripple and temperature. Never feed 5V into RXD, TXD, M0, M1 or AUX.

Use a premade carrier/breakout for the SMD E22 module; do not design a custom PCB.

## Power and signal distribution

| Rail | Source | Loads |
|---|---|---|
| 5V main | Olimex DCDC6-16-TO5, 5V/2A | ESP32-S3 DevKitC-1 5V input, M100 PRO, E22-900T22S VCC |
| 5V EKM014 logic | TRACO TSR 2-2450 | EKM014 VL only |
| 3.3V RF | Pololu D24V10F3 #2830 unit 1 | nRF24L01+ PA/LNA |
| 3.3V sensor/I/O | Pololu D24V10F3 #2830 unit 2 | GY-91, ADS1115, MCP23017 |
| 3S battery motor path | Fused battery distribution | Cytron MDD20A motor supply |
| 3S battery actuator path | Separately fused battery distribution | EKM014 VH; VL remains separate 5V |

- Do not parallel regulator outputs.
- Keep motor/solenoid high-current returns separate from sensor/radio return wiring until the planned battery-negative distribution point.
- Add local manufacturer-recommended decoupling at nRF24 and E22. Verify actual regulator stability with real modules before RF range tests.
- DevKitC-1 power options are mutually exclusive. During bench tests, avoid backfeeding its 5V rail from both USB and external DCDC6; use a data-only USB cable or isolate USB 5V when externally powering the board.
- EKM014 VH is the load rail and VL the logic rail. Candidate: VH from 3S pack, VL from TSR 2-2450 5V. Keep VH disconnected during logic-only tests.

## Safety and unassigned signals

The following are deliberately **not assigned to arbitrary GPIOs** until the physical interface is selected and verified:

- Physical STOP loop and NC contact wiring.
- Independent watchdog contact/feedback.
- Motor isolator/contactor feedback.
- Coil isolator feedback and physical coil-current-zero feedback.
- Independent non-retriggerable EKM014 pulse-limiter ready/feedback.

The current `BoatPhysicalDriverAdapter` refuses to initialize unless the independent safety contacts and independent pulse-limiter/current-zero feedback are commissioned. The R20 BOM does not yet contain a selected module providing all those signals. Low-battery software can hold outbound motion and request Home diversion, but if a release is active and physical idle/current-zero feedback cannot be confirmed, the correct response is STOP, not an assumed-safe RTH. Do not bypass this gate to make the GPIO map appear complete.

## Firmware status

- `Boat/source/boat_radio_hw_config.h` contains candidate radio GPIO values, but pin-audit/logic-level/interoperability flags remain 0.
- `Boat/source/tracker_sensor_profile.example.h` contains candidate GPS/I²C pins and ADC ratio, but `pinAuditSigned=false` and the battery profile remains uncommissioned.
- `Mcp23017DirectionAdapter` uses address 0x27 and GPB0/GPB1 for MDD20A direction.
- `Ekm014Mcp23017Adapter` uses GPA0–GPA3 for one-hot EKM014 input requests. Its GPIO readback is not coil-current feedback and does not replace the independent pulse guard.
- Actual motor/solenoid output GPIOs remain disabled in the diagnostic target.

## Sign-off checklist

1. Buy/inspect exact ESP32-S3-DevKitC-1-N32R16V board and record PCB revision.
2. Verify header numbering and candidate pins against that board's schematic.
3. Verify E22 suffix, pad numbering, 5V VCC, 3.3V logic and premade carrier.
4. Verify M100 PRO connector labels before applying 5V.
5. Verify MCP23017 address straps and I²C scan.
6. Calibrate ADS1115 divider against a DMM and update the commissioned sensor profile.
7. Measure rail current/ripple/temperature under simultaneous GPS and radio TX.
8. Verify MDD20A and EKM014 mapping using dummy loads only.
9. Resolve independent STOP/watchdog/pulse-limiter/current-zero feedback before enabling physical actuator outputs.

## Manufacturer references

- [Espressif ESP32-S3-DevKitC-1 v1.1 user guide and J1/J3 header tables](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html)
- [Espressif ESP32-S3-DevKitC-1 board and N32R16V ordering information](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/)
- [HGLRC M100 PRO product page](https://www.hglrc.com/products/hglrc-m100-pro-gps)
- [Cytron MDD20A product page](https://www.cytron.io/p-20amp-6v-30v-dc-motor-driver-2-channels)
- [Texas Instruments ADS1115 datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf)

Use the exact EBYTE E22-900T22S user manual for the purchased suffix/revision. Do not substitute an E22-900T22S1C or V2 pinout.

## R21 ADC input/wiring revision

The GPIO7/8 I²C candidate mapping remains unsigned. ADS1115 AIN0 now uses the superseding [R21 150kΩ/33kΩ battery divider](BOAT_R21_BATTERY_ADC_COMMISSIONING.md), not the original R20 100kΩ/33kΩ proposal. Validate the input relative to its 3.3V VDD during every power transition, including the unpowered state; PGA ±4.096V does not change the analog input absolute maximum rating.

## R23 update: manufacturer-checked header assignments and full module connections

[R23 GPIO/header + harness guide](BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md) and [64-net machine-readable connection matrix](BOAT_R23_MODULE_CONNECTION_MATRIX.csv) take precedence for actual proposed module-to-module wiring. The Espressif DevKitC-1 published J1/J3 header positions match all 18 assigned GPIOs in the R20 crosswalk; this is **documentation-level verification only**, not physical verification of the yet-to-be-purchased PCB or module connector orientation.

Notable electrical holds: M100 PRO powered at 5V but TX/I²C HIGH must be tested against ESP32's 3.3V tolerance; E22 TXD/AUX at 5V VCC still needs level measurement; Waveshare MCP23017 `A0/A1/A2` open HIGH = I²C `0x27`; 5V EKM014 VL accepts 3.3V GPIO inputs and separate 3S VH must stay disconnected. The updated purchase register correctly assigns two separate Pololu D24V10F3 units to nRF24 and sensors/I²C. Reserved GPIO1,2,41,42 remain unwired pending actual safety interface selection. **All firmware actuator and radio audit flags remain false.**
