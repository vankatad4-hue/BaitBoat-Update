# Flytec V080 — VS Code + PlatformIO migration R1

**2026-11-22 — parallel, experimental source migration; official Arduino CLI remains the reference until two native PlatformIO target builds pass.** This is not an ESP-IDF/CMake rewrite and does not change the firmware's radio, safety, map, GNSS UTC, joystick or LVGL logic. Existing Boat VER014 and Remote VER028 are unchanged; all physical RF/STOP/actuator checks remain unsigned.

## Why two PlatformIO projects

`Boat/platformio.ini` and `Remote/platformio.ini` are independent projects with different hardware targets. Open the provided `Flytec_V080.code-workspace` to browse both, or open the `Boat` and `Remote` subfolders separately in VS Code when running PlatformIO tasks.

Both reuse the same version-controlled firmware via `src_dir = source`, but their sketch layouts differ. **Boat:** `firmware/Flytec_Boat.ino` is in a subdirectory, so the guarded `Boat/source/platformio_entry.cpp` adapter includes it once; `firmware/**` is excluded from separate PIO compilation. **Remote:** `Flytec_Remote.ino` is at the source root, so PlatformIO automatically creates one `.ino.cpp` unit. The redundant Remote adapter was moved to the R30H-10 historical archive after it caused multiple definitions of `setup()/loop()` at link time. Both Arduino target workflows still compile their original .ino sketch. **Do not move or duplicate the firmware or edit generated LVGL code to migrate.**

## Local Windows installation

Install [VS Code](https://code.visualstudio.com/) and the official [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode). Clone the existing branch:

```powershell
git clone --branch hardware-ready-dual-radio-r1 https://github.com/vankatad4-hue/BaitBoat-Update.git
cd BaitBoat-Update
code Flytec_V080.code-workspace
```

From a VS Code PlatformIO terminal (or `pio` installed in PATH), compile without flashing:

```powershell
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

**The PlatformIO Upload action is intentionally blocked in both projects** by `upload_protocol = custom` and `tools/pio_deny_upload.py`. A successful build is **not** a safety signoff. Do not bypass this lock for real motors, solenoids or unverified radio modules.

## Board memory and versions

| Parameter | Boat | Remote |
|---|---|---|
| Controller | ESP32-S3-DevKitC-1-N32R16V | Waveshare ESP32-S3-Touch-LCD-4.3 **Standard** |
| PlatformIO environment | `boat_n32r16v_diagnostic` | `remote_waveshare_43_diagnostic` |
| Arduino framework | PlatformIO espressif32 **8.0.0** includes Arduino core **3.3.12** | Same |
| Existing reference | Arduino CLI ESP32 core **3.0.2** | Arduino CLI ESP32 core **3.0.2** |
| Flash | 32 MB, **OPI** | 16 MB, **QIO** |
| PSRAM | 16 MB, **OPI** | **OPI** per selected Waveshare hardware |
| `board_build.arduino.memory_type` | `opi_opi` | `qio_opi` |
| Application partition | `huge_app.csv` diagnostic, no OTA | `huge_app.csv` |
| UI | None | Waveshare RGB ST7262 / GT911, 800×480, LVGL **9.6.0** |

The generic `esp32-s3-devkitc-1` board manifest **does not describe the actual N32R16V or Waveshare module memory automatically**. These overrides are mandatory to verify against purchased boards. No automatic copying of GPIO or changing `BOAT_*_AUDIT_SIGNED` / `REMOTE_*_AUDIT_SIGNED` is allowed.

PlatformIO core v3.3.12 is a **toolchain upgrade** from the existing Arduino v3.0.2 reference. Library API/board SDK compatibility is **unproven until both PIO builds finish**. Do not promote PlatformIO over the existing Arduino builds on the basis of `platformio.ini` alone.

## Libraries and Bulgarian font

Boat currently requests `RF24 1.4.11` and `RadioLib 7.3.0`; Remote requests `LVGL 9.6.0`, `RF24 1.4.11`, `ESP32_Display_Panel v1.0.0`, `ESP32_IO_Expander v1.0.1` and `esp-lib-utils v0.1.2`. Versions correspond to the previously working Arduino CI dependencies; any mismatch under PlatformIO should be corrected **in the PIO migration** first, without silently upgrading official firmware.

The Bulgarian LVGL font is generated at build time from a legally installed DejaVuSans source and corresponding license. Font files are **not stored in GitHub**. Linux CI performs:

```bash
sudo apt-get install fonts-dejavu-core
bash Remote/tools/generate_bg_font.sh
cp out/ui-font/flytec_bg_14.c out/ui-font/flytec_bg_14.generated.h Remote/source/
pio run -d Remote -e remote_waveshare_43_diagnostic
```

For Windows, generate the font from a **locally licensed** DejaVuSans.ttf using the existing `Remote/tools/generate_bg_font.sh` in WSL/Linux (requires Node.js and `lv_font_conv` via `npx`), then copy only your *local generated* `flytec_bg_14.c` and `flytec_bg_14.generated.h` into `Remote/source/` before the PIO build. Keep the original font license nearby. The generated files are ignored by `.gitignore`. The code can build without this optional generated resource, but the complete Bulgarian glyph coverage then is **not guaranteed**, so do not promote that build as an accepted Remote UI.

## CI and test boundary

[Experimental PlatformIO migration workflow](../.github/workflows/platformio-migration-build.yml) invokes `pio run` for both platforms without flashing, records compiler logs, and verifies that radio and physical safety audit flags remain locked. Existing Arduino CLI target workflows **remain active, independent and authoritative**. Existing host tests are unchanged.

If a PlatformIO job fails, inspect its compile log; common migration issues are missing dependency/version pins, ESP32 Arduino 3.3 versus 3.0 board API drift, PSRAM/flash mode, LVGL `lv_conf.h` include path, display panel's custom config and font resources. A CI installation or package-download failure is **not** a passing compile.

**Success requirement before changing the official build system:** both PlatformIO target builds cleanly compile the full existing app; versions and SDK flags are recorded; all existing host tests and legacy Arduino target builds still pass; flash/PSRAM size is checked on actual boards; then safe actuator-disconnected hardware bring-up and R30A physical safety acceptance are still required. This migration alone adds no hardware purchase, no custom PCB, no radio transmission and no independent physical STOP.

## R30H-10 PlatformIO transitive Network dependency fix (2026-11-22)

After the LVGL integer-point fix, [PlatformIO CI #45061849117](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849117) produced **Boat SUCCESS** and **Remote FAILURE** at `Remote/source/settings_ui.cpp` → `WiFi.h` → `WiFiGeneric.h` `Network.h: No such file or directory`. PIO's dependency graph contained `WiFi @ 3.3.12` but omitted the framework `Network` library. The standalone Arduino Remote compile [#45061849238](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849238) is SUCCESS.

The diagnostic Remote `platformio.ini` switches **`lib_ldf_mode = deep+` → `deep`** (transitive header search without the problematic extra preprocessor macro simulation). This is a configuration-only trial motivated by Arduino-ESP32's historical report [espressif/arduino-esp32#9782](https://github.com/espressif/arduino-esp32/issues/9782), which describes this exact class of PlatformIO Network.h failure with `deep+` and success with `deep`. The new result is **UNVERIFIED until the fresh Remote PlatformIO CI completes**; subsequent unresolved dependencies, if any, must be fixed individually. No production Arduino toolchain change, radio pin unlock, actuator output, or version promotion is authorized.

## R30H-10 duplicate-entrypoint linker fix

On [PlatformIO #45062526394](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45062526394), Boat compiled successfully and Remote reached linking after the `lib_ldf_mode = deep` fix; Remote failed with duplicate `setup()` and `loop()` definitions from `Flytec_Remote.ino.cpp.o` and `platformio_entry.cpp.o`. The second Remote-only entry adapter is now removed from the active source tree and retained as historical archive. The single native root-level `Flytec_Remote.ino` is explicitly included by the source filter. This is a **compile-only change**; no radio/STOP/RTH/SD functionality, safety pin flags or official versions changed. A fresh completed PlatformIO CI run is required before PASS.


## R30H-10 source optimization and verified dual-build migration — 2026-11-22

See [full source cleanup and build evidence](SOURCE_CLEANUP_R30H_10_R1.md).

- **Both PlatformIO targets: SUCCESS** — [workflow #45063024581](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024581), Boat N32R16V and Remote Waveshare Standard with ESP32 Arduino core 3.3.12, compilation-only, no upload.
- **Both Arduino reference targets: SUCCESS** on the archived-source revision — [Boat #45061849197](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849197) and [Remote #45061849238](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849238). Remote target rerun for latest PIO adapter cleanup is separately tracked under [#45063024540](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024540).
- **Both host workflows: SUCCESS** after the Remote entrypoint correction — [Remote #45063024532](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024532); Boat host [#45061849147](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849147); static active-source inventory [#45063024449](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024449) SUCCESS.
- **197 active-source files, 56 sketch/C++ compile roots, one intentionally preserved reference/example header**. Six retired unreferenced or duplicated files were preserved in [history](https://github.com/vankatad4-hue/BaitBoat-Update/tree/hardware-ready-dual-radio-r1/docs/archive/2026-11-22/source_cleanup_r30h10) and removed from live sources. This is static review and safe cleanup, not complete runtime performance optimization.
- Remote LVGL 9.6 sonar DEMO point coordinates repaired (float-to-integer), duplicate SD status corrected with 4-case host test. Retired Heltec SX1262 and unused older configuration files archived. Boat uses selected nRF24/E22 UART and INA231 BUS-only; sonar still deferred.
- **Safety/physical acceptance remains OPEN**: no real radio STOP/RTH/PPS accuracy or independent STOP/current-zero acceptance, no motor/solenoid power approval. Remote GPIO6 (SD dummy CS/nRF CE) and GPIO15 (RS485/nRF CSN) remain electrically conflicted and unsigned. Keep Boat VER014 / Remote VER028 and reference Arduino build; passing PIO is a viable parallel compilation path, not firmware promotion.


Compilation is now verified for Boat and Remote in the final experimental PlatformIO CI. Keep the established Arduino CLI targets and do not enable hardware signoffs; retain distinct ESP32 core version checks.
