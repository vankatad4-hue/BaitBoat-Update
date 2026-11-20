# LVGL 9.6 Display Adapter Compatibility Audit — R1

Date: 2026-11-20  
Target confirmed by user: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948  
Reviewed source: `Remote/source/display_port.cpp`, `display_port.h`, `esp_panel_board_custom_conf.h`

## Scope and result

This is a source-level audit of the current display adapter. The target is now confirmed, so variant-selection uncertainty is closed. **The adapter is not yet verified against the actual dependency headers or a target compiler.** A GitHub Actions target-compile workflow was added to provide that next evidence step.

## API and lifecycle review

- LVGL version guard requires major 9, minor 6. This correctly rejects the vendor demo's LVGL 8 port.
- `lv_init()`, `lv_tick_set_cb()`, `lv_display_create()`, `lv_display_set_color_format()`, `lv_display_set_flush_cb()`, `lv_display_set_buffers()`, `lv_indev_create()`, `lv_indev_set_type()` and `lv_indev_set_read_cb()` are LVGL 9-style APIs; this audit did not verify signatures against the installed 9.6.0 headers.
- The flush callback has a defensive invalid-input path. A potential robustness improvement is to avoid calling `lv_display_flush_ready()` if the display pointer itself is null; LVGL should normally supply a valid display, but guard it explicitly.
- Single-buffer partial rendering (40 rows, RGB565, 64,000 bytes) is an allowed configuration in principle; actual performance and copy/flush semantics remain unverified.
- The adapter assumes `LCD::drawBitmap()` completes the copy synchronously and returns a boolean. Confirm the exact return type and transfer completion semantics for ESP32_Display_Panel 1.0.0. If asynchronous, the flush-ready call must be deferred until transfer completion.
- `Touch::readPoints()` and `TouchPoint` signatures are assumed from the vendor demo. Confirm against the pinned board-library headers. Touch orientation/coordinate transform and calibration are not implemented.
- LVGL service runs from Arduino `loop()` at a nominal 5 ms cadence. This establishes one owner and avoids a duplicate task, but worst-case blocking calls (Wi-Fi, SD/map operations) may starve refresh/input.
- Board startup is `Board::init()` then `Board::begin()`; exact return types and whether touch/LCD are enabled by the custom board config must be confirmed by compilation.
- Board config names ST7262/RGB, 800x480 and GT911 are present. It must be kept alongside the sketch in the Arduino compilation unit/library search context.
- SD initialization reuses the board CH422G expander, configures SPI GPIO12/13/11 and dummy CS GPIO6, with physical CS on EXIO4. This wiring is board-specific and not validated electrically.
- Backlight output uses CH422G EXIO2 on/off. No brightness PWM.
- Failure paths do not fully unwind allocated board/LVGL/PSRAM objects. Acceptable for a fail-closed startup that does not retry, but retry/recovery should add cleanup.

## Build configuration created

`Remote/docs/TARGET_BUILD_CONFIGURATION_R1.md` records the target and pinned candidate versions. A new workflow `.github/workflows/remote-target-build.yml` attempts a full Arduino CLI compile using:
- ESP32 Arduino core 3.0.2
- LVGL 9.6.0
- ESP32_Display_Panel 1.0.0
- ESP32_IO_Expander 1.0.1
- esp-lib-utils 0.1.2
- Generic ESP32-S3 target with 16 MB flash and OPI PSRAM

These are candidate pins based on the project build checklist/demo inspection; the workflow may reveal package-manager availability, dependency-resolution, board-menu option, or source API mismatches. **No target compile result is claimed yet.** The exact dependency set will only be frozen after a successful build log and dependency inventory.

## Required next actions

1. Obtain the target workflow result and preserve its logs/artifact.
2. Fix the first actual compiler/dependency error, not speculative API changes.
3. Confirm the resolved dependency versions and ESP32-S3 board option names from the build output.
4. Re-run until the complete Remote sketch compiles with warnings/errors reviewed.
5. Only after compile: board smoke test for startup, panel refresh, touch coordinates, SD mount/read, backlight and refresh/tearing.
6. Keep propulsion, release actuators and radios disconnected for display bring-up.

## Status

Source audit: complete.  
Target confirmed: yes.  
Build workflow committed: yes.  
Full target compile: pending.  
Physical board validation: not performed.

## CI attempts and corrective actions

- Attempt 1 failed before compiling source: Arduino CLI requires the sketch directory name and primary `.ino` basename to match. Workflow now stages a temporary folder named `FlytecRemoteBuild` and renames the entry sketch accordingly.
- The same workflow now captures compiler output into an artifact log.
- Host CI first failed because the unit test used a reduced `BoatState` stub layout that differed from the full struct compiled into the guard module (ODR/layout mismatch). The test now links the actual `boat_state.cpp` and full header.
- The next host run exposed a bad invalid-latitude vector: the test encoded 50.0000001°, which is valid, rather than 91°. Corrected vector encodes 910,000,000 E7.

- Remote host-test CI run #7 completed **successfully** after fixing the test layout and latitude test vector; telemetry formatting warnings found under `-Werror` were also corrected. Host-only coverage now passes; this does not validate the ESP32 target.
- Target compile run #3 successfully installed the pinned toolchain/libraries and staged the sketch, but failed because LVGL requires an explicit `lv_conf.h`. Added a versioned LVGL 9.6.0 configuration file and copy step for the next target compile attempt.

- Target compile attempt #4 now reached C++ preprocessing and identified the missing external dependency `RF24.h`; added pinned RF24 1.4.11 to the build workflow. This is a dependency-manifest discovery, not a display-adapter API error yet. Retry pending.

- Added a null-display guard in the flush callback during source review; malformed callback inputs now only signal flush-ready when a valid display handle exists.

## First full-source compiler findings (LVGL 9.6)

- The first full target compile reached `boat_ui.cpp` and exposed an LVGL 9.6 API change: `lv_line_set_points()` expects `lv_point_precise_t`, not `lv_point_t`. The DEMO echogram line-point arrays and helper signature were converted to precise-point coordinates.
- The same compile identified two missing global footer-label declarations (`inputStatusLabel`, `safetyStatusLabel`) even though the labels were created and refreshed in the UI code. Their declarations have been added.
- These source fixes are committed; compile retry pending. This is evidence from the target compiler, not yet a successful full build.
