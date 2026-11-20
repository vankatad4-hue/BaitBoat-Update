# Target build configuration decision record R1

**Branch:** `hardware-ready-dual-radio-r1`  
**Date:** 2026-11-20  
**State:** User-confirmed hardware selection; dependency/build manifest remains unfrozen pending version compatibility checks.

## Remote candidate target

**User confirmed:** We will use **Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948**. This is not the 4.3B or BOX variant. The unit has not yet been purchased or physically inspected, so its actual revision/markings must still be checked on receipt.

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

1. On receipt, verify the physical SKU/board revision and connector layout match the standard SKU 25948.
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

## User hardware selection confirmation — 2026-11-20
- User explicitly confirmed the standard Waveshare ESP32-S3-Touch-LCD-4.3 unit specified above.
- Hardware selection is no longer a design question. Purchase/physical verification remains outstanding.
- Build dependency pinning and compilation remain the next software gate; no changes to display architecture are needed.

## Dependency compatibility investigation — 2026-11-20

Official Waveshare documentation confirms standard SKU 25948 and the board capabilities. Its older Arduino instructions/demo list LVGL 8.x (the wiki notes LVGL 8.3/8.4) and legacy ESP32_Display_Panel / ESP_IO_Expander versions. The newer Waveshare example repository has an LVGL v9 path, including an ESP-IDF example using `waveshare/esp32_s3_touch_lcd_4` BSP 3.0.0 and LVGL 9.5.0. These paths are not interchangeable without checking source APIs and board config.

**Build decision:** Keep the project LVGL 9.6 requirement, but do not pin a dependency set yet. Next, inspect the Remote display adapter against the newer official LVGL9 BSP/API and determine whether LVGL 9.6.0 is compatible with the BSP's supported range. If not, document a deliberate version change rather than silently downgrading.

Official references:
- Board docs: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3
- Legacy Arduino wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3
- Official example repo: https://github.com/waveshareteam/ESP32-S3-Touch-LCD-4
- LVGL9 example README: https://github.com/waveshareteam/ESP32-S3-Touch-LCD-4/blob/main/examples/esp-idf/07_lvgl_demo_v9/README.md
- ESP32 Display Panel: https://github.com/esp-arduino-libs/ESP32_Display_Panel
