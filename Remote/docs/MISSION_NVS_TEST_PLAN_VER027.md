# Mission waypoint/NVS test plan — Remote ver027

## Preconditions
- Remote compiled for the exact Waveshare ESP32-S3-Touch-LCD-4.3 SKU and LVGL/Arduino versions.
- Display/touch, GNSS telemetry freshness and NVS access verified independently.
- Boat actuator power remains disconnected. Mission-start control must remain blocked.

## Test cases
1. **Fresh fix save:** valid GNSS timestamp less than 2s old; save one waypoint; confirm coordinates and selected slot shown.
2. **Power cycle:** restart remote; verify waypoint, home and BAIT/HOOK planning flags restored exactly.
3. **16 slots:** save 16 distinct coordinates; verify slot order, selection, and all slots available.
4. **Clear selected:** clear selected slot; restart; confirm it remains empty while other slots survive.
5. **Marker flags:** toggle BAIT and HOOK independently; restart; verify independent flags and UI labels.
6. **Home:** set home using fresh fix; restart; verify home remains; invalid/stale fix must refuse overwrite.
7. **Stale fix:** GNSS age >=2s or invalid; save and set-home actions must refuse.
8. **Corrupt record:** alter stored record bytes/CRC in a controlled test; ensure it is rejected and empty state is shown, not loaded.
9. **Storage failure:** simulate NVS begin/write failure; UI must report RAM-only or write failure and must not claim save success.
10. **Map overlay:** valid fresh fix; verify home/waypoint marker labels appear at relative positions; stale/no fix hides them.
11. **Range:** marker beyond 150m local display range is hidden; marker inside range appears. This relative grid is not a navigational map.
12. **No mission execution:** START MISSION stays disabled and no mission/release command frame is emitted in all tests.

## Pass criteria
- Persistent writes report success only when Preferences `putBytes` writes the full record.
- CRC/magic/coordinate validation rejects malformed data.
- No release, motor or mission command is sent from this UI scaffold.
- Record test evidence and firmware build hash before proceeding to real tile rendering or boat-side mission execution.

**Not yet run:** target compilation, touchscreen tests, NVS power-cycle, corruption tests, map marker tests.

13. Legacy migration: restore a CRC-valid ver026 record; confirm the first four slots and Home migrate, and upgraded record survives second restart.
14. Capacity: attempt a 17th point with all slots full; it must be refused without overwriting.
15. SD map storage: verify card mount, manifest signature, sample tile size/read, missing tile and malformed tile rejection. Renderer is not implemented yet.
