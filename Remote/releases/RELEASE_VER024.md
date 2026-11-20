# Remote ver024 — touchscreen UI and input feedback

- Added touchscreen MANUAL, AUTOPILOT and RETURN HOME request controls to the main operating footer.
- Added live joystick raw/normalized values, calibration/read health, neutral/active indication and ADC error status.
- Added compact STOP confirmation, failsafe/low-battery/interlock readiness and pending command ACK count feedback.
- Unknown/stale boat battery telemetry is now shown as `--` rather than an assumed 0%.
- These are UI-only request/status changes. Boat-side execution is not implemented; key provisioning remains disabled and commands cannot be considered operational.
- Not compiled against target board/LVGL and not hardware tested.
