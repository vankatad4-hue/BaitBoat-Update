# R55 — storage architecture correction

User correction: the SD card is **integrated in the Waveshare Remote**, not the ESP32-S3 Boat. Reverted all R54 external Boat microSD module sources, test, documentation and workflow entry. Do not procure or wire a Boat microSD module.

## Storage ownership
- **Remote SD:** offline maps, 8 lake packs, 16 waypoints, Home and route data, map index. Remote existing SD code must be audited independently; no new Remote hardware required.
- **Boat ESP32-S3 internal NVS:** local figure-eight compass calibration and other safety-critical local settings. No dependence on Remote SD for Boat startup.
- **Telemetry:** Boat may send calibration state/results to Remote, but remote SD is not the authoritative Boat calibration store.

Added `Boat/source/figure8_calibration_nvs.h/.cpp`: Arduino Preferences-backed independent slot A/B keys, compatible with R53 `Figure8CalibrationStore`. NVS write returns success only after Preferences reports full blob stored. CRC and read-back verification remain in the store manager.

**Not wired into Boat sketch or target-compiled in this commit.** Need an explicitly authorized calibration candidate acceptance path, quality checks, NVS failure/rollback injection tests and boot-time validation before applying calibration. Do not auto-apply unchecked records. All actuators remain disabled pending independent hardware safety acceptance.
