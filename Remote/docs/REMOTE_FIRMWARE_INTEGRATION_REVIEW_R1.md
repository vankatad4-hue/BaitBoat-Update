# Remote Firmware Integration Review R1

Updated: 2026-11-20  
Branch: `hardware-ready-dual-radio-r1`  
Status: source review only; not compiled or hardware validated.

## Review scope
Reviewed the current remote entry point and core modules: `Flytec_Remote.ino`, `boat_state`, `boat_ui`, `joystick`, `joystick_adc`, `physical_button_scanner`, `release_button_guard`, `boat_commands`, `boat_radio`, `boat_telemetry`, `dual_radio_manager`, and `display_port`.

## Current integration that exists
- One application setup/loop and shared state initialisation.
- Display port is initialised before LVGL UI creation.
- Radio, telemetry, button scanner, joystick reader and UI refresh are called from the main loop.
- Commands are routed through a registered command transport.
- Button IDs map STOP, mode toggle, RETURN HOME, waypoint placeholders, fullscreen map/sonar, settings placeholders, and release-bank status/cancel placeholders.
- The four release keys are routed through the hold-key + separate OK hold guard. Release remains disabled unless primary authenticated link, fresh STOP state and actuator-ready telemetry are all true.
- Joystick input is fail-closed when ADS1115 reads fail or calibration is not valid.

## Blocking integration gaps
1. **Authentication provisioning:** `radioKey_get()` intentionally fails closed unless a key exists. There is no safe provisioning/rotation UI or documented commissioning procedure. Radio control TX is therefore unavailable by design.
2. **Inbound application integration:** `boatRadio_update()` validates inbound frame authenticity/session/sequence, but does not dispatch valid ACK/telemetry payloads into command acknowledgement and `telemetryApplyToBoatState()`. The current telemetry state is not driven by a real decoded boat payload.
3. **Command acknowledgement:** command send success means the transport accepted/queued a frame, not that the boat acted. STOP confirmation and other command results need explicit ACK/telemetry correlation and timeouts.
4. **Radio hardware readiness:** nRF GPIO assignments and shared SPI/SD conflicts remain provisional; E22 bridge/module configuration is unverified. Drivers are not compiled or tested on the exact hardware.
5. **Mission controls:** physical keys for save/select waypoint and touchscreen mission controls are placeholders; mission data and command flow are not complete.
6. **Navigation/sonar views:** map is still a diagnostic placeholder/grid and offline map utilities do not render a full tile map. Sonar panel is not a live echogram.
7. **Settings-to-boat sync:** sonar profile and remote settings are stored locally; no boat-side settings protocol exists. UI options should be clearly marked local-only/pending.
8. **Power/display validation:** LVGL 9.6 and Waveshare library signatures, flush completion semantics, touch orientation/calibration, SD operation alongside radio SPI, and backlight behavior require actual build and board testing.
9. **Loop scheduling:** I2C scanning/ADC operations are synchronous. Measure worst-case loop latency and watchdog/radio service time; replace blocking reads with bounded/non-blocking tasks if required.
10. **Safety readiness:** `actuatorSafetyReady` is false by default and no trusted telemetry currently sets it true. This is correct fail-closed behavior and must not be bypassed.

## Required next work — remote software
### R1 — App/state/telemetry contract
Define a versioned payload schema for heartbeat, boat status, command ACK, link metrics, GNSS, battery, motor status, STOP latch and actuator-interlock flags. Implement parser/dispatcher with bounds checks, freshness checks and sequence/session rules. Connect valid payloads to telemetry state and ACK correlation.

### R2 — Command lifecycle
Implement command IDs across retries and radio failover; distinguish queued, transmitted, acknowledged, rejected, timed-out and expired. STOP has priority. Prevent stale manual-control packets from being replayed. Only trusted fresh boat telemetry can confirm STOP release or safety readiness.

### R3 — Remote UI integration
Finish navigation, waypoint, release/status and settings interactions; explicitly display disabled/pending functions. Ensure controls reflect acknowledged state rather than optimistic local state.

### R4 — Build and test
Add a reproducible build configuration (board/core/library versions and compile flags), compile on host where possible, then compile the complete Arduino target. Add tests for protocol dispatch, command ACK lifecycle, stale telemetry, failover restrictions, joystick calibration and button/release state machine.

### R5 — Hardware bench gate
Display/touch only; ADC and buttons; radio pair with actuators disconnected; authentication and ACK; failover; STOP and failsafe verification. Do not energise motors or solenoids until the separate boat electrical/actuator review passes.

## Conclusion
The remote is an integrated scaffold with significant working source modules, but it is not a complete/operational firmware. The next coding phase should start with the **telemetry and command lifecycle contract**, then finish UI wiring around confirmed data, rather than adding new screens while the control path is still disconnected.
