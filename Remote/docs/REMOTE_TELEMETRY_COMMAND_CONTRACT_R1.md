# Remote/Boat Telemetry and Command Contract R2

Status: design contract for integration; **not implemented or tested**. Both firmware sides must implement the same definitions before real control is enabled.

## Frame envelope
- 32-byte fixed `RadioFrame`; version 1; authenticated HMAC; CRC is error detection only.
- Source/destination IDs and session epoch are validated before payload dispatch.
- Sequence freshness is tracked per authenticated session and per transport, with modular rollover handling.
- `commandId` is currently 16-bit on-wire even though `BoatCommand.commandId` is 32-bit. R1 must explicitly use a 16-bit wire action ID and never assume the full 32-bit value survives encoding.
- Sender-local `tick16` is not compared against the other device's unsynchronised clock. Freshness is based on local receipt time, session/sequence, and local watchdogs.
- `flags` is reserved in R1 and must be zero unless a field is explicitly assigned below.

## Payload rules
Each frame has eight payload bytes. The first byte identifies a payload subtype for TELEMETRY and ACK. Remaining fields are little-endian. Reject unknown subtype, invalid enum, reserved nonzero bits, invalid ranges and stale sessions. Never infer safety from missing/invalid fields.

### HEARTBEAT
- `payload[0]`: sender role/status bitfield; R1: bit0 = application alive, bit1 = primary radio available, bit2 = backup radio available, bit3 = STOP latched; bits4–7 reserved zero.
- `payload[1]`: protocol/application build major.
- `payload[2..7]`: reserved zero for R1.
Heartbeat alone does **not** confirm command execution or actuator safety.

### ACK payload subtype 0 — command result
- `payload[0]` = 0.
- `payload[1]` = result: 0 accepted/queued, 1 executed, 2 rejected/safety gate, 3 invalid command, 4 stale, 5 duplicate (already handled), 6 busy, 7 internal failure.
- `payload[2]` = command type echoed.
- `payload[3]` = command ID low byte.
- `payload[4]` = command ID high byte.
- `payload[5]` = boat STOP-latch state: 0 released, 1 latched, 2 unknown.
- `payload[6]` = actuator interlock summary: bit0 all release interlocks healthy, bit1 motors enabled, bit2 GNSS valid, bit3 heading valid; upper bits reserved zero.
- `payload[7]` = reserved zero.
ACK is correlated to the matching command ID/type and session. ACK result=executed is not enough to claim actuator physical movement; telemetry feedback must support that claim.

### TELEMETRY subtype 1 — boat status
- `payload[0]` = 1.
- `payload[1]` = mode: 0 manual, 1 autopilot, 2 return-home, 3 failsafe, 4 calibration; others invalid.
- `payload[2]` = STOP state: 0 released, 1 latched, 2 unknown.
- `payload[3]` = safety flags: bit0 release interlocks ready, bit1 left motor enabled, bit2 right motor enabled, bit3 GNSS valid, bit4 heading valid, bit5 low battery, bit6 failsafe active; bit7 reserved zero.
- `payload[4]` = battery percent 0–100; 255 unknown.
- `payload[5]` = GNSS fix type.
- `payload[6]` = satellites 0–255.
- `payload[7]` = reserved zero.

### TELEMETRY subtype 2 — battery and speed
- `payload[0]` = 2.
- `payload[1..2]` = battery millivolts, unsigned little-endian; 0xFFFF unknown.
- `payload[3..4]` = speed cm/s, unsigned little-endian; 0xFFFF unknown.
- `payload[5..7]` = reserved zero.

### TELEMETRY subtype 3 — GNSS latitude
- `payload[0]` = 3.
- `payload[1]` = coordinate sample counter (modulo 256).
- `payload[2..5]` = signed 32-bit latitude in units of 1e-7 degrees; valid −900000000..900000000.
- `payload[6..7]` = horizontal accuracy in metres ×100, unsigned; 0xFFFF unknown.

### TELEMETRY subtype 4 — GNSS longitude
- `payload[0]` = 4.
- `payload[1]` = same coordinate sample counter as subtype 3.
- `payload[2..5]` = signed 32-bit longitude in units of 1e-7 degrees; valid −1800000000..1800000000.
- `payload[6..7]` = reserved zero.
Latitude and longitude records must be paired by sample counter; discard mismatched or stale pairs. This retains roughly centimetre-level numeric resolution (actual GNSS accuracy is much worse) without truncating coordinate ranges.

### TELEMETRY subtype 5 — heading and navigation
- `payload[0]` = 5.
- `payload[1..2]` = heading centidegrees 0..35999; 0xFFFF unknown.
- `payload[3..4]` = speed cm/s; 0xFFFF unknown.
- `payload[5]` = navigation status enum: 0 idle, 1 route active, 2 waypoint arrival, 3 return-home active, 4 navigation degraded, 5 failsafe.
- `payload[6]` = waypoint/route index.
- `payload[7]` = sample counter.

### TELEMETRY subtype 6 — sonar summary (not echogram)
- `payload[0]` = 6.
- `payload[1..2]` = depth cm; 0xFFFF unknown.
- `payload[3..4]` = sonar range cm; 0xFFFF unknown.
- `payload[5]` = sonar health: 0 unknown, 1 healthy, 2 degraded, 3 fault.
- `payload[6]` = alarm flags: bit0 fish/alarm indication, bit1 shallow-water alarm, bit2 bottom-lock indication, bit3 reserved for future; bits4–7 reserved zero.
- `payload[7]` = sample counter.
This low-rate summary is not raw CHIRP data and cannot render a fish-arch echogram.

## Command lifecycle
1. Remote allocates a command ID; action commands are not repeated under a new ID during transport retry/failover.
2. Remote sends only when the link manager policy allows that command/link.
3. Boat authenticates and validates the frame, session, sequence, command, mode, interlocks and age/queue rules.
4. Boat returns ACK subtype 0 with explicit result and command ID.
5. Remote displays pending/result/timeout and does not optimistically update STOP, mode or release state.
6. Duplicate IDs return DUPLICATE/already-handled result without executing again.
7. STOP is idempotent, highest priority and handled independently of normal command queue.
8. Clear-STOP, mission start, waypoint edit and release require primary authenticated link; backup link must not authorize them.
9. Manual steering is replaceable/latest-value control, expires on boat-side watchdog, and backup steering is limited/capped and airtime-controlled.

## Freshness and safety
- UI data must carry a local receive timestamp per field group. GNSS, battery, heading, interlock and STOP freshness limits are defined by the safety requirements and implemented separately; expired values become UNKNOWN.
- Release guard requires primary link, recent confirmed STOP-released telemetry, and all interlocks fresh/healthy. A stale positive value is not valid.
- Link state cannot be declared healthy from a frame that is merely CRC-valid; it must be authenticated, correct peer/session and accepted sequence.
- The boat independently enforces motor command timeout, STOP latch, actuator interlocks, geofence/GNSS/heading checks and failsafe.
- No control operation is enabled by this document. All hardware pins and actuator integration remain unverified.

## Implementation notes / blockers
- This document is a proposal. The payload formats need cross-review for bandwidth, precision and reserved bytes before source implementation.
- Because 8-byte payloads are tight, telemetry must be sent as grouped frames. Coordinate precision above is deliberately inadequate for precise waypoint arrival; design a multi-frame higher-precision coordinate/waypoint record before implementing autopilot.
- Command IDs are currently truncated to 16 bits by the codec. The dedupe cache must use session + 16-bit ID and correctly handle wrap; do not compare IDs as ordinary signed integers.
- Implement common protocol definitions in shared code and add unit tests for each decoder and malformed/unknown payload.


## R2 cross-review correction
- Removed the duplicated sonar subsection that incorrectly repeated subtype 6 with subtype 5 values.
- Subtype 5 is heading/navigation; subtype 6 is sonar summary only.
- A portable payload decoder has been added to both source trees and host-tested. The decoder is not yet connected to the radio receive path or UI/ACK state.
- HEARTBEAT wire payload remains inconsistent with the current sender scaffold; sender implementation must be updated to use the documented flags/build-major fields before integration.
