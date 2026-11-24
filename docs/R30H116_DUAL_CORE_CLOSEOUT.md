# R30H-116 closeout — 2026-11-24

Paused at user request. Next session starts R30H-117, not a release.
Branch: `platformio-unified-r30h20`. Official Boat VER014 / Remote VER028.

## Verified baseline before closeout edits

- Lifecycle/latency host suite: [45340287169](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45340287169) PASS.
- Standalone bench matrix: [45340310819](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45340310819) all three compile targets PASS.
- Remote host: [45340281048](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45340281048) PASS.
- Source inventory: [45340260843](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45340260843) PASS.
- Package: [45340321334](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45340321334) PASS.
- Primary target [45340281060](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45340281060): Boat PASS, Remote running at inspection. Do not infer final result.

## Consistency review of production call paths

| Area / files | Current state | Integration dependency |
|---|---|---|
| Boat firmware, boat_application_port, boat_dual_radio_port | Main loop retains all application and physical radio ownership | Split PHY polling/admission from authentication, session, ACK/status, command queue, mission and actuator logic; never run both owners |
| Boat GNSS, sensor/telemetry and safety loop | Existing application-owned call path, monotonic timing | Keep sensor Wire, motor and solenoid ownership on application side; bound serial diagnostics and sensor work |
| Remote Flytec_Remote, boat_radio, boat_commands, boat_state, telemetry | Single Arduino task; no new worker activated | Route copied frames and outcomes without migrating mutable session/UI state |
| Remote ADC, buttons, MCP CE, SC16/SC18 bridge, GT911, CH422G | Owner-task entry fences | Vendor internals and bus timing remain unverified; Core 0 must not call Wire or physical adapters |
| Remote joystick calibration | Synchronous 32 samples, delay(4) plus ADS conversion waits | At least 128 ms delay alone, exceeding provisional 100 ms request age; cooperative calibration required before migration |
| Remote display/map/SD | Existing application-side UI/storage | Audit blocking LVGL callbacks and SD/file operations, measure service gap; no second LVGL owner |
| PHY completion and safety | nRF link evidence, E22 local evidence distinct | Never treat either as authenticated STOP ACK or physical current-zero |
| Timing stats | Owner-only snapshots, Core 0 read after join | Wall-clock duration is not CPU utilization; real CPU/stack/RAM measurements pending |

Review was targeted at active entry points, transport adapters, control paths and source inventory; not a fresh line-by-line audit of every tracked file.

## Small consistency fixes saved before pause

- Boat `boat_dual_radio_port.cpp` 41fbc12: revoke E22 availability and authenticated RX freshness on sticky local transport fault; discard backup non-emergency queue entries and pending feedback.
- Remote `boat_radio.cpp` 652d1b2: revoke backup transport readiness and pending local retry on E22 fault without clearing STOP confirmation requirement.
- Remote `display_port.cpp` 4447a9c: rejected foreign-task touch read returns RELEASED instead of leaving input state unspecified.
- These changes require new CI/target verification; baseline PASS does not attest new commits.

## Resume checklist — R30H-117 (not started)

1. Verify latest CI/packaging, especially Remote primary target and closeout commits.
2. Establish production ownership/call graph and migration contract, including STOP/ACK/session semantics and late completion handling.
3. Replace blocking calibration path and inspect SD/LVGL/vendor I2C service gaps; define commissioning metrics for CPU/stack/RAM and latency.
4. Plan Boat and Remote migration separately with no concurrent legacy/worker ownership, default-disabled hardware gates, and fault tests.
5. Keep RF, motor, solenoid and upload locks unchanged until actual acceptance.

Physical testing NOT_RUN/BLOCKED. No release increment or flashing. No scheduled continuation requested.
