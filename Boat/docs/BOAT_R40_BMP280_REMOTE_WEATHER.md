# R40 — GY-91 BMP280 weather telemetry and code audit
2026-11-21. Source implementation only; physical validation pending.

## Boat
- MPU9250+AK8963 heading remains on GY-91. BMP280 probes I2C 0x76 and 0x77, verifies chip ID 0x58, reads factory calibration and compensated 20-bit pressure/temperature.
- Bosch integer compensation with checked pressure 30000..120000 Pa and temperature -40..85 C, rejects uncalibrated or skipped raw values. Normal-mode x1 pressure/temperature, filter x4, 1000ms standby. Poll 1s; freshness expires at 5s.
- HGLRC M100 PRO remains dedicated GNSS candidate. No assumption of M100 compass.
- Diagnostic boat firmware sends subtype 7 weather packet every 5s when sensor is valid and authenticated primary radio is fresh. This is NOT proof of RF integration.

## Wire and Remote
- Added telemetry subtype 7, eight bytes: [7, temperatureCentiC int16 LE, pressurePa uint32 LE, reserved zero]. Both Boat and Remote decoder validate physical bounds.
- Remote stores received pressure/temperature, invalidates weather after 10s, displays Bulgarian 'ВРЕМЕ' label with C and hPa or dashes when stale.
- This is **local boat sensor weather**, not a meteorological forecast. Temperature can be biased by electronics heat and pressure is not sea-level corrected. No weather prediction/alerts.

## Audit / tests
- Added Bosch BMP280 reference compensation and invalid sample tests.
- Added shared Boat/Remote weather payload decoder tests and CI gate. Previous remote telemetry host CI had a transient failure when boat_telemetry.cpp was committed before BoatState; a later run succeeded after BoatState update.
- Prior host suites passed on the branch, but R40 newest CI and complete target builds need their own green runs. No claim of exhaustive full-source review; audit focused on sensor, payload, UI and cross-radio safety boundaries.
- Hardware pinout, real BMP280/GY-91 I2C, M100 GNSS, E22 RF and independent actuator safety remain uncommissioned.

## Follow-up
- Validate actual GY-91 chip identity and BMP280 compensation with a reference barometer/thermometer, confirm I2C electrical levels and self-heating.
- Target-build both devices, review UI glyph/font and screen geometry, test weather packet age/failure and real radio telemetry.
- Keep actuators disconnected until independent STOP, watchdog, contactor and coil onehot interlocks are physically proven.
