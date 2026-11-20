# Remote Target Build Configuration R1

Date: 2026-11-20  
Target: **Confirmed Waveshare ESP32-S3-Touch-LCD-4.3 Standard (SKU 25948)**

## Hardware target

- ESP32-S3-WROOM-1-N16R8
- Flash: 16 MB
- PSRAM: 8 MB OPI
- LCD: ST7262 RGB, 800 × 480, RGB565
- Touch: GT911
- Board I/O expander: CH422G
- TF/microSD: onboard interface; CS via CH422G EXIO4, dummy Arduino CS GPIO6, SPI SCK12/MISO13/MOSI11
- Board variants 4.3B and BOX are not targets.

## Candidate software pins

These pins are recorded for reproducibility and must be validated from the CI compile and resolved library output:

| Dependency | Version |
|---|---:|
| Arduino-ESP32 core | 3.0.2 |
| LVGL | 9.6.0 |
| ESP32_Display_Panel | 1.0.0 |
| ESP32_IO_Expander | 1.0.1 |
| esp-lib-utils | 0.1.2 |

The supplied vendor demo bundle includes an LVGL 8.x port. Its LVGL-port files are intentionally excluded; only the board-level panel, touch and expander interfaces are used by the custom LVGL 9.6 adapter.

## Build command candidate

The GitHub Actions workflow `.github/workflows/remote-target-build.yml` uses Arduino CLI and the generic ESP32-S3 FQBN with 16MB Flash and OPI PSRAM options:

```sh
arduino-cli compile --verbose --fqbn 'esp32:esp32:esp32s3:FlashSize=16M,PSRAM=opi' Remote/source
```

This is an initial reproducible-build attempt, not a frozen validated toolchain. Arduino library-manager availability, dependency versions, FQBN option syntax and compile success remain pending. If it fails, use the build log to correct the manifest/workflow and record the actually resolved versions.

## Gate before claiming complete Remote compile

- [ ] Target workflow completes successfully.
- [ ] Resolved compiler/core and all library versions recorded.
- [ ] Full sketch and every local source module compile/link.
- [ ] Warnings reviewed (no ignored warnings without a documented reason).
- [ ] Build log retained as workflow artifact.
- [ ] Display adapter compile verified against the actual panel, touch and LVGL signatures.

## Hardware smoke test after target compile

Board not purchased/available at the time of this checkpoint. Keep all boat actuators and radio modules disconnected. Once available, test:
1. LCD initialisation, colour format and UI rendering.
2. Touch read, orientation, calibration and edge mapping.
3. Backlight on/off and reset behaviour.
4. TF mount, file read and SD activity during display refresh.
5. UI loop responsiveness under Wi-Fi/SD activity.
6. Refresh rate, tearing and partial-buffer stability.
