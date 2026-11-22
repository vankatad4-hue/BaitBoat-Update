# Boat host-side tests

These tests use a host C++ compiler and small Arduino/frame stubs. They do **not** validate cryptography or radio hardware.

## Payload codec/encoder

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror Boat/source/radio_payload_encoder.cpp Boat/source/radio_payload_codec.cpp Boat/tests/radio_payload_encoder_test.cpp -o /tmp/boat_payload_test
/tmp/boat_payload_test
```

## Command deduplication and failsafe decision logic

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/boat_radio_safety.cpp Boat/tests/boat_radio_safety_test.cpp -o /tmp/boat_safety_test
/tmp/boat_safety_test
```

## Command codec and backup-link policy

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/radio_command_codec.cpp Boat/tests/radio_command_codec_test.cpp -o /tmp/boat_codec_test
/tmp/boat_codec_test
```

## Sequence guard

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/radio_command_codec.cpp Boat/tests/radio_sequence_guard_test.cpp -o /tmp/boat_sequence_test
/tmp/boat_sequence_test
```

Command-codec and sequence-guard tests supply stub frame-authentication/freshness functions. They validate policy and sequence logic only, not HMAC/CRC or physical links. Tests cover backup restrictions, throttle clamp, command-ID dedupe scope/cache behavior, cross-radio replay classification, wraparound and stale/out-of-order sequence rejection, and link-failsafe decision logic. They do not test the BoatDualRadioPort session/queue integration or actuators.

## Bounded command queue

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/boat_command_queue.cpp Boat/tests/boat_command_queue_test.cpp -o /tmp/boat_queue_test
/tmp/boat_queue_test
```

## Fail-closed command policy

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -I Boat/tests/stubs Boat/source/boat_command_policy.cpp Boat/tests/boat_command_policy_test.cpp -o /tmp/boat_policy_test
/tmp/boat_policy_test
```

This is a portable decision model only; it does not actuate hardware.

## R15 module-only checks
```sh
python3 Boat/tools/module_only_policy_r15.py
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror Boat/tests/module_only_output_r15_test.cpp -o /tmp/module_only_r15
/tmp/module_only_r15
```
The new `module-only-hardware-r15.yml` workflow runs these tests, separately from archived custom KiCad workflows. This checks manifest/wiring safety conditions and a software-only blocked-by-default module output gate; no physical module was connected.


## R77 dual-radio watchdog regression

The R77 host tests cover:
- authenticated primary/backup freshness watchdog LOST/RESTORED transitions;
- STOP_ALL on total link loss and no automatic RTH;
- autonomous mission hold/resume when either radio returns;
- backup-only navigation but primary-only irreversible releases;
- abort/no replay for release intent/pulse after primary loss;
- stale command queue purge while retaining emergency STOP;
- neutral-before-manual recovery and all-off actuator requests.

The full set is run by the Boat portable host tests workflow. These are software tests only; they do not prove physical output isolation or an independent hardware safety channel. See ../docs/BOAT_R77_DUAL_RADIO_WATCHDOG_IMPLEMENTATION.md.


R77A also retains the authenticated session across ordinary RF silence so the backup link can restore the same session after a long outage. Remote dual-radio manager tests cover transition from LINK_NO_LINK back to LINK_BACKUP. Sequence-limit rekey and endpoint power-cycle still use the primary handshake.


## R78 dual-radio loss, NVS Home and dummy-load bench preparation

- The Boat stores the authenticated mission Home coordinate in a two-slot, CRC-verified NVS record during mission preparation and reads it back before acknowledging preparation.
- Both radios stale: motor and solenoid requests remain OFF immediately. The 10-second RTH timer is measured from the last accepted authenticated frame.
- At timeout, automatic RTH is allowed only for an eligible active mission with matching valid Boat NVS Home, fresh GNSS/heading, healthy battery, valid geofence and local release/STOP interlocks. No radio never authorizes a release.
- If any gate fails, outputs remain STOP. Link recovery before timeout resumes the held mission; after RTH starts, the old outbound mission is not resumed. Primary-radio manual takeover requires a new explicit request, neutral sticks and a stationary/idle boat.
- boat_home_point_record_test.cpp and boat_home_point_nvs_test.cpp validate record CRC/range handling, two-slot persistence and corruption fallback using a host Preferences emulator; boat_radio_safety_test.cpp, mission_link_loss_test.cpp and boat_hardware_supervisor_test.cpp validate timer/policy/decision logic. CI does not prove actual NVS retention, RF, GPS, motors, coils or independent power cutoff.
- Physical bench procedure: use isolated, current-limited supplies and rated resistive/indicator dummy loads only; leave actual motors and solenoids disconnected. The test plan is ../docs/BOAT_R78_RADIO_LOSS_DUMMY_LOAD_BENCH_TEST.md.
