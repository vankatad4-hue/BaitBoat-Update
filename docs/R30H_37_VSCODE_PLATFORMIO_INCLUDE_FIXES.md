# R30H-37 — VS Code + PlatformIO include and excluded-code repair

Use the **repository root** `Flytec_V080.code-workspace` with **File → Open Workspace from File**, not a standalone `.ino` opened outside the project. This workspace contains two independent PlatformIO projects, `Boat/` and `Remote/`.

## Fix red squiggles / `#include errors detected`
1. Install VS Code **PlatformIO IDE** and **C/C++ (Microsoft)** extensions. Open the workspace, trust the folder and allow PlatformIO to install its packages.
2. From the Command Palette run **PlatformIO: Rebuild IntelliSense Index** for each project. Or run the workspace tasks **Flytec: Boat refresh IntelliSense compile database** and **Flytec: Remote refresh IntelliSense compile database**. They run `pio run -t compiledb` inside the correct project.
3. Run `pio run -e boat_n32r16v_diagnostic` from `Boat/`, and `pio run -e remote_waveshare_43_diagnostic` from `Remote/`. The first dependency installation can take time. **Do not flash**: both targets intentionally deny upload.
4. If IntelliSense still reports missing `Arduino.h`, `lvgl.h`, `esp_panel_board.h`, `RF24.h` or ESP32 headers, check that the corresponding project's `.pio` packages and `compile_commands.json` exist; then run **C/C++: Reset IntelliSense Database** and reload VS Code. Do not add guessed Windows-specific Arduino library paths.
5. Keep `Remote/source/lv_conf.h` and `esp_panel_board_custom_conf.h` under `Remote/source`. LVGL font generation remains a separate licensed local prerequisite.

## Why code is greyed out / excluded
- `#if BOAT_NRF_PIN_AUDIT_SIGNED` and related radio macros evaluate to **0** in diagnostic builds. The editor correctly greys out those **uncommissioned real-hardware branches**. They are not missing files; changing the macro to 1 merely to remove grey text would bypass intentionally inactive hardware paths.
- `Boat/source/firmware/Flytec_Boat.ino` is a legacy reference; the active PlatformIO entry is `Boat/source/platformio_entry.cpp`. Boat `build_src_filter` excludes `.ino` deliberately to avoid duplicate setup/loop.
- `Remote/source/Flytec_Remote.ino` **is** the active Remote entrypoint and is included by its PlatformIO source filter. Never add a second `setup()/loop()` adapter.
- `**/.pio` is hidden in the Explorer because it is generated build output. This does **not** exclude it from compilation or IntelliSense. To browse generated headers temporarily, change `files.exclude` in VS Code.
- Archived code and tests are not part of production firmware. PlatformIO's source filters are the authoritative list, not VS Code's syntax coloring.

## Repairs in R30H-37
- Fixed invalid Boat LoRa pin `static_assert` names.
- Fixed doubly escaped regex in GPIO verification, which caused all radio pin macros to be reported as altered.
- Updated old command-codec host expectations to the explicit authenticated LoRa backup policy.
- Updated old Boat pin test to INA231 (the chosen device), not the retired ADS1115.
- Removed the retired independent physical-feedback CI gate.
- Added per-project VS Code C/C++ settings and compile-database tasks.
- Improved source-inventory diagnostics to identify failed guard numbers.

**Status:** CI and actual PlatformIO compilation must be checked. These changes repair known source/configuration defects but do not imply that every compiler error has been eliminated.
