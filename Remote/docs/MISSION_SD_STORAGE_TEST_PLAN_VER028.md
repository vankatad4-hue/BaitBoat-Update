# SD waypoint persistence test plan — ver028

1. Use a FAT32 card; confirm remote Serial says TF card ready.
2. Open Mission. Save a fresh GPS fix into WP01; wait for SD confirmation.
3. Inspect card: /missions/waypoints.bin exists and has a non-zero fixed record size.
4. Set Home and add BAIT/HOOK tags; verify each action reports SD success.
5. Power cycle remote with card inserted. Confirm all 16 slots, Home, flags and selected waypoint restore.
6. Remove card and reboot. Confirm NVS fallback restores the last mirrored record (if mirror was written).
7. Reinsert card with SD record; verify SD record is preferred over NVS.
8. Rename/corrupt/truncate the SD record; verify CRC/size rejection and NVS migration/recovery status.
9. Fill all 16 slots; save into selected slot and then verify full-slot behavior.
10. Simulate a failed write (read-only card/full card where safe); UI must not claim success.
11. Power interruption during temporary-file write/rename; inspect recoverability. No boat operation during this test.
