# Flytec Remote ver028 — offline map renderer and SD waypoint storage

## Changes
- Added geographic Web-Mercator map rendering for SD XYZ RGB565 tiles.
- Bounded 3×3 viewport tile slots with missing-tile fallback and tile reuse.
- Zoom controls; 5–19, default zoom 15.
- Boat marker, all 16 WP markers with BAIT/HOOK tags, Home and route line to selected WP or Home in Return-Home mode.
- Mission/Home persistent record now on SD at /missions/waypoints.bin, CRC checked and read-back verified; NVS remains backup/fallback.
- Added MBTiles converter tool and storage test plans.

## Not complete / verification gate
- No map data is included in ZIP. Populate SD /maps using a lawful MBTiles source and converter.
- North-up only; pan, style selection, route planning and on-screen attribution are pending.
- No build or hardware validation has been run. Compile LVGL 9.6.0/Waveshare, test display/touch, tile reads and SD write recovery before treating this as a usable firmware release.
- Boat mission execution, autopilot, sonar and actuator functions remain unavailable and fail-closed.
