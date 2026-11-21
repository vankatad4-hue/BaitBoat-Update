# Modular hardware purchase and substitutes R2 — 2026-11-21

## Locked functional choices
- Heltec Wireless Tracker V1.2 with integrated GNSS + SX1262 EU868; exact SKU and pinout must be checked. https://heltec.org/project/wireless-tracker/
- Cytron MDD20A dual brushed motor driver. https://www.cytron.io/distributor
- Electrokit EKM014 4-channel low-side MOSFET module, part 41024394. https://docs.electrokit.com/en/modules/EKM014/
- nRF24L01+ PA/LNA primary boat radio, separate from integrated LoRa.
- Ready-made 5V DC/DC module, and separately clean regulated 3.3V for PA/LNA radio.
- MPU9250/BMP280 sensor breakout, genuine part verification required.
- Existing 3S4P battery and 2 motors + 4 solenoids retained subject to measurement.

## Substitute policy
- EKM014 unavailable: 2x EKM013 dual-channel modules ONLY after verifying ratings, logic polarity, thermal behavior and one-at-a-time control. https://docs.electrokit.com/en/modules/EKM013/
- MDD20A unavailable: 2x equivalent independent reversible brushed DC motor drivers with PWM/DIR and demonstrated motor stall-current margin; do not substitute single-direction MOSFET boards.
- MPU9250 unavailable/genuine status doubtful: ICM-20948 breakout + BMP280, requiring driver/calibration update; do not call drop-in.
- Heltec V1.2 unavailable: hold controller purchase until revised schematic and software resource audit; V1.1 pinout is not evidence for V1.2.
- DC/DC unavailable: any branded, assembled regulator with verified 9–12.6V plus transient input, 5V output, measured peak-current margin and thermal evidence; avoid unverified anonymous boards.

## Supplier shortlist
Manufacturer Heltec https://heltec.org/ ; Cytron distributors https://www.cytron.io/distributor ; Farnell Bulgaria https://bg.farnell.com/ ; Bulgarian electronics retailer https://www.erelement.com/ ; Electrokit https://www.electrokit.com/ ; AliExpress only after seller, exact SKU and datasheet review. These are supplier/catalogue links, not verified product availability or current prices.

## STOP PURCHASE for unselected parts
Do not order main fuse, branch fuse, cable gauge, connector rating, DC/DC converter or alternative motor driver until actual motor stall/start current, solenoid current and BMS performance are measured. Do not buy a purported 3.3V nRF adapter unless its PA/LNA peak current and filtering are verified.
