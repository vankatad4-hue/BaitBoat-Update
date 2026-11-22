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
