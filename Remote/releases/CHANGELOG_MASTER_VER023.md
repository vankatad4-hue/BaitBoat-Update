# Consolidated development changelog — VER023

**Date:** 2026-11-20  
**Branch:** `hardware-ready-dual-radio-r1`  
**Scope:** Target compile workflow correction, compile evidence, continuation handoff. Development only; package marker remains 028.

## Verified build
- Remote target compile run #19 (GitHub Actions ID `44781879101`) passed at commit `3f189cb1ac65a764aec678893f1df857d113c815`. Full compile/link succeeded and firmware artifact uploaded.
- Build config: ESP32-S3 16MB Flash, OPI PSRAM, `PartitionScheme=huge_app`; ESP32 Arduino core 3.0.2, LVGL 9.6.0, ESP32_Display_Panel 1.0.0, ESP32_IO_Expander 1.0.1, esp-lib-utils 0.1.2, RF24 1.4.11.
- Sketch: 1,430,957 bytes (45% of 3,145,728 app bytes). Global RAM: 178,136 bytes (54% of 327,680; 149,544 bytes remaining). No compiler errors or C++ warnings reported.
- Build workflow was corrected to choose the larger app partition and export binaries. This is compile/link evidence only, not physical board, touch, SD, Wi-Fi, radio or safety validation.
- Host tests run `44779439310` passed and packaging run `44779439396` passed on firmware-source SHA `2289e84ad73e1931f18f357d0e437fed5e0d6c12`. Latest workflow-only commit does not change firmware source.

## Remaining safety/integration boundary
- Remote board is not yet physically available/verified. Key provisioning/control TX remains fail-closed.
- Boat target firmware is not compiled and has no command executor, telemetry publisher, actuator integration, or failsafe execution.
- Offline map and sonar are scaffolds/demo; online map provider is not implemented.
- Current ZIPs remain unverified snapshots; no VERSION bump or release package in this milestone.

## Next engineering work
1. Audit Wi-Fi/settings scan/connect timer cancellation and screen re-entry/object lifetime.
2. Audit Map Manager import/coverage parser bounds, malformed/duplicate records, zoom/tile coordinate validation, overflow-safe estimate, partial-copy cleanup and readback.
3. Audit map renderer PSRAM/cache allocation, RGB565 descriptor/stride, tile ownership, error handling and LVGL teardown/re-entry.
4. Fix verified issues, run host tests and target compile, update continuation/status files. Only then decide whether a package marker/release is warranted.
5. Physical bring-up, electrical validation, radio tests, and failsafe review remain separate required gates.
