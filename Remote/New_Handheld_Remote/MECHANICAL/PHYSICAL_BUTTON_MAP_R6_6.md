# Physical Button Map R6.6

Front-facing layout, two columns × four rows per side.

## Left bank L1–L8
L1 STOP; L2 Manual/Autopilot toggle; L3 Return Home; L4 Save waypoint; L5 Waypoint list/select; L6 Map full-screen; L7 Sonar full-screen; L8 Settings.

## Right bank
- R1 left bait hopper only
- R2 right bait hopper only
- R3 left hook only
- R4 right hook only
- R5 release status
- R6 release-lock/status
- R7 back/cancel

The bottom-right physical key in the right bank is **OK/confirm**, not a bank-scanned R8 key. It is wired separately to MCP23017 0x21 GPA4. MCP23017 0x20 therefore scans 15 keys (L1–L8 and R1–R7), with GPB7 reserved.

## Power and guard
One power key above the display. No duplicate bottom power key.
For releases: hold release key ≥1200ms, release, then hold OK ≥800ms within4s. STOP, stale/unverified safety telemetry, primary-link loss, timeout or cancel aborts. No release over backup LoRa.
