# R30H-34 — LoRa command parity implementation and limits

**Scope:** expand authenticated E22 fallback beyond STOP/RTH, without making false claims of full nRF equivalence.

## Implemented
- Both Remote and Boat command codecs share the same explicit backup whitelist: emergency STOP; mode change to RETURN_HOME or AUTOPILOT; individually addressed left/right hopper and left/right hook release.
- Remote link manager and E22 send transport use one policy function; Boat independently checks the command codec.
- Existing Boat release one-hot/pulse limits, STOP state, mission state, command ID deduplication, radio authentication and physical-output software checks remain mandatory.
- Manual motor commands, CLEAR_STOP and legacy combined-release commands remain denied over LoRa. This is intentional, not a missing GPIO assignment.
- Boat/Remote LoRa wiring remains as in R30H-33. No dedicated external hardware safety elements added.

## Why this is NOT full redundancy
- The current LoRa transmit pacing is approximately **3000 ms** and the primary motor command lease is **200–250 ms**. Simply enabling manual packets would produce stale motor commands and is not an acceptable implementation.
- Both sides currently establish/rekey the authenticated session via primary nRF. An E22-only cold boot cannot establish an independent authenticated session.
- Mission preparation/control and ordinary telemetry are still primary-focused. Backup is not yet an equal replacement for map telemetry, mission upload, or control feedback.
- A release action accepted for local transmission is not proof of delivery or physical release; Boat ACK may be delayed by LoRa airtime and its existing queue.
- E22 on-air time, RF settings and legal duty-cycle limits are unverified. Do not change radio audit flags based on this source update.
- PlatformIO and host regression must be rerun after these changes.

## Next development milestones
1. Add symmetric authenticated session establishment and rekey over either radio.
2. Add bounded LoRa mission/telemetry/ACK queues, prioritizing STOP and STOP STATUS, with retry and duplicate suppression.
3. Measure real LoRa latency/airtime; design a separate low-rate manual control lease only if measured timings and duty cycle permit it. Otherwise retain STOP/RTH/mission autonomy instead of claiming manual parity.
4. Add host integration tests for failover, cold boot, session rekey, release dedupe and delayed ACK; confirm both PlatformIO target builds.
