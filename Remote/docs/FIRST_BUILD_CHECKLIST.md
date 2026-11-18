# Flytec remote — first Arduino IDE build checklist

## Exact target

- Board: Waveshare ESP32-S3-Touch-LCD-4.3 standard, SKU 25948 (not the 4.3B variant).
- MCU module: ESP32-S3-WROOM-1-N16R8.
- Display: 800 × 480 RGB565, ST7262.
- Touch: GT911.
- LVGL: 9.6.0 (project compile guard accepts 9.6.x).

## Library versions present in the uploaded Waveshare demo ZIP

- ESP32_Display_Panel 1.0.0
- ESP32_IO_Expander 1.0.1
- esp-lib-utils 0.1.2
- LVGL 8.4.0 — DO NOT install this bundled LVGL folder for this project.

Install the three board-support libraries above from the vendor archive. Install LVGL 9.6.0 separately. Confirm `lvgl.h` reports major 9, minor 6, patch 0.

## Arduino IDE settings

1. Arduino-ESP32 core 3.0.2 or newer.
2. Select the Waveshare board profile if installed; otherwise follow the board guide for the ESP32S3 Dev Module profile.
3. Flash Size: 16 MB.
4. PSRAM: enabled, 8 MB OPI.
5. Keep `esp_panel_board_custom_conf.h` in the same sketch directory as `Flytec_Remote.ino`.

## Compile / upload sequence

1. Open the `Flytec_Remote.ino` sketch from the `Boat_menu_update` folder.
2. Compile with LVGL 9.6.0 selected. A failure at the LVGL version guard usually means Arduino IDE selected the bundled LVGL 8.4.0 library or another copy earlier in its library search path.
3. Do not connect any external joystick, radio, motor or solenoid wiring.
4. Upload to the Waveshare display board.
5. Open Serial Monitor at 115200 baud.
6. Check for `DISPLAY: Waveshare LCD, GT911 and LVGL 9.6.x ready.`
7. Confirm that the screen renders the UI and that touch events are detected.

## What to report if the build fails

Copy the first compiler error and about 20 lines around it, plus:
- Arduino-ESP32 core version
- ESP32_Display_Panel version
- ESP32_IO_Expander version
- esp-lib-utils version
- LVGL version shown in `lvgl.h`
- selected board profile and PSRAM setting

## Current limitations

This source is not yet a verified hardware build. The Waveshare vendor example in the supplied archive is LVGL 8.3.x; its LVGL port cannot be copied into the LVGL 9.6 project. The current `display_port.cpp` is a separate LVGL 9 display/touch adapter and still requires a physical compile and display/touch test. Screen tearing, refresh rate and touch orientation have not been validated.

The radio and joystick hardware remain deliberately disabled. Do not connect this development firmware to the boat's power stages.
