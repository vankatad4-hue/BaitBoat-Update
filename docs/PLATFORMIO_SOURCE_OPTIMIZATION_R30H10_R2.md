# Flytec R30H-10 — PlatformIO optimization R2 (2026-11-22)

**Development:** VS Code + PlatformIO is primary. This revision applies conservative source improvements. Official Boat VER014 / Remote VER028 unchanged.

## Source changes
- Remote: current RF primary/backup connectivity is applied **before** computing telemetry-derived actuator readiness. A pending STOP still overrides any fresh status. Physical/touchscreen/boot-pressed STOP use one handler; fewer inconsistent `millis()` reads for telemetry and UI cadence.
- ADS1115: validate unique inputs and address, drop stale ready state on reinit failure, stage four axis values, and publish nothing on partial I2C/conversion errors. Add portable deterministic injected-error regression.
- Boat: one timestamp for manual release completion telemetry and radio link freshness; motor/solenoid policy unchanged.
- PlatformIO: Boat LDF discovery `deep` (previously `deep+`) to match proven ESP32-S3 Remote behavior; VS Code C++ IntelliSense and .ino associations. Rebuild confirmation required.
- CI: portable ADC regression in Remote host and PIO checks, package cache (no firmware caching), PR target checks, and source-adapter uniqueness enforcement.

## Reproduce
```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -IRemote/tests/adc_stubs -IRemote/source Remote/source/joystick_adc.cpp Remote/tests/joystick_adc_atomic_test.cpp -o /tmp/remote_adc && /tmp/remote_adc
python Boat/tools/verify_r30h10_physical_lock.py
pio run -d Boat -e boat_n32r16v_diagnostic
pio run -d Remote -e remote_waveshare_43_diagnostic
```

**Evidence note:** prior PIO CI #45063024581 applies to code *before* this revision. Do not claim these changes target-compile verified until fresh CI succeeds. No actual GPIO, nRF/E22, PPS or independent STOP current-zero measurement has occurred; all 18 physical gates unsigned, upload denied. Remote GPIO6 and GPIO15 conflicts remain. This is focused cleanup, not indiscriminate rewriting of 197 source files.
