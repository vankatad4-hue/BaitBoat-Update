# R42 — Figure-eight-only boat compass calibration
2026-11-21. Selected operating procedure: **on-water figure-eight only**. No ground rotation or 3-axis manoeuvres required.

## Two compass sources
- GY-91: MPU-9250 integrated AK8963 magnetometer; existing I2C initialization at 0x0C.
- HGLRC M100 PRO: manufacturer advertises an I2C QMC5883L compass alongside GNSS UART UBX at 115200 baud, 3.6–5.5 V supply. The GPS UART is **UBX**, not automatically NMEA; existing tracker GNSS parser must be adapted before GPS can be called integrated.
- 2026 HGLRC production variation: QMC5883P may replace QMC5883L. Current R42 decoder only supports verified QMC5883L at I2C 0x0D; unsupported QMC5883P fails closed until its identity, registers, scale and conversion are separately implemented. Never assume identical register maps.
- Each sensor has its own raw sample collection and independent horizontal calibration candidate.

## Operating procedure
1. Install both compasses in their final positions. Keep at least adequate clearance from motor wires, magnets and coils.
2. With independent hardware STOP and actuator protections already accepted, enter compass calibration mode. Calibration must never energize motors or bypass STOP itself. Drive manually, slowly and continuously in repeated figure-eight patterns, including both left and right full curves, on calm water.
3. Record at least 150 valid samples per sensor, at least nine of twelve horizontal sectors, and plausible X/Y spans. The algorithm estimates XY hard-iron offsets and diagonal XY scaling. It **cannot** solve vertical Z-axis calibration, sensor tilt or full soft-iron ellipsoid.
4. Finish; reject the entire dual-sensor calibration if either sensor is missing or has insufficient coverage. Review candidates and verify heading against stable GNSS course when moving, accounting for current, wind and slip. Do not blindly set heading.commissioned=true.
5. Never automatically persist or apply candidates without validated sensor mounting orientation and sign convention. In-water heading checks must compare both magnetometers and reject sustained disagreement.

## Implemented now
- R42 figure-eight accumulator and tests, GY-91 and QMC5883L acquisition, dual-sensor candidate API, diagnostic serial c/f.
- **Not implemented**: remote UI control of calibration; QMC5883P detection/driver; nonvolatile save/apply; verified horizontal body-axis mapping; real compass agreement/fusion; true M100 UBX GNSS parser; physical on-water acceptance. No claim of completed dual-compass calibration or autonomous navigation.

## Hardware note
The QMC5883L is a separate compass on the M100 board, accessed through SDA/SCL; it is **not calibrated by sending UBX commands to GPS**. M100 supply 3.6–5.5 V does not establish safe I2C logic levels; verify the exact board before connecting ESP32-S3 GPIO.
