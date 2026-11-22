# R30F — Authenticated Boat → Remote diagnostic telemetry (subtype 8)

**Date:** 2026-11-22  
**Status:** paired firmware protocol integration and host/target verification; **NOT A CONTROL PERMISSION**. No new external modules, separate actuator batteries, custom PCBs or sensor selections. The owner-confirmed **one 3S4P pack** still supplies MDD20A VM and EKM014 VH through the future common bus.

## Why a new subtype instead of reusing STOP/STATUS

The legacy `RADIO_MSG_TELEMETRY` message contains exactly 8 application payload bytes inside a **32-byte** `RadioFrame` with HMAC/session/sequence validation. Existing telemetry subtypes 1–7 represent STATUS, BATTERY, latitude/longitude, navigation, sonar summary and weather. R30E's **D5** frame was local USB diagnostic data, NOT a valid authenticated RF payload.

R30F adds an **optional subtype 8**, using the existing authenticated primary nRF `RADIO_MSG_TELEMETRY`. It does **not** renumber any old subtype, change `RadioFrame` size, change `RADIO_PROTOCOL_VERSION=1` or introduce an actuator control message. The Boat's existing `sendHardwareTelemetry` requires a provisioned authenticated session and a currently fresh primary nRF link; it sends only on `LINK_NRF_PRIMARY`. There is **no** backup-LoRa diagnostic transmit path.

## Exact 8-byte wire payload, little-endian

| Byte | Field | Contract |
|---|---|---|
| 0 | Subtype | Exactly `0x08` |
| 1 | Sub-version | Exactly `0x01` |
| 2 | R30E reason | 1–14; 1 = NO_READER, 14 = UNCOMMISSIONED |
| 3–4 | Flags LE16 | Only bits 0–11; **UNVERIFIED 0x0002** and **COMMISSIONING_LOCK 0x0800** mandatory |
| 5–6 | Voltage LE16 | `0xFFFF` if no fresh observation; 9000–13000 mV with observed flag |
| 7 | Sequence | 8-bit diagnostic sequence for display/replay analysis, **not** authentication |

`radio_safety_diagnostic_v1.h` is byte-for-byte identical on Boat and Remote; CI compares both. Decoding rejects missing commissioning lock, missing UNVERIFIED, unknown fault reasons, unsupported flags/version, a missing voltage sentinel or an unobserved message claiming plausible live battery voltage. Diagnostic reason codes 1–7 are unobserved; 8–14 require observed flag. **Observed never means commissioned.** Authentication is provided exclusively by the enclosing `RadioFrame` HMAC/session plus existing replay guards — the diagnostic 8-bit sequence is not an authentication token.

Examples:
- **No reader installed:** `08 01 01 02 0A FF FF 07` — NO_READER, UNVERIFIED, LOCK, source fault, unavailable mV.
- **Synthetic uncommissioned observation:** `08 01 0E 03 08 E0 2E 08` — reason 14, observed, UNVERIFIED, LOCK, 12000 mV. This remains **unfit for live actuator safety confirmation**.

Older Remote decoders ignore unknown subtype 8. Mixed-firmware compatibility is therefore non-destructive, but the diagnostic will appear only after both sides are updated.

## Boat integration

`Boat/source/firmware/Flytec_Boat.ino` continues to emit local `R30E,LOCAL_ONLY,...` USB diagnostic records every 2 s, retaining the separate D5 frame. In addition it creates a `RadioDecodedPayload` kind `RADIO_PAYLOAD_SAFETY_DIAGNOSTIC` and passes that to existing authenticated `radio.sendHardwareTelemetry`, which can refuse transmission while radio GPIO/key/handshake is uncommissioned.

The Boat presently constructs an EMPTY R30D observer, so it reports **NO_READER, UNVERIFIED, COMMISSIONING_LOCK**, never a measured healthy hardware contact. It attaches **NO physical Boat actuator HAL** and all INA231/current-zero/common-disconnect safety commissioning gates remain false. This is a firmware path, not evidence an RF packet was sent over real radios.

## Remote integration

`Remote/source/radio_payload_codec.cpp` validates subtype 8 only as a normal telemetry **after** `Remote/source/boat_radio.cpp` performs the standard authenticated frame and sequence checks. The Remote accepts only `LINK_NRF_PRIMARY`; backup telemetry is denied. Additional replay/freshness safeguard: a successfully parsed subtype 8 **does not call `g_linkManager.reportValidRx`** and thus cannot prolong primary-radio *control* freshness. It is not an ACK, command, mission feedback or STOP confirmation.

`Remote/source/boat_telemetry.cpp` stores fault code, flags, sequence, optional mV, and local receive timestamp in separate **read-only** fields. It deliberately does not touch `statusReceivedAtMs`, normal battery telemetry/percent, motor flags, `emergencyStopConfirmed`, mission state, `actuatorSafetyReady` or general received-link timestamp. The indicator expires after **6000 ms**, and also hides on loss of primary authenticated link. Resetting the authenticated session resets its diagnostic state.

`Remote/source/boat_ui.cpp` shows a separate Bulgarian footer line (never replacing the red STOP/low-battery alert): `ДИАГН.: НЕПОТВ. / НЯМА ЧЕТЕЦ` or `ДИАГН.: НЯМА ДАННИ (НЕПОТВ.)`. Hardware failure reasons may be colored red, but **no diagnostic status is ever painted as a green operational safety approval**.

## Automated negative tests and exact boundaries

- `Boat/tests/r30f_optional_safety_diag_codec_test.cpp` and `Remote/tests/r30f_optional_safety_diag_codec_test.cpp`: strict 8-byte golden vectors, valid roundtrips, reject missing commissioning and unverification flags, old version, invalid reason, reserved bits, mismatched voltage/observed, and confirm legacy STATUS/BATTERY stay byte-compatible.
- `Remote/tests/r30f_diagnostic_telemetry_isolation_test.cpp`: primary-only telemetry receipt, backup rejection, stale 6-second expiry, no normal status/battery/link timestamp change, no STOP ACK or motor safety grant from diagnostic, session reset clears observations.
- `Boat/tools/verify_r30f_diagnostic_protocol.py`: identical shared decoder contract on both endpoints, authenticated transmit/receive scaffolds, *no control freshness refresh*, uncommissioned physical safety flags still false and R30A's 24 physical gates still OPEN.
- Existing Boat/Remote host and ESP32 target workflows must all pass independently before R30F is marked software-verified. Physical RF reception, range, provisioning and fault-wire acceptance still require purchased modules and supervised testing.
- The 73-row R30A wiring list stays unsigned, physical STOP/watchdog/common DC cutoff not installed, independent coil pulse/current-zero unproven; 81 older physical acceptance rows remain NOT_RUN. Versions **Boat VER014 / Remote VER028** remain unchanged; CHIRP sonar deferred.

**Next action:** run actual Boat and Remote codec/telemetry host CI, compile both ESP32-S3 target programs, fix all errors and record exact passing run IDs in the continuation checkpoint. Do not treat a green software build as permission to connect motor or solenoid load power.

## R30F confirmed verification — 2026-11-22

Actual GitHub Actions results were inspected after each job completed:

- **[Boat host #45048750474](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048750474): SUCCESS.** R30F strict subtype-8 mirrored wire-vector tests, static Boat/Remote security contract and all other Boat portable host tests passed.
- **[Remote host #45048847021](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048847021): SUCCESS.** R30F paired codec golden vectors, invalid/forged-commissioned packet rejection and telemetry-isolation test passed, including a test that diagnosis cannot clear an already confirmed STOP. All other Remote portable host tests passed.
- **[Boat ESP32-S3 target #45048442529](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048442529): SUCCESS.** Diagnostic-only Boat target includes R30F optional authenticated send route, plus locked INA231 and historical locked ADS1115 benchmarks. This is a compilation test, **not** real RF transmission or actuator work.
- **[Remote Waveshare ESP32-S3 4.3-inch target #45048618202](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45048618202): SUCCESS.** Complete Remote LVGL/SD/UI/radio target compiled, **1,460,005 / 3,145,728 program bytes (46%)**, **179,512 / 327,680 global dynamic RAM bytes (54%)**, 148,168 bytes reported available for local variables. This is compile output, not a measured runtime memory budget or physical board acceptance.

Intermediate early R30F CI runs failed because their respective commit snapshots invoked `verify_r30f_diagnostic_protocol.py` before that file and its linked R30F document were committed; both strict C++ wire codecs already passed in those runs. The later complete host runs above are **SUCCESS**. No command type or radio-frame authentication contract was changed.

**Status at R30F:** Boat and Remote software paths, strict codecs, host tests and ESP32-S3 builds are verified. Physically uninstalled and unverified STOP/watchdog, shared DC cutoff, independent one-hot coil limiter/current-zero sensors and actual RF range remain **outstanding**. Both endpoints retain all motor/coil STOP locks; R30A 24 physical gates OPEN and prior 81 physical tests NOT_RUN. Single shared 3S pack, no separately switched motor/coil domains, no new module selection. Boat VER014 / Remote VER028 unchanged.

**Next engineering step:** R30G — review rate limiting and bounded-queue/backpressure behavior of diagnostic packets during manual control and deliberate link loss, then regression-test no control-freshness or priority inversion. Physical radio acceptance and motor power remain blocked.

## R30G transport scheduling amendment — supersedes immediate send assumption

R30F's type-8 wire bytes, authenticated 32-byte frame and Remote decoding/UI remain unchanged. Effective Boat TX policy is now [R30G one-slot optional scheduling](BOAT_R30G_RADIO_DIAGNOSTIC_PRIORITY.md). `BoatDualRadioPort::sendHardwareTelemetry(RADIO_PAYLOAD_SAFETY_DIAGNOSTIC)` returns **true when a replaceable one-slot offer is accepted**, **not** when the RF packet is delivered or ACKed. `BoatDualRadioPort::update()` transmits only after mandatory control, mission and heartbeat work, with short expiry and no retries. R30F safety-field semantics (`UNVERIFIED|COMMISSIONING_LOCK`) and independent physical STOP requirement remain unchanged.
