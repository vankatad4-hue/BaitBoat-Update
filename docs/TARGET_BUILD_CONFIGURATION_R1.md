# Target build configuration decision record R1

**Branch:** `hardware-ready-dual-radio-r1`  
**Date:** 2026-11-20  
**State:** Candidate configuration identified; build manifest intentionally not frozen.

## Remote candidate target

Current source targets **Waveshare ESP32-S3-Touch-LCD-4.3, standard (SKU 25948)**, not the newer 4.3B variant. This identification comes from the source comments and the earlier design selection. It must be reconfirmed before purchase/build.

Source adapter:
- `Remote/source/display_port.cpp`
- 800×480 RGB565
- ST7262 LCD driver class through `esp_display_panel.hpp`
- GT911 touch
- board IO expander for backlight
- TF/microSD interface
- LVGL 9.6.x; compile-time check currently requires major 9/minor 6

Official Waveshare docs list SKU 25948 for ESP32-S3-Touch-LCD-4.3 and describe 16MB Flash/8MB PSRAM, RGB display, GT911 touch and TF-card slot. Their example resources include Arduino and ESP-IDF. Their published demo dependency/API versions are not identical to this project's LVGL 9.6 requirement; the project's source adapter must be checked against a pinned compatible `ESP_Display_Panel` release and board configuration.

## Why the target manifest is not yet frozen

The current source imports `esp_display_panel.hpp`, uses `Board`, `LCD`, `Touch`, `drawBitmap`, PSRAM allocation and SD APIs. A target build needs a consistent version set of:
- Espressif Arduino-ESP32 core / ESP-IDF component baseline
- ESP_Display_Panel
- ESP_IO_Expander
- LVGL 9.6.x and project `lv_conf.h`
- RF24 / SPI radio dependencies
- SD filesystem and any map/crypto utilities

Choosing arbitrary latest versions is unsafe: Waveshare's board examples and library interfaces vary across releases and LVGL v8/v9 interfaces are not interchangeable. A manifest should be created only after verifying a tested compatible set against the official board example and this source.

## Candidate compile plan

1. Confirm SKU 25948 is the intended Remote display (standard, not 4.3B or BOX).
2. Retrieve official demo package and identify its exact ESP32_Display_Panel and ESP_IO_Expander versions and board config.
3. Select an ESP32 Arduino core supported by that dependency set; lock the version.
4. Integrate LVGL 9.6.0 and the project's configuration; compile a display-port smoke target first.
5. Compile all Remote modules and run portable tests in CI.
6. Compile Boat separately after exact ESP32-S3 board revision/pin audit; keep GPIO and actuator outputs disabled.
7. Commit build manifests, lock/version table and logs only when reproducible.

## Boat target

Boat controller remains a candidate Heltec Wireless Tracker ESP32-S3 with onboard GNSS/SX1262. Exact board revision and GPIO/resource conflicts are unresolved. Do not choose a generic ESP32-S3 PlatformIO board ID or enable radio/GPIO code until the pin audit confirms a matching physical revision.

## Known limitations

- This document is a decision record, not a build manifest.
- No target compile is claimed.
- No hardware validation is claimed.
- The board has not been purchased/identified by inspection.

## Official documentation references
- Waveshare board overview: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3
- Waveshare resources/examples: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3/Resources-And-Documents
- Waveshare usage guide: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3/Instructions-For-Use
- Waveshare FAQ: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3/FAQ
