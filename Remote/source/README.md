# Remote Source — Current Development Status

Updated 2026-11-20. The GitHub repository is canonical; this source is a development scaffold, not a tested release.

## Versioning
- ZIP: `Flytec_Remote_verXXX.zip`.
- Preserve older archives; current marker `releases/VERSION` is `006`.
- Increment for every package change and follow `../../docs/ZIP_RELEASE_POLICY_R1.md`.

## Controller/display baseline
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948, 800×480 RGB, GT911 touch, CH422G expander.
- Arduino IDE; target LVGL 9.6.0 only. Board-support libraries are separate from LVGL.
- Confirm Arduino-ESP32 core, board menu settings, PSRAM and exact display libraries before compile.
- Display adapter is a first integration pass, not compile or physical validation.

## Remote hardware/radio baseline
- Primary nRF24L01+ PA/LNA, 2.4GHz; left external antenna.
- Backup EBYTE E22-900T22S UART, EU868 candidate; right external antenna.
- E22 UART bridge: SC16IS750 I2C address 0x48; I2C GPIO8/9. UART 9600 8N1.
- Provisional nRF CE GPIO6, CSN GPIO15, SPI MOSI11/SCK12/MISO13. GPIO6 sensor-AD path and GPIO15 RS485 TX are repurposed; isolate/disconnect onboard functions and verify exact board revision before wiring.
- nRF channel76, 250kbps, fixed32, CRC16, PA_LOW initial.
- E22 candidate channel18=868.125MHz, 62.5kbps, 32-byte subpacket, 10dBm, transparent; exact module variant and configuration readback must be confirmed.
- 1S4P 18650 battery proposal. The Waveshare PH2.0 input/charger recommendation, protection and simultaneous load behavior must be checked; do not assume the 2400mAh pack proposal is approved by the onboard charger.

## Code inventory
- `Flytec_Remote.ino`: app entry and setup.
- `display_port.*`: board LCD/touch and LVGL adapter.
- `boat_state.*`, `boat_commands.*`, `boat_radio.*`: application state/command scaffold.
- `nrf24_link.*`, `e22_uart_link.*`, `sc16is750_bridge.*`: radio transport and UART bridge.
- `radio_protocol.*`, `radio_command_codec.*`, `radio_key_store.*`: CRC/HMAC frame and command safety scaffolds.
- `dual_radio_manager.*`: link state/failover/recovery policy.
- `physical_button_scanner.*`: MCP23017 button scan/debounce; right-bank release keys and separate OK input feed the release-confirmation state machine. Board pin map is provisional.
- `release_button_guard.*`: two-step release guard, primary-link and boat interlock gate; native host test available.
- `joystick.*`, `joystick_calibration_ui.*`: calibration UI; hardware ADC reader currently fails closed.
- `boat_ui.*`, `settings_ui.*`: UI scaffold; map and sonar displays are not operational data views.
- `offline_map.*`, `map_projection.*`: tile validation/read and projection utilities, not a complete geographic map renderer.
- `boat_telemetry.*`: telemetry structure/validity; radio telemetry is not connected.

## Bench test sketches
- `hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `hardware_test/E22_SC16IS750_Bench/E22_SC16IS750_Bench.ino`
- Fixed diagnostic probes only, CRC checked, no authenticated commands or actuator payloads. See `hardware_test/README.md`.
- Bench tests remain uncompiled/unverified.

## Safety / limitations
- Key storage returns false by default; authenticated communication/control is disabled.
- Session/replay protection, application ACK, state synchronization and boat-side command execution are incomplete.
- No joystick ADC input, boat-side actuator execution, waypoint/mission save/select, autopilot, return-home execution, sonar hardware integration or live telemetry is operational. Release GPIO scanning is coded but not physically verified; safety readiness remains false by default.
- Remote STOP is not an independently validated safety system.
- Do not connect actuator power or operate the boat.

## Canonical project record
Read `../docs/PROJECT_MASTER_STATUS.md` for the full project state, requirements, hardware selections, blockers and release gates.

## Joystick subsystem update

`joystick_adc.cpp` reads four ADS1115 inputs (provisional 0x4A/AIN0–AIN3). The main sketch uses this reader; failed reads suppress stick output. Calibration and control preferences are stored in ESP32 NVS and only a fully validated endpoint capture sets the calibration-valid flag. Controls Settings exposes layout, override, dead-zone and calibration entry. See `../docs/JOYSTICK_ADC_INTEGRATION_R1.md`.
