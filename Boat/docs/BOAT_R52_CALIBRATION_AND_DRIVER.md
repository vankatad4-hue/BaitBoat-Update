# R52 — calibration persistence format and driver safety

- Fixed EKM014 host test feedback timestamp during active pulse.
- Physical driver adapter refuses nonzero motor demand while coil-current-zero is unconfirmed or coil pulse settling is pending.
- Added `figure8_calibration_record.h` with versioned fixed-format CRC32 records, sanity checks, sequence-based selection of valid dual-slot calibration records. Host regression verifies corruption fallback.
- **Storage backend is not implemented**: flash NVS/SD transactional writes, atomic activation, signed provenance and field validation remain pending. Do not claim persistent calibration is operational.
- **Automatic figure-eight remains incomplete**: GPS path/radius/geofence controller, real motion output and authenticated Remote Start/Abort are not implemented. R46 heading-progress supervisor alone does not provide autonomous trajectory.
- R51 last observed host run 44951517101 success and ESP32-S3 diagnostic target 44951495536 success; these predate R52 changes.
- No EKM014 physical outputs, no independent watchdog or contactor, no physical commissioning. Boat VER014 unchanged.
