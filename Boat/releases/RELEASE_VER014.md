# Boat ver014 — fail-closed application policy scaffold

- Added portable `BoatCommandPolicy` with a boot-latched STOP state, STOP/clear-stop conditions, mode-switch prerequisites, link-specific freshness, manual steering caps and independent release-channel checks.
- Clear-STOP requires primary link, physical STOP released, battery healthy and neutral motors.
- Autopilot requires primary link plus fresh GNSS/heading, geofence and valid mission target.
- Return-home requires the specific incoming link fresh and navigation/home prerequisites valid.
- Backup manual steering is capped at ±250; primary manual steering is capped at ±1000. Each command is checked against freshness of its own link.
- Independent release requests require primary, STOP clear, neutral propulsion, battery/interlock conditions; autopilot release additionally requires release-zone confirmation. Combined release commands remain invalid.
- Outputs are decision/request fields only. No GPIO, motor or solenoid driver is connected.
- Host test source and CI workflow are added; no successful CI result or compile has yet been observed.
