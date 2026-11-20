# Flytec V080 Remote — development source


## Project archive versioning rule

- Every distributed project ZIP must use the filename format `Flytec_Remote_verXXX.zip`.
- The initial release in this naming scheme is `Flytec_Remote_ver001.zip`.
- Every modification to any project file that changes the project deliverable increments `XXX` by exactly 1 (for example: `ver001` → `ver002`).
- Never overwrite an earlier versioned ZIP. Keep each release as a separate archive so previous versions remain recoverable.
- The version number in the ZIP filename is the authoritative project release number; update this rule's examples or release notes only when needed, not the naming format.

## LVGL API baseline

**Target: LVGL 9.6.0 (9.6.x compatible patch releases).** The project must not be built with LVGL 8.x, 9.0.x, or another minor release. `display_port.cpp` contains a compile-time major/minor version guard.

Use the current official documentation: https://lvgl.io/docs/open/

Relevant API references:
- Display API: https://lvgl.io/docs/open/api/display/lv_display_h
- Input-device API: https://lvgl.io/docs/open/api/public/indev/lv_indev_h
- Timer API: https://lvgl.io/docs/open/api/public/core/lv_timer_h
- Display setup: https://lvgl.io/docs/open/main-modules/display/setup
- Integration overview: https://lvgl.io/docs/open/integration/overview

The project uses the LVGL 9 object model (`lv_display_*`, `lv_indev_*`, `lv_screen_active()`, `lv_screen_load()`, `lv_button_create()`). The version check prevents accidentally compiling against the older LVGL 9.0 baseline used in earlier drafts.

## Arduino IDE folder

Open `Flytec_Remote.ino` from this folder. `Boat_menu_update.ino` was removed because it contained a second empty `setup()` and `loop()`.

## Dual-radio architecture R2 (2026-11-20)

- **Primary:** nRF24L01+ PA/LNA, matching modules on remote and boat, 2.4GHz, 250kbps initial setting.
- **Backup remote:** EBYTE E22-900T22S UART module candidate, EU868.
- **Backup boat:** Heltec Wireless Tracker candidate's integrated SX1262, if exact revision/band confirmed; no extra E22 module by default.
- Remote RIGHT antenna is LoRa 868MHz; LEFT antenna is nRF24 2.4GHz. Waveshare ESP32-S3 Wi-Fi/Bluetooth uses its onboard antenna.
- LoRa PHY must be configured identically on remote E22 and boat's native SX1262; interoperability is not assumed. Backup is safety/low-rate traffic, no sonar, no bait/hook release or mission start.
- Dual-link state machine, authenticated/replay-protected packets, duplicate suppression, failover hysteresis, command ACK/state confirmation and boat-local failsafe are specified but not implemented.
- 500m remains an unvalidated target. Regulatory power/channel/duty-cycle and antenna gain/cable-loss compliance must be verified for Bulgaria/EU.
- See `../docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R2.md`, `../docs/DUAL_RADIO_PROTOCOL_FIRMWARE_SPEC_R2.md`, and `../New_Handheld_Remote/PCB/REMOTE_HARDWARE_PIN_ASSIGNMENTS_R2.md`.

## Current modules

- `Flytec_Remote.ino`: application entry point; one joystick callback and one initialisation sequence.
- `boat_state.h/.cpp`: shared boat state; starts STOP-latched and unconfirmed.
- `boat_commands.h/.cpp`: application command objects and transport callback.
- `boat_radio.h/.cpp`: command routing and radio state API integrated with dual-link supervisor; transport remains disabled without drivers.
- `radio_protocol.h/.cpp`: fixed 32-byte frame, CRC16 and HMAC-SHA256 authentication tag.
- `radio_command_codec.h/.cpp`: command encode/decode, freshness and backup-policy checks.
- `dual_radio_manager.h/.cpp`: health, failover, recovery hysteresis and restricted LoRa command policy.
- `boat_ui.h/.cpp`: main map/sonar placeholder screen and STOP/STICKS controls.
- `joystick.h/.cpp`: four-axis normalisation, calibration and safety gates.
- `joystick_calibration_ui.h/.cpp`: LVGL 9.6 calibration/settings screen.
- `boat_telemetry.h/.cpp`: telemetry structure and GPS validity/freshness handling; no radio packet decoder is connected yet.
- `settings_ui.h/.cpp`: Wi-Fi, Display and Sonar preferences, category placeholders and LVGL virtual keyboard.
- `offline_map.h/.cpp`: map manifest validation and raw RGB565 tile reads from SD into PSRAM.
- `map_projection.h/.cpp`: WGS84/Web-Mercator projection and heading-based viewport offset math.
- `display_port.h/.cpp`: Waveshare board initialisation, LVGL 9.6 display/touch adapter and optional TF-card mount; first-pass integration, pending an actual board build and test.

## Safety state

The remote starts with `emergencyStop=true` and `emergencyStopConfirmed=false`. A future validated telemetry handler must set the confirmation flag only after receiving an authenticated/valid boat status packet. No radio hardware driver is registered. Link state remains disconnected and command sends return false. The joystick ADC reader returns false. Motor commands are additionally inhibited until a complete four-axis calibration succeeds.

Calibration values currently exist only in RAM; persistent Preferences/NVS storage has not yet been implemented.

## Display integration boundary

`display_port.cpp` now contains a concrete board adapter based on the uploaded Waveshare Arduino demo. It initialises `Board`, obtains the LCD and touch objects, then registers an LVGL 9 display and pointer input device. `Flytec_Remote.ino` creates the UI only after this initialisation succeeds.

For the selected Waveshare ESP32-S3-Touch-LCD-4.3 (SKU 25948), the board uses an ST7262 RGB display, GT911 capacitive touch controller and CH422G I/O expander. Its official Arduino example is the source of truth for the hardware initialisation sequence. Keep that implementation inside `display_port.cpp` (or a board-specific file called by it), not inside the boat UI modules.

The adapter must establish one owner for each operation: `lv_init()`, display creation/flush callback, touch input registration, LVGL tick source, and LVGL timer-handler/task scheduling. If the official example runs the LVGL handler in a FreeRTOS task, `displayPort_loop()` must not run another handler. If the example expects loop-based servicing, put that exact servicing call in `displayPort_loop()`.

LVGL 9.6's generic port requires a display with a flush callback and draw buffer(s); touch is registered as an input device with a read callback. The LVGL 9.6 API uses `lv_display_*` and `lv_indev_*` interfaces. The board example supplies the hardware-specific implementation and configuration. See the current LVGL display and input-device API references in the links below.

### Waveshare board setup notes (SKU 25948 — standard 4.3-inch board)

**Confirmed board resources**

- Display: 800 × 480 ST7262 RGB panel.
- Touch: GT911 capacitive controller.
- I/O expander: CH422G, on the same I²C bus as the touch controller.
- LCD RGB/timing GPIOs: GPIO 5 (DE), 3 (VSYNC), 46 (HSYNC), 7 (PCLK), 1/2/42/41/40 (R0–R4), 39/0/45/48/47/21 (G0–G5), and 14/38/18/17/10 (B0–B4).
- Touch: GPIO 4 (IRQ), GPIO 8 (SDA), GPIO 9 (SCL); CH422G EXIO1 controls touch reset.
- CH422G EXIO2 controls display backlight; EXIO4 is TF-card chip select; EXIO5 selects the USB/CAN path.
- GPIO 8/9 are shared I²C for the touch controller, CH422G and external I²C header. Avoid adding devices at conflicting addresses.
- GPIO 15/16 are multiplexed for the onboard RS485/CAN functions. GPIO 19/20 are native USB D-/D+.

Do not allocate any of the above pins to joystick inputs, buttons or radio functions. This list is a board-resource reference, not a new pin assignment for the remote's external controls.

**LVGL version compatibility**

- Waveshare's official Arduino guide for this exact board lists `ESP32_Display_Panel` v0.1.4 or later, `ESP32_IO_Expander` v0.0.4 or later, and LVGL 8.4.0. The documented `08_lvgl_Porting` example is LVGL 8.x.
- The project target is LVGL 9.6.0 (9.6.x compatible patch releases). We must therefore reuse only verified board-level panel/expander/touch setup from the official example and implement the LVGL 9.6 display and input registration separately. Do not copy LVGL 8 calls or its porting files into this project.
- The Waveshare FAQ calls out 16 MB Flash and 8 MB OPI PSRAM settings. Confirm the exact Arduino-ESP32 core and board menu settings against the downloaded demo before compiling.

**Battery connector warning**

- The board's PH2.0 battery connector is for a single-cell 3.7 V Li-ion battery. Waveshare recommends a capacity below 2000 mAh. The proposed 2 × 18650, 1200 mAh cells in parallel form a 1S2P pack of approximately 2400 mAh, so this is above the stated recommendation. Verify the onboard charger/protection design before using that pack; do not connect a second charger in parallel.

**Source needed for the next integration step**

The uploaded `ESP32-S3-Touch-LCD-4.3-Demo(1).zip` was inspected. Its `Arduino/examples/09_lvgl_Porting/09_lvgl_Porting.ino` uses `Board`, `board->init()`, `board->begin()`, `board->getLCD()` and `board->getTouch()`. The matching official `esp_panel_board_custom_conf.h` is included in this project folder so the library can detect and configure the standard Waveshare 4.3-inch board from the project directory (which takes priority over the library defaults). Its accompanying README explicitly targets LVGL 8.3.x, and its `lvgl_v8_port.cpp/.h` use removed LVGL 8 APIs. Those LVGL 8 port files are therefore NOT copied into this LVGL 9.6 project. Instead, `display_port.cpp` uses the board-level API from the uploaded example and the LVGL 9.6 `lv_display_*` / `lv_indev_*` APIs. The LVGL 9.6 draw buffer is a 40-row RGB565 partial buffer allocated in PSRAM. Touch reads the first GT911 point. The LVGL handler is serviced from `displayPort_loop()` every ~5 ms; no second FreeRTOS LVGL task is started.

## Not ready for boat operation

This is a source consolidation and API-alignment draft, not a verified firmware release. The C++ source files passed a syntax-only check against lightweight Arduino/LVGL interface stubs; this is not a substitute for compiling against the actual LVGL 9.6.0 library and Waveshare board package. GPIO assignments, joystick electrical levels, radio pins/protocol, telemetry authentication/acknowledgements, dual-link failover, boat-side watchdog and motor/solenoid interlocks remain unimplemented. Do not connect it to motor drivers or operate the boat.


### Display adapter implementation notes

- The adapter assumes the standard board reports 800 x 480 and RGB565, and checks the dimensions before LVGL registration.
- It allocates one 40-line partial buffer in PSRAM (64,000 bytes) and sends each rendered area through the board LCD driver's `drawBitmap()`.
- The GT911 callback reads one point with `readPoints(&point, 1, 0)`. Multi-touch gestures are not yet used.
- `lv_tick_set_cb(millis)` supplies LVGL time; `lv_timer_handler()` is called only by `displayPort_loop()`. Do not add a second LVGL task or timer handler.
- The flush callback assumes the RGB panel driver's `drawBitmap()` has completed the copy when it returns. This is based on the RGB-board path in the vendor port; verify on the exact installed `ESP32_Display_Panel` library.
- This source is a first integration pass, not a verified full build or hardware test. Install the board libraries from the uploaded demo package (including `ESP32_Display_Panel`, `ESP32_IO_Expander` and `esp-lib-utils`) and install LVGL 9.6.0 separately. Do not install the demo package's LVGL 8 library as the project's LVGL dependency. The uploaded demo package's LVGL port itself is v8.3, so it cannot validate the LVGL 9.6 port.


## LVGL 9.6.0 migration status

- All application UI files use the LVGL 9 object model; no LVGL 8 port source is included.
- `display_port.cpp` now requires LVGL major 9, minor 6 at compile time, preventing accidental builds with 9.0.x or 8.x.
- Display setup uses `lv_display_create`, `lv_display_set_color_format`, `lv_display_set_flush_cb`, and `lv_display_set_buffers` with partial RGB565 rendering.
- Touch setup uses `lv_indev_create`, `lv_indev_set_type`, and `lv_indev_set_read_cb`.
- `lv_tick_set_cb(millis)` supplies the tick and the Arduino loop owns the single `lv_timer_handler()` call.
- The public APIs above are present in the current LVGL documentation. This source has not yet been compiled against the actual LVGL 9.6.0 library and the user's installed ESP32 board package; the compile-time guard is a version selector, not a substitute for that build.


## LVGL 9.6.0 + Arduino IDE installation notes

### Important: the demo ZIP contains LVGL 8.4.0

The uploaded Waveshare demo archive contains these library versions:

| Library | Version in the uploaded archive |
|---|---:|
| `ESP32_Display_Panel` | 1.0.0 |
| `ESP32_IO_Expander` | 1.0.1 |
| `esp-lib-utils` | 0.1.2 |
| `lvgl` | **8.4.0** |

Install the three Waveshare/Espressif board-support libraries from the demo archive, but **do not install or use the archive's `lvgl` folder**. It is LVGL 8.4.0 and this project deliberately requires LVGL 9.6.x. Install LVGL 9.6.0 separately and verify the selected library's `lvgl.h` reports:

```cpp
#define LVGL_VERSION_MAJOR 9
#define LVGL_VERSION_MINOR 6
#define LVGL_VERSION_PATCH 0
```

The project has a compile-time guard in `display_port.cpp`; an LVGL 8.4.0 installation should fail that check intentionally.

### Arduino IDE board settings

For the Waveshare ESP32-S3-Touch-LCD-4.3 standard board (SKU 25948):

1. Install Arduino-ESP32 core **3.0.2 or newer**. Waveshare's FAQ identifies 3.0.2 as the minimum for the `esp_memory_utils.h` dependency.
2. Select the board profile specified by the Waveshare guide. If using `ESP32S3 Dev Module`, match the settings to the board guide rather than guessing.
3. Set Flash Size to **16 MB**.
4. Enable the board's **8 MB OPI PSRAM** setting.
5. Use the USB/serial port associated with the board. If upload does not start, use the board's BOOT/download procedure from the Waveshare guide.
6. Keep `esp_panel_board_custom_conf.h` beside `Flytec_Remote.ino` in the sketch folder. It configures the standard 4.3-inch board resources.

Waveshare's FAQ specifically calls out 16 MB Flash and 8 MB OPI PSRAM. It also recommends Arduino-ESP32 3.0.2 or newer for a missing `esp_memory_utils.h` build error.

### First compile / first display test

Do this before connecting any joystick, radio, motor driver or solenoid:

1. Install the required board-support libraries and LVGL 9.6.0 as described above.
2. Open `Flytec_Remote.ino` from this folder in Arduino IDE.
3. Compile without uploading first.
4. Confirm the compiler's version guard accepts LVGL 9.6.x.
5. Upload with the board disconnected from all external control wiring.
6. Open Serial Monitor at 115200 baud.
7. Confirm the log reports the board, LCD and touch initialisation as successful.
8. Verify that the main UI appears and that the touchscreen responds to taps.

### What this adapter currently does — and does not do

- Uses Waveshare's `Board`, `board->init()`, `board->begin()`, `getLCD()` and `getTouch()` hardware setup.
- Registers the LVGL 9 display using `lv_display_create()`, an RGB565 partial draw buffer, and a flush callback.
- Registers the GT911 as an LVGL pointer input device.
- Services LVGL from the Arduino `loop()`; it does not create a second LVGL task.
- Uses a 40-line RGB565 partial buffer (64,000 bytes) allocated in PSRAM.

**Display-port caveat:** Waveshare's supplied 4.3-inch example is an LVGL 8.3.x port and uses its own anti-tearing/task implementation. That LVGL 8 port is not compatible with this LVGL 9.6 project and is intentionally not reused. The custom LVGL 9.6 adapter here is a first integration path; it has not yet been compiled on the user's board or measured for tearing, refresh rate, or touch-coordinate orientation. If the first live test shows tearing or sluggish refresh, the next task is to adapt the RGB frame-buffer/anti-tearing strategy for LVGL 9.6 rather than importing the LVGL 8 port.

### Safety status

The remote is still a non-operational development build:
- nRF24L01+ PA/LNA radio transport is a placeholder and always reports disconnected. The selected starting data rate is 250 kbps; pins, channel and packet protocol are not yet assigned.
- Joystick ADC pin assignments are intentionally absent; the hardware reader returns `false`.
- Joystick calibration and the radio/STOP acknowledgement path are not connected to a boat.
- No boat-side watchdog, authenticated telemetry, motor control, or solenoid interlocks are implemented.

Do not connect this code to the boat's motor or solenoid power stages. The first physical test is display + touch only.


## GPS and map development status (v2 baseline update)

The remote-side GPS display and telemetry model have been extended. `BoatTelemetry` includes fix type, satellite count and horizontal accuracy; telemetry receipt is timestamped; GPS is considered usable only with a 3D fix, valid coordinate ranges and telemetry no older than 2 seconds. The main map panel now displays coordinates and GPS quality and shows a boat marker only for a fresh fix.

**The map is not yet a real geographic map.** The current viewport is a diagnostic grid, explicitly labelled as missing offline map data. The next step is to mount the Waveshare microSD interface and add an Arduino-compatible offline RGB565 tile loader. The nRF24L01+ PA/LNA transport is still a placeholder, so live GPS data is not yet received by this remote application. See `docs/GPS_AND_OFFLINE_MAP_PLAN.md`.


## Offline map storage update

- Added TF-card mounting through the existing board-owned CH422G expander; physical SD CS is CH422G EXIO4.
- Uses GPIO6 only as the Arduino SD library's dummy CS argument; SPI data pins are MOSI 11, SCK 12 and MISO 13.
- SD failure is non-fatal: display/touch can still start without a card.
- Added an offline tile-pack validator and raw RGB565 tile reader using PSRAM.
- Expected map pack layout and tile format are documented in `docs/OFFLINE_MAP_STORAGE.md`.
- This is storage and tile-loading infrastructure only. The current screen still shows a diagnostic grid; there is no geographic map renderer or bundled map imagery yet.

### Map projection status

Added `map_projection.h/.cpp` with WGS84 ↔ Web-Mercator world-pixel conversion and compass-heading viewport-offset rotation. This is the renderer's geographic foundation only. The LVGL map panel still displays its diagnostic grid until the tile cache, RGB565 viewport drawing, and map controls are implemented and tested.

## Remote Settings screen (first implementation)

A SETTINGS button is available in the main header. The new `settings_ui.h/.cpp`
module provides a separate 800x480 settings screen with Wi-Fi, Display, Controls,
Map and System categories. Wi-Fi includes network scanning, SSID/password fields,
a built-in LVGL virtual keyboard, NVS profile storage and a connect action.
Display preferences are stored in ESP32 NVS. The present Waveshare board
configuration selects CH422G expander backlight switching, so the implemented
backlight control is ON/OFF only; it is not a variable-brightness slider. True
brightness adjustment requires switching to a verified PWM-capable board
backlight configuration and confirming the actual hardware pin/driver support.
The screen-timeout and night-theme choices are currently saved preferences, not
yet active dimming/theme engines. Controls, Map and System categories currently
show integration placeholders. Wi-Fi password is stored in NVS; enable ESP32
flash encryption in a production deployment if credential-at-rest protection is
required. Wi-Fi scanning/connection has not been tested on physical hardware.


## Sonar settings UI update (2026-11-16)

The remote Settings screen now includes a Sonar category with locally persisted profile values in ESP32 NVS: depth range, gain, fish-detection sensitivity, noise rejection, ping-rate preference, echogram scroll speed, colour palette, fish arches/symbols, fish alarm, shallow/deep alarms, and alarm depth thresholds. The settings content panel is scrollable.

Important: these are remote-side saved preferences only. The current boat radio is still a placeholder and no compatible CHIRP sonar module/control protocol has been integrated. Therefore the values are not yet applied to sonar hardware. The final available controls and valid ranges must be matched to the selected sonar module's documented API.

---

## Master project record and latest review

The detailed, consolidated record of the whole Flytec V080 retrofit—including retained boat hardware, battery and actuator information, PCB envelope, Waveshare remote hardware, navigation requirements, UI decisions, sonar requirements, map architecture, settings, safety constraints, outstanding work and validation status—is in [`docs/PROJECT_MASTER_STATUS.md`](docs/PROJECT_MASTER_STATUS.md). Read this first when continuing the project in a future session.

### Latest code-review optimisations (2026-11-16)

- Wi-Fi scan and connection status now use LVGL timers instead of blocking the UI event loop.
- Sonar slider settings are saved on release rather than every movement event to reduce repeated NVS writes.
- The Settings screen is reused when reopened, keeping the virtual keyboard attached to the correct screen and avoiding repeated screen allocation.
- The stored screen-timeout value is reflected in its label; the feature remains a saved preference only, not an active dimmer.
- Map projection rejects unsupported zoom values above 22; offline tile loading rejects unknown style values.

### Current functional boundary

The current archive remains a remote-side development baseline, not boat-ready firmware. The radio transport, joystick ADC hardware reader, live GPS packet reception, boat-side GPS/compass firmware, CHIRP sonar hardware interface, real geographic LVGL map renderer, waypoint persistence, and boat-side autopilot/failsafe remain unimplemented. Sonar preferences are local only. The visible map is still a diagnostic grid. Backlight supports ON/OFF only. Consult the master status document before assuming any planned feature is operational.


## Latest hardware architecture update — ver003

Boat-side controller/GNSS selection is now Heltec Wireless Tracker V1.2 (ESP32-S3FN8 + UC6580 GNSS), replacing the generic ESP32 carrier and separate HGLRC M100 PRO B101 GNSS. Heading/motion sensing is consolidated into a GY-91-style MPU-9250 + BMP280 module. The external nRF24L01+ PA/LNA radio decision is unchanged; the Tracker SX1262 is not selected for the command link. This is documentation only: no boat-side sensor or radio drivers are implemented. See `docs/HARDWARE_SELECTION_VER003.md` and `docs/PROJECT_MASTER_STATUS.md`.


## Hardware modularity update — ver004

The remote uses a central carrier/interconnect PCB with removable cables for the nRF24L01+ PA/LNA, joystick modules, physical buttons and power module. The Waveshare ESP32-S3-Touch-LCD-4.3 remains the main display/controller. See `docs/CARRIER_PCB_CONNECTION_PLAN_VER004.md`. Connector pin orders are logical only; no new GPIO assignments are approved.

This release adds documentation only. Radio and joystick inputs remain non-operational placeholders until hardware integration and testing.


## Dual-link command-safety scaffold update (2026-11-20)
- `boat_radio.h/.cpp` now expose primary/backup connection status, active-link and link-state enums. They deliberately return disconnected values until drivers/LinkManager are implemented.
- `BoatCommand` now carries `sequence`, stable `commandId`, and `createdAtMs`; current sender stamps command ID equal to sequence and time. This is only an application scaffold, not wire-protocol authentication or boat-side duplicate suppression.
- Do not connect to actuators. Release-command idempotency, backup-mode restrictions and authenticated acknowledgements remain implementation and test gates.


## Hardware design revision R2
- Added pin assignment proposal: `New_Handheld_Remote/PCB/REMOTE_HARDWARE_PIN_ASSIGNMENTS_R2.md`; nRF CSN direct GPIO6 candidate, CE through MCP23017 only for bench timing validation; LoRa E22 UART via SC16IS750 I2C-UART bridge @0x48 to avoid GPIO15/16 RS485 conflict. ADC addresses changed to 0x4A/0x4B. Exact physical header crosswalk remains pending.
- Added detailed schematic sheet spec, pin connections CSV, test build and BOM R2. Native KiCad capture/ERC/PCB still not completed.
- Revised remote envelope target 300×210×70mm; physical fit check is required.
- LoRa airtime/profile and Bulgarian/EU legality must be checked; initial backup command cadence is capped at 1Hz in software policy.
