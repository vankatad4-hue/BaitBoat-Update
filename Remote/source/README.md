# Remote source — current integration notes

Active Remote hardware decisions are in [Remote README](../README.md) and [Remote master status](../docs/PROJECT_MASTER_STATUS.md). The older source inventory is archived at [Remote_source_README_pre_R79.md](../../docs/archive/2026-11-22/Remote_source_README_pre_R79.md).

## Key modules

- `boat_state.*` / `boat_telemetry.*`: Boat status, telemetry freshness and safety-state application.
- `boat_ui.cpp`: main Boat telemetry screen; separate Boat-battery indicator with green/yellow/red color and warning text.
- `boat_radio.cpp`: authenticated primary/backup radio control and mission preparation gate.
- `mission_ui.cpp`: Bulgarian mission controls; blocks preparation when Boat battery telemetry is stale or charge estimate is red.
- `map_renderer.*`, `lake_storage.*`, `mission_storage.*`: SD-backed offline maps and up to 16 saved waypoints.

## R79 Boat battery UI

The Remote displays Boat voltage plus estimated percentage, not the Remote's own battery. Green is above 50%, yellow 21–50%, red 20% or lower or a Boat low-battery flag. Missing/stale data is shown as unavailable. Critical battery is shown as `КРИТ. БАТ. — STOP`, distinct from the low-battery `НИСКА БАТ. — ЗАРЕДИ/СМЕНИ` warning (RTH appears only when Home return is actually reported). The Boat remains the safety authority and independently initiates RTH.

## LVGL / PlatformIO compatibility (R30H-10 software pass)

The non-actuating, explicitly labeled DEMO sonar illustration in `boat_ui.cpp` uses integer LVGL 9.6 point coordinates instead of narrowing float initializers. This repairs the known Remote PlatformIO compiler error without changing sonar, timing, STOP/RTH, radio security, or source-of-truth Arduino behavior. A complete fresh PlatformIO and Arduino target CI result is required before calling either build verified.

The portable SD status helper (`ui_sd_status.h`) now distinguishes **no mounted TF card**, **mounted card without offline map**, and **ready offline map**; all four combinations are covered by a C++11 host regression. The previous duplicated conditional incorrectly reported “НЯМА SD КАРТА” while the card was mounted.
