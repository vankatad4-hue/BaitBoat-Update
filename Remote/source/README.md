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
