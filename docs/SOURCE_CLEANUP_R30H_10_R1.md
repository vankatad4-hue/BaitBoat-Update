# Flytec R30H-10 software cleanup / build compatibility — 2026-11-22

## Scope and confirmed initial fault

GitHub Actions PlatformIO run [#45059667477](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45059667477) completed with Boat **SUCCESS** and Remote **FAILURE**. Remote `boat_ui.cpp` sonar DEMO uses float-list initializers for `lv_point_precise_t`, which in that LVGL 9.6/toolchain resolves to an integer precision type. Remote compilation failed on narrowing at lines 397–398; other coordinates produced related warnings.

## Applied non-functional cleanup

1. Convert the illustrative sonar line/grid/arch points to bounded integer pixel math. Reject width/height under two pixels. Do not connect a real sonar or change DEMO labeling.
2. Exclude the retired **Heltec SX1262** source implementation from both Boat PlatformIO and staged Arduino diagnostic builds. Retain source and historical regression tests; remove the unused RadioLib target dependency. Keep selected E22 UART backup and nRF24 active code unchanged.
3. Correct Boat source README's stale ADS1115-as-active language: the selected INA231 BUS-only implementation is still uncommissioned and fails closed.

## Constraints and verification

- The current one-common-3S motor/solenoid bus design, independent hardware STOP/current-zero blocker, uncommissioned pin audits, and all authentication, RTH, battery-failsafe and physically disabled radio gates remain unchanged.
- No bulk deletion of legacy modules with live tests or reference documents: retrospective files stay for traceability. This is a conservative source cleanup, **not** proof that all project modules are optimized or release-ready.
- Required: Remote and Boat host tests, Arduino target builds, **both** experimental PlatformIO targets. A passing compile proves only source compatibility, not physical RF/STOP, GPS/PPS, PWM or current measurements.
- R30H-10 physical scope remains blocked pending actual board/module inspection and passive analyzer captures; authorized Boat VER014 / Remote VER028 stay fixed.

## Follow-up safe UI correction

The main screen's duplicated SD label selection was inverted for `sdCardReady=true`. It now uses `Remote/source/ui_sd_status.h`, a pure side-effect-free helper shared across GPS-fix/no-fix display paths and checked by `Remote/tests/ui_sd_status_test.cpp` in Remote portable CI. This does not modify storage mounting, file writes, map indexing or safety rules.

## Additional pin safety audit

Confirmed a previously underdocumented **Remote GPIO6 collision**: `displayPort_mountSD()` uses it for Arduino SD dummy CS while `radio_hw_config.h` proposes nRF CE on the same wire, and Waveshare labels it Sensor AD. The SD implementation now takes its dummy pin value from a single named macro, not two magic numbers. A conditional compile-time `#error` forbids signing the nRF audit flag while SD dummy CS and nRF CE remain equal. This is a safety guard, **not a pin reassignment**. GPIO15/nRF CSN versus RS485 TXD remains a second unsigned physical conflict. RF-only hardware must not be connected/energized until both are resolved.

## Removal of proven unreferenced legacy source (non-destructive archive)

After the read-only [source-inventory workflow](../.github/workflows/source-audit.yml) narrowed a 203-file inventory to five review-only headers, inspected the contents and archived precisely: old R36/R02 `Boat/source/config/{boat_config.h,radio_config.h}`, obsolete `Remote/source/boat_config.h` dimensions, and the retired Heltec SX1262 implementation pair `Boat/source/heltec_sx1262_link.{cpp,h}`. The byte-for-byte originals are retained under [archive](archive/2026-11-22/source_cleanup_r30h10/README.md), with git history. No live source/header includes or portable host tests referenced those files. `heltec_sx1262_airtime.h` remains in live source for an intentionally retained portable regression. All other candidate headers are kept, including commissioned-config examples and LVGL/ESP panel externally included headers. No motor/radio/safety source logic removed.

## Post-archive verification

The follow-up read-only source inventory on [GitHub Actions #45061849119](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849119) completed **SUCCESS** with **198 source files**, **57 active compiler translation units/sketch roots**, and **one intentionally retained example header** (`Boat/source/tracker_sensor_profile.example.h`). The archived five files are absent from the active source tree. Boat host [#45061849147](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849147) and Remote host [#45061849142](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849142) both completed SUCCESS on archive commit `fa7d2f17`. Arduino ESP32 and experimental PlatformIO target verification are separate and must not be inferred from these host passes.

## Experimental PlatformIO dependency discovery regression

Real CI [#45061849117](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849117): Boat PlatformIO compiled, Remote still fails **after** the fixed LVGL code because `WiFiGeneric.h` cannot find `Network.h`; the PIO dependency graph omits Arduino framework `Network` despite `WiFi`. Remote `lib_ldf_mode` now uses `deep` instead of `deep+` to test known preprocessor-aware discovery incompatibility. Pending new PIO CI, no Remote PIO PASS claim. Arduino targets both PASS on `fa7d2f17` (Boat [#45061849197](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849197), Remote [#45061849238](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849238)).

## Native Remote sketch instead of duplicate entry adapter

[PIO #45062526394](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45062526394) overcame the former `Network.h` discovery issue with `lib_ldf_mode=deep`, but failed at Remote link due to duplicate `setup()`/`loop()` from automatically generated root `Flytec_Remote.ino.cpp` plus `Remote/source/platformio_entry.cpp` (which manually included the same sketch). The redundant Remote adapter is now archived and excluded from active builds; root `.ino` is compiled once directly. Boat's **subdirectory** adapter remains needed and untouched. All actual hardware/RF/STOP/PPS checks still NOT_RUN. New PlatformIO CI verification pending.


## R30H-10 source optimization and verified dual-build migration — 2026-11-22

See [full source cleanup and build evidence](SOURCE_CLEANUP_R30H_10_R1.md).

- **Both PlatformIO targets: SUCCESS** — [workflow #45063024581](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024581), Boat N32R16V and Remote Waveshare Standard with ESP32 Arduino core 3.3.12, compilation-only, no upload.
- **Both Arduino reference targets: SUCCESS** on the archived-source revision — [Boat #45061849197](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849197) and [Remote #45061849238](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849238). Remote target rerun for latest PIO adapter cleanup is separately tracked under [#45063024540](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024540).
- **Both host workflows: SUCCESS** after the Remote entrypoint correction — [Remote #45063024532](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024532); Boat host [#45061849147](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45061849147); static active-source inventory [#45063024449](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45063024449) SUCCESS.
- **197 active-source files, 56 sketch/C++ compile roots, one intentionally preserved reference/example header**. Six retired unreferenced or duplicated files were preserved in [history](https://github.com/vankatad4-hue/BaitBoat-Update/tree/hardware-ready-dual-radio-r1/docs/archive/2026-11-22/source_cleanup_r30h10) and removed from live sources. This is static review and safe cleanup, not complete runtime performance optimization.
- Remote LVGL 9.6 sonar DEMO point coordinates repaired (float-to-integer), duplicate SD status corrected with 4-case host test. Retired Heltec SX1262 and unused older configuration files archived. Boat uses selected nRF24/E22 UART and INA231 BUS-only; sonar still deferred.
- **Safety/physical acceptance remains OPEN**: no real radio STOP/RTH/PPS accuracy or independent STOP/current-zero acceptance, no motor/solenoid power approval. Remote GPIO6 (SD dummy CS/nRF CE) and GPIO15 (RS485/nRF CSN) remain electrically conflicted and unsigned. Keep Boat VER014 / Remote VER028 and reference Arduino build; passing PIO is a viable parallel compilation path, not firmware promotion.

