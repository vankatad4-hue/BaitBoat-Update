# Remote ver026 — waypoint persistence and relative map markers

- Mission planner now stores waypoint/home/BAIT-HOOK marker state in ESP32 NVS Preferences.
- Saved record has version magic and CRC32 integrity check; malformed record or invalid coordinates are rejected and slots reset to empty.
- Save-current and set-home continue requiring a valid GNSS fix not older than 2 seconds.
- Selecting, clearing and editing bait/hook planning markers writes the NVS record.
- Main placeholder map now overlays home and waypoint markers with BAIT/HOOK marker suffixes, relative to the current boat fix; markers outside the local ±150m display window are hidden.
- The map is still a grid placeholder, not geographic tiles, and the relative overlay is not suitable for navigation.
- Mission execution remains disabled; no boat executor, geofence, persistent route validation, authenticated command, or release sequencing is implemented.
- Source changes are not target-compiled or hardware-tested. ver026 archive/checksum not confirmed.
