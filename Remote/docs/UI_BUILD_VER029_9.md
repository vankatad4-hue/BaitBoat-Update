# Remote UI build milestone — ver029.9

Updated 2026-11-20. Source-only UI milestone; compile and simulator verification are still pending.

## Screen inventory
- **Main operating screen (800×480):** radio/battery/mode header; equal map and sonar panes; Bulgarian labels; map zoom/source/manager controls; manual, autopilot, return-home, mission, calibration and STOP controls; live joystick and safety footer.
- **Map pane:** offline renderer, GPS/fix status, map-source mode, zoom and Map Manager entry. Google online tiles remain an unimplemented optional provider; ONLINE/AUTO must not claim live imagery.
- **Sonar pane:** depth-scale labels, grid, bottom trace and illustrative fish arches. All visual traces are explicitly marked DEMO; the UI does not read sonar measurements and must not be used to infer fish/depth.
- **Mission planner:** 16 persisted waypoint slots; set home, capture fresh GPS, select, clear, bait/hook planning flags. Selected point now shows its coordinates and computes range/bearing only while GPS fix is fresh (<2 s). Mission execution remains blocked.
- **Joystick calibration:** calibration flow, ADC validity, endpoint/centre capture, dead-zone and neutral-required exit; safety lock preserved.
- **Settings:** Wi-Fi, display, controls, sonar profile, map source and system sections; user-visible content localized into Bulgarian where available. Features not supported by current hardware/software are labeled as pending.
- **Offline Map Manager:** GPS/manual centre, 1 km area, zoom 14–19, storage estimate, coverage, staged import, validation, progress and fail-closed install.

## Sonar UI integration contract
The drawing in the sonar pane is a static mock-up only. Future sonar driver data must be supplied through a validated interface that includes:
- sample timestamp / freshness;
- depth range and sample units;
- raw or processed intensity bins with a bounded buffer;
- signal-quality/invalid-data state;
- fish-arch detection metadata (optional; never required to draw the raw echogram);
- shallow/deep alarm state sourced from the sonar data processor.
No demo line is allowed to remain visible once the live-data mode is selected. When data is missing/stale, show **NO DATA** and clear the plot. UI rendering must not block radio, joystick, safety, or map tasks.

## Layout gates
- Target resolution: 800×480 landscape, LVGL 9.6.0.
- Keep STOP visible in every normal operating state; no mission/sonar/map screen may cover the hardware STOP input.
- Touch targets should be >=40 px where practical; long labels use wrap/dot modes.
- Map and sonar equal split; expand to single-pane and restore split without losing state.
- Do not report radio, battery, GPS, sonar, online-map or mission state unless its source is valid and fresh.

## LVGL Pro verification checklist (when simulator is available)
1. Load source with LVGL 9.6.0 and target display profile; inspect all screen transitions.
2. Main screen: confirm 800×480 fit, no clipped labels, equal pane sizing, footer alignment and STOP visibility.
3. Map: zoom +/-; source cycling; open/close Map Manager; full-screen map and restore.
4. Sonar: tap expand/restore; verify DEMO watermark is persistent and visually unmistakable; no tap-through to other controls.
5. Mission: test 16 slot selection; selected coordinate and range/bearing display; no fresh GPS shows unavailable; save/home blocked on stale fix; clear and bait/hook flags persist.
6. Calibration: check all strings and labels; confirm safety interlock and neutral exit.
7. Settings: open every category; scroll; verify keyboard does not obscure fields; Wi-Fi scan/connect timers terminate safely; map-source and sonar controls fit.
8. Font: verify Bulgarian Cyrillic glyph coverage, punctuation, degree sign and metre units.
9. Touch: all buttons >=40 px; no unintended parent click/expand events from child controls.
10. Runtime: check heap/PSRAM, timers cleanup on navigation, repeated screen entry/exit and LVGL warnings.
11. Record screenshot per screen and all failures; fix before release packaging.

## Explicitly not complete
No Arduino compile, LVGL Pro render, hardware touch test, live sonar driver, CHIRP processing, Google provider, boat-side mission executor, actuator operation or release build has been validated.
