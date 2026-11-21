# R41 GY-91 compass calibration — bench procedure
Source-only candidate workflow; not a hardware acceptance certificate.

1. Keep **both motor and all solenoid power disconnected**. Mount GY-91 in its final orientation away from motors, coils, current-carrying wiring, steel hardware and radio PA.
2. Flash the diagnostic ESP32-S3 boat sketch with audited M100 PRO UART and GY-91 I2C pins, start Serial Monitor at 115200 baud.
3. Type **c** to start the RAM-only magnetic sweep. Rotate the whole boat/electronics assembly slowly through multiple orientations, including roll/pitch/yaw. Avoid magnets, keys, metal desks and energized solenoids.
4. Type **f** to finish. At least 150 valid magnetometer samples, plausible 3-axis field span and scale factors are required. Invalid sweeps return no calibration.
5. Capture the printed **candidate** offsets (uT) and diagonal scales. Review repeatability, magnetic distortion and compass orientation; copy approved values into the commissioned TrackerSensorConfig.heading.magOffset and magMatrix (diagonal) with the correct body axes and independently verified accelerometer orientation matrix.
6. Verify magnetic north against a reliable compass at multiple headings; set local declination for true north if desired. Repeat with motor currents flowing ONLY after independent safety hardware has been physically accepted.
7. Only then sign off heading.commissioned=true. No automatic flash/NVS save or autopilot authorization occurs in this tool.

This is a min/max hard-iron + diagonal soft-iron **initial estimate**, not a full ellipsoid-fit or tilt/axis alignment calibration. In particular, no yaw-only spin is sufficient. The carrierHeading function already rejects uncommissioned coefficients, bad field strength and invalid tilt.

R40 BMP280 data remains a **local** sensor observation. Pressure is station pressure, not sea-level adjusted; temperature may reflect hull heating. GNSS source remains HGLRC M100 PRO, not the GY-91.
