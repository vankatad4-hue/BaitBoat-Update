# Flytec V080 modular bait-boat retrofit

This repository contains the Boat controller, handheld Remote, firmware, hardware decisions, tests, and commissioning procedures for the Flytec V080 retrofit.

## Project guide

- [Boat firmware and hardware](Boat/README.md)
- [Remote firmware and hardware](Remote/README.md)
- [Project revision and updates](PROJECT_REVISION_AND_UPDATES.md)
- [Active continuation checkpoint](docs/CONTINUATION_CHECKPOINT_VER001.md)
- [Project progress review](docs/PROJECT_PROGRESS_REVIEW_R1.md)

## Development setup

VS Code with PlatformIO is the primary build and CI path for both endpoints. See [the PlatformIO workflow](docs/PLATFORMIO_PRIMARY_WORKFLOW_R30H10.md) and [workspace file](Flytec_V080.code-workspace).

```powershell
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

These are compile-only targets. A successful build or host simulation does not establish physical commissioning or hardware acceptance.

## System overview

- **Boat:** ESP32-S3 controller, GNSS/compass, nRF24 primary radio, E22 backup radio, motor driver, and release controller.
- **Remote:** Waveshare ESP32-S3 touchscreen controller with LVGL, primary and backup radio links, joysticks, and SD-backed offline maps.
- **Official firmware markers:** Boat VER014 and Remote VER028.

**Sonar choice — 2026-11-27:** Owner-selected preferred candidate: Open Echo TUSS4470 + Lucky D+T wired 200 kHz / 45° transducer, including live water temperature measurement. Electrical compatibility, probe pinout, temperature sensor characteristics, shallow-water performance and CHIRP capability remain unverified. Implementation and procurement remain deferred; no firmware, wiring or other hardware decisions changed.

## Operational status

Firmware upload and physical RF/hardware commissioning remain controlled by unsigned project gates. Do not treat compilation, simulated tests, software acknowledgements, or GPIO register readback as proof of physical operation. Keep actuators disconnected unless the applicable approved commissioning procedure authorizes a specific test.

For current decisions, test evidence, open work, and revision history, use the linked project documents above.
