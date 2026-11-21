# R53 — dual-slot calibration persistence interface

Implemented `Boat/source/figure8_calibration_store.h` with `Figure8StoreIo` readSlot/writeSlot callbacks, versioned CRC32 validation, sequence-based slot selection, write to inactive slot, read-after-write verification and recovery to the last valid slot. `Boat/tests/figure8_calibration_store_test.cpp` checks first write, second write, corrupted new slot, failed write and invalid candidate. CI host gate added.

**Not yet a physical persistent store:** No ESP32 Preferences/NVS or SD-card read/write callbacks connected; backend must guarantee complete-record atomic replacement or use a transactional journal. This component must not be described as live calibration saving. A verified, authenticated local/Remote workflow must authorize candidate acceptance; record CRC detects accidental corruption, not malicious tampering. No automatic activation on startup until sensor calibration provenance and quality tests are commissioned.

Remaining: NVS transactional backend, field-proven calibration limits, GPS geofence/radius controller, authenticated Remote Start/Abort and abort telemetry, physical independent safety hardware, target builds and hardware tests. Official Boat VER014 unchanged.
