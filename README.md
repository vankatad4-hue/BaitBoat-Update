# Flytec V080 Bait Boat Retrofit

Firmware and hardware documentation for the Flytec V080 bait-boat retrofit.

## Project layout

- `Boat/` — boat-side firmware, configuration and hardware interfaces.
- `Remote/` — touchscreen remote firmware and UI.
- `docs/` — shared architecture, decisions, safety and validation.
- `shared/` — protocol definitions and common configuration.
- `releases/` — versioned source archives.

## Current baseline

- Preserve the Flytec V080 hull, two 12 V brushed 550 motors, battery indicator and main power switch; replace the original control system.
- Boat controller: Heltec Wireless Tracker V1.2 (ESP32-S3 + UC6580 GNSS).
- Navigation sensor: GY-91-style MPU-9250 + BMP280 module; exact board revision and wiring still require verification.
- Remote: Waveshare ESP32-S3-Touch-LCD-4.3 standard board, 800×480, LVGL 9.6.0, Arduino IDE; hardware not yet purchased.
- Primary radio: nRF24L01+ PA/LNA on both boat and remote; initial air-rate target 250 kbps; 500 m is a test target, not a guaranteed range.
- Motor driver: Cytron MDD20A dual-channel module, physically separate from the small carrier PCB.
- Solenoid driver: separate two-channel MOSFET module; bait and hook solenoids interlocked for one-at-a-time operation.
- Sonar: ready CHIRP module with live echogram/fish-arch output is the target; no module has yet passed the required verification.
- Boat carrier PCB envelope: 77 × 79 × 14 mm; keep high-current motor and solenoid paths off this carrier.

## Safety status

Motor current, startup/stall current, actual solenoid coil current, exact battery/BMS capability and final fuse/wire sizing are not yet verified. Firmware and hardware must default propulsion and solenoid outputs to OFF on reset or loss of valid control. Autopilot return-home on radio loss is allowed only when GNSS, heading, home position and boat health are valid.

## Versioning

Never overwrite a released archive. Increment the version for every deliverable modification:
- `Flytec_Boat_verXXX.zip`
- `Flytec_Remote_verXXX.zip`

The current archived baseline available in the ChatGPT project Library is ver004 for both boat and remote. See `docs/IMPORT_STATUS.md` for repository import status.
