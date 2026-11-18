# Boat controller and navigation sensor selection — ver003

## Selected hardware architecture

The project selection is updated to use the **Heltec Wireless Tracker V1.2** as the boat-side main processor and GNSS board, replacing the previously planned generic 38-pin ESP32 carrier plus the separate HGLRC M100 PRO B101 GNSS module. The integrated GNSS chipset is identified by Heltec as **UC6580** and the processor as **ESP32-S3FN8**. The board also contains an **SX1262 LoRa** radio; it is not the selected command/telemetry radio for this project and is left unused unless a later decision changes that. The selected remote-to-boat link remains **nRF24L01+ PA/LNA at 2.4 GHz**, initially tested at 250 kbps.

The selected external motion/heading sensor is a **GY-91-style MPU-9250 + BMP280 module**. The MPU-9250 supplies 3-axis accelerometer, gyroscope and magnetometer measurements, consolidating the previously discussed QMC5883L compass and MPU6050 IMU roles into one sensor package. The BMP280 supplies barometric pressure and temperature; it is not an underwater depth sensor.

## Integration rules and unresolved verification

- Confirm the physical board is specifically Wireless Tracker V1.2. Obtain and use the matching V1.2 schematic and pin map before assigning GPIOs. Do not copy pin assignments from V1.1 or V2.
- Confirm GNSS antenna arrangement and whether the external GNSS IPEX/U.FL antenna option is populated/usable on the purchased board. The GNSS antenna needs a clear sky view; do not bury it beneath motors, battery wiring or metal.
- Confirm the exact V1.2 GNSS power-enable and UART routing. Heltec's V1.1 hardware update log describes a GNSS power-control change; do not assume the V1.1 GPIO3 instruction applies unchanged to V1.2.
- Confirm the actual GY-91 board's chip markings, regulator/level-shifter presence, supply-voltage range, I2C address and pin labels. GY-91 breakout boards are not all identical. Use 3.3 V logic with the ESP32-S3 unless the exact board documentation proves otherwise.
- Prefer I2C for the initial GY-91 connection to simplify wiring; SPI remains an option only after checking the exact breakout pinout and available Heltec pins. No GPIO assignment is approved yet.
- Calibrate the MPU-9250 magnetometer after final installation. Mount it as far as practical from motors, motor leads, solenoids, high-current wiring and steel parts. Perform hard-/soft-iron calibration with the boat fully assembled and check heading with motors both off and running.
- Use GNSS course-over-ground as a useful heading aid only while moving sufficiently fast; it is not a replacement for a calibrated magnetic heading sensor at low speed or while stationary. Fuse GNSS, magnetometer and gyro data with validity checks before autopilot use.
- The BMP280 measures atmospheric pressure. Do not use it to infer water depth. The selected CHIRP sonar remains a separate subsystem.
- Heltec's board includes SX1262 LoRa, but the project's selected nRF24L01+ PA/LNA radio remains external and unchanged. Verify a dedicated low-noise 3.3 V supply for the nRF24 PA/LNA module.
- The Tracker's published dimensions are approximately 65.48 × 28.06 × 13.52 mm; check connector, antenna and mounting clearance against the 77 × 79 mm PCB envelope and 30 mm compartment height.
- The Tracker is a development board, not a motor-driver board. External protected motor and solenoid driver stages, watchdogs, emergency stop and failsafe logic remain required.

## Firmware status

This is a **hardware architecture selection only**. No Heltec GNSS parser, MPU-9250/BMP280 driver, sensor fusion, nRF24 driver, motor driver, solenoid driver, autopilot or failsafe implementation is added by this release. Boat firmware remains a non-actuating scaffold. Do not connect powered actuators or operate the boat using this scaffold.

## Manufacturer / component references

- Heltec Wireless Tracker product specifications and board resources: https://heltec.org/project/Wireless-Tracker/
- Heltec Wireless Tracker documentation and hardware update log: https://wiki.heltec.org/docs/devices/open-source-hardware/esp32-series/lora-32/wireless-tracker/hardware-update-log
- MPU-9250 product specification: https://www.digikey.com/en/resources/datasheets/invensense/mpu-9250-product-spec
- BMP280 product page / datasheet: https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/
