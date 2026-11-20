# Physical Button Map R5

Front view.

## Left bank (8)
L1 STOP; L2 Manual/Autopilot toggle; L3 Return Home; L4 Save waypoint; L5 waypoint list/next; L6 map full-screen; L7 sonar full-screen; L8 menu/settings.

## Right bank (8)
R1 H-L left bait hopper only; R2 H-R right hopper only; R3 K-L left hook only; R4 K-R right hook only. R5 release status/page; R6 release-lock/status; R7 map/sonar split; R8 back/cancel. R5–R8 are UI-only.

## Confirm
Dedicated OK: hold release key >=1200ms, release, then hold OK >=800ms within 4000ms. STOP, stale/unverified safety telemetry, primary-link loss, timeout or cancel aborts. No release over backup LoRa.

## Status
Independent command IDs, codec validation, fail-closed command methods and host-tested guard are implemented. GPIO/MCP23017 scanning, debounce, event mapping and physical two-stage release guard are integrated in remote source. Boat telemetry does not yet set `actuatorSafetyReady`, so releases remain blocked. Boat solenoid execution/interlocks are not implemented.

## Preview
See the [R5 SVG front-panel preview](../CAD/REMOTE_PREVIEW_R5.svg). The layout illustration uses L1–L8 for left-side functions and R1–R8 for the right side; R1–R4 are independent hopper/hook releases.
