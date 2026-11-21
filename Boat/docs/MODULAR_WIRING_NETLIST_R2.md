# Modular boat wiring / netlist R2 (2026-11-21)
DESIGN-ONLY. Not a final GPIO-numbered or safety-approved schematic. No custom PCB, no RP2040, no external E22 on boat. Supersedes the high-level R1 wiring document where details differ.

## Wiring topology
BATT 3S4P -> BMS -> manual master disconnect -> main protective fuse (rating TBD after measurements) -> insulated positive distribution terminal. Negative return -> insulated negative distribution terminal. Separate fused feeds to MDD20A, EKM014 and a 3S-compatible ready-made regulated 5V module. Sonar reserve disconnected. Keep high-current paths away from logic/compass and use locking connectors.

### Motor driver MDD20A
- Motor supply VB+ / VB- from motor branch.
- M1A/M1B -> left brushed 550 motor.
- M2A/M2B -> right brushed 550 motor.
- DIR1, PWM1, DIR2, PWM2 -> four MCU GPIOs only after pin audit; GND -> logic reference.
- Manufacturer PWM truth table: PWM low = brake, PWM high and DIR low = forward, PWM high and DIR high = reverse (actual orientation depends on motor polarity).
- Never power motors until hardware STOP/disable behavior, current limits, thermal behavior and startup sequencing are bench-verified.

### EKM014
- VH load supply -> protected battery branch (3S 9–12.6V), VL driver supply -> separate regulated 5V branch; VH >= VL always.
- EKM014 GND -> common reference; do not apply 12.6V to VL (12V maximum).
- IN1 -> LEFT_BAIT, IN2 -> RIGHT_BAIT, IN3 -> LEFT_HOOK, IN4 -> RIGHT_HOOK (logical GPIO signals, exact pins TBD).
- Channel 1/2/3/4 load + and - terminals -> corresponding solenoid coil positive and switched return. Use board terminal labeling as authority.
- Integrated 10A slow-blow VH fuse is not a substitute for upstream wiring protection or one-at-a-time software interlock. Verify coil 6A assumption and board heating.
- All outputs OFF at startup; use hardware-safe reset defaults, independent pulse timeout and no simultaneous actuation.

### Controller, RF and sensors
- Heltec Wireless Tracker v1.2: regulated 5V to board-designated 5V input ONLY after verifying exact board revision and manufacturer power specification.
- Integrated SX1262 LoRa: no external wiring beyond antenna.
- Integrated GNSS: use its proper GNSS antenna; keep away from motor noise.
- nRF24L01+ PA/LNA: regulated clean 3.3V (not 5V), CE/CSN/SCK/MOSI/MISO plus optional IRQ on GPIOs TBD. External decoupling, RF cable and separate antenna.
- MPU9250+BMP280 breakout: I2C SDA/SCL to vetted MCU pins, power per breakout datasheet, calibrated heading and magnetic separation from motors.
- Battery telemetry: approved battery monitor module or protected ADC front end after design review. Never direct 3S to GPIO.

## GPIO budget
Direct MCU signals requested: 4 motor + 4 solenoid + 5 nRF SPI/control (+ optional IRQ) + 2 I2C = 15–16 logical functions before reserved onboard LoRa/GNSS/display. Off-the-shelf MCP23017/TCA9555 expander can move 4 slow solenoid inputs (and other slow status pins) to I2C; do not place motor PWM or timing-critical SPI on an expander. If safe GPIOs remain insufficient, redesign module allocation rather than assigning forbidden pins.

## Revision-specific pin gate
Heltec V1.1 documents mention LoRa GPIO8–14, GNSS GPIO33/34 and several TFT/boot pins. THESE ARE NOT VERIFIED FOR V1.2. No final numeric GPIO map is authorized until V1.2 official schematic, board markings and firmware peripheral pin use are reconciled. USB/boot/strapping and onboard battery ADC pins require special caution.

## Engineering verification
Bench dummy loads first; validate common ground, 5V and 3.3V under radio transmit, EKM014 trigger polarity, watchdog STOP, mutual exclusion, motor polarity and 3S transient margin. Select fuses, wire gauge and connectors after measuring both motors' stall/start currents and solenoid resistance/thermal duty. Avoid water operation until independent failsafe and RF testing.
References: https://docs.electrokit.com/en/modules/EKM014/ ; https://cdn.robotshop.com/media/c/cyt/rb-cyt-287/pdf/cytron-20a-6v-30v-dual-dc-motor-driver-datasheet.pdf ; https://heltec.org/project/wireless-tracker/
