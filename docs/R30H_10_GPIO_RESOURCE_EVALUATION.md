# R30H-10 — passive GPIO assignment for physical testing (2026-11-22)

**Status: candidate allocation and read-only preflight only. No GPIO electrically accepted, no flashing or RF authorization.** VS Code + PlatformIO is the development environment.

## Boat ESP32-S3-DevKitC-1-N32R16V
See [exact GPIO/header ledger](../Boat/docs/BOAT_R30H_10_PIN_MAP.csv) and `Boat/source/r30h10_gpio_test_profile.h`.

- GNSS HGLRC M100 PRO: UART TX GPIO4, RX GPIO5. SCL GPIO7 and SDA GPIO8 shared I²C. M100 compass QMC5883L candidate address 0x0D. GY-91 MPU9250 0x68 with magnetometer 0x0C, BMP280 0x76/77, INA231 BUS-only 0x40 and MCP23017 0x27. Probe 3.3 V compatibility and strap identity; no 5V I²C pullups.
- nRF24: CSN9 CE10 MOSI11 SCK12 MISO13 IRQ21; no RF / GPIO authorization yet.
- E22-900T22S 26-pad module: NRST6 AUX14 M0=15 M1=16 UART TX17 (to E22 RX) RX18 (from E22 TX); check carrier levels/pad order and legal EU868 antenna.
- MDD20A PWM1=39 PWM2=40 **reserved but never enabled**. Direction via MCP23017@0x27 GPB0/1. EKM014 IN1–IN4 via MCP23017 GPA0–3. No motor or coil power; independent electrical STOP remains unproven.
- GPIO1/2/41/42 reserved for separate future safety/feedback arrangement. GPIO35–37 memory and 47/48 1.8 V on WROOM-2 V variants; do not use. GPIO0/3/45/46 strapping, 19/20 USB, 43/44 UART console, RGB LED 38/48 revision-dependent.
- GNSS PPS is **-1 / unassigned**. The documented M100 PRO external cable has no identified PPS pin; UART UTC is not physical PPS.

## Remote Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU25948
See [exact GPIO/resource ledger](../Remote/docs/REMOTE_R30H_10_PIN_MAP.csv) and `Remote/source/r30h10_gpio_test_profile.h`.

- I²C SDA8/SCL9 shared by CH422G/touch and the external MCP23017 (button 0x20, confirm 0x21), ADS1115 joystick 0x4A, SC16IS750 E22 UART 0x48. SC16IS750 bridge GPIO0=M0, GPIO1=M1, GPIO2=AUX, GPIO3=NRST. These are **expander GPIOs**, not ESP GPIOs.
- TF SPI MOSI11/SCK12/MISO13: **already owned by SD**. Real TF CS=CH422G EXIO4; software currently asserts it while SD mounted. Concurrent nRF SPI requires software bus/chip-select arbitration before RF tests.
- Sensor AD GPIO6 is currently SD library dummy-CS; onboard RS485 uses TX GPIO15/RX GPIO16. Former remote nRF24 GPIO6 CE / GPIO15 CSN conflict with them. **Current direct nRF CE=-1 and CSN=-1** in live source; RF24 library does not implement MCP23017 CSN/CE. No safe direct pair yet approved.
- RGB screen occupies GPIO0/1/2/3/5/7/10/14/17/18/21/38–42/45–48; touch IRQ4; USB19/20; USB-UART43/44. Do not repurpose.
- Remote PPS=-1. Authenticated GPS UTC from Boat is wall-clock information, not a physical PPS discipline.

## R30H-10 safe passive test order
1. Photograph and identify actual PCB revisions, header connector orientation and supply levels; use current-limited isolated **logic-only** power.
2. With all radio/actuator harnesses disconnected, passive analyzer capture of existing TF/CH422G/I²C/USB signals; no new probe output or firmware pin drive.
3. Verify I²C device addresses and voltage/pullups before adding optional peripherals; GNSS UART identity/baud on Boat.
4. Measure Waveshare TF CS ownership and SPI idle behavior; design a reviewed bus-arbitration solution before sharing SPI with nRF.
5. Only after an independent hardware review (including direct nRF CE/CSN design, physical modules/antennae, and logic levels) prepare separate RF-only firmware; no motor or solenoid power.
6. PPS only if genuine receiver output is identified and electrically verified; otherwise record PPS=NOT_AVAILABLE. Independent STOP/current-zero requires separate dummy-load acceptance and is blocked.

All existing 18 H10 physical gates remain unsigned. GitHub CI checking these files is **not** a physical signoff. Boat VER014 / Remote VER028 unchanged.

Official manufacturer sources: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html and https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3 .
