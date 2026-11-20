# Remote ver025 — waypoint and mission-planning UI scaffold

- Added a mission-planning touchscreen accessible from the operating footer.
- Four local waypoint slots support selecting, saving a fresh GNSS coordinate, and clearing a slot.
- A local HOME coordinate can be set from a fresh GNSS fix.
- Selected waypoint can be tagged for planned BAIT and HOOK actions; these are planning flags only.
- Mission-start control is deliberately blocked with an explanation.
- Waypoints/home are volatile RAM only; no persistent storage, navigation executor, geofence/mission validator, secure command transport or release-action implementation is integrated.
- Fix freshness gate uses the remote's existing BoatState GNSS validity and timestamp; save/home are rejected when the fix is invalid or older than 2 seconds.
- Added mission-screen SVG preview.
- Also corrected missing UI status-label declarations introduced in the ver024 footer update.
- Target compile and hardware validation remain pending.
