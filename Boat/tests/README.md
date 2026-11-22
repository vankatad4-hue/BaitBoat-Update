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


## R79 low-boat-battery failsafe

- `boat_battery_policy_test.cpp` checks 3S voltage estimation, green/yellow/red bands, the 3-second low-battery RTH latch, stale-sample fail-closed behavior and critical STOP.
- `mission_link_loss_test.cpp` verifies low battery holds outbound motion and permits only validated Home diversion.
- `boat_application_low_battery_test.cpp` exercises the production application bridge with a simulated battery sample and dummy output; it checks Home-only navigation with all release outputs off.
- Thresholds are commissioning defaults and require calibration with the actual pack and divider.


## R79/R20 additional tests

- `boat_battery_policy_test.cpp`: 3S estimate, filtering, warning bands, RTH latch, critical STOP and stale-sample fail-closed.
- `boat_application_low_battery_test.cpp`: mission hold/diversion and critical-stop application bridge using simulated telemetry.
- `ekm014_mcp23017_adapter_test.cpp`: MCP23017 GPA0–GPA3 one-hot EKM014 logical output, bank/address mapping, readback mismatch and fail-closed return semantics.
- `mcp23017_direction_adapter_test.cpp`: R20 MCP23017 0x27 GPB0/GPB1 direction mapping.
- `test_r65_pin_resources.py` now validates the R20 candidate crosswalk and unsigned status; the historical R65 crosswalk is not procurement authority.

## R21 battery ADC full-charge/invalid-readings regression

- `tracker_sensors_test.cpp` now verifies the corrected 150kΩ/33kΩ shared divider value reconstructs a healthy 12.6V full pack without crossing the 3.0V ADS1115 code sanity limit; invalid code is rejected.
- `boat_battery_policy_test.cpp` verifies the **first** invalid reading stops voltage authorization immediately; 1-second data-age invalidation is additional protection. Raw 9.9V critical STOP remains a separate immediate action.
- These are host-only tests. Hardware ADC power sequencing and actual DMM measurements are explicitly **NOT_RUN**.

### R22 separate ADS1115 sensor-only bring-up

- `boat_ads1115_bench_math_test.cpp` verifies 3S full-charge and 10.8V thresholds with the R21 150kΩ/33kΩ ratio, plus zero/negative raw counts, bad pack voltage and ADC input above the firmware's 3.0V sanity bound.
- `../tools/verify_ads1115_bench_isolation_r22.py` verifies sensor-only sketch GPIO audit lock and disallows actuator/radio APIs.
- CI also builds the locked standalone ESP32-S3 Arduino sketch. A successful compiler test is not a DMM calibration, input back-power review or live sensor acceptance.

## R23 ESP32-S3 candidate GPIO and 64-module-net contract

- `boat_r23_gpio_contract_test.cpp`: all 18 ESP GPIO placements match R20/DevKitC-1 J1/J3 candidates and existing nRF/E22 macros; no collisions or use of reserved 1.8V/memory/strapping/USB pins; MCU safety/actuator flags remain false.
- `../tools/verify_gpio_connections_r23.py`: verifies each of 64 `BOAT_R23_MODULE_CONNECTION_MATRIX.csv` rows against the complete 44-position R20 header crosswalk and disallows any active EKM014, MDD20A or fabricated physical-safety wiring.
- Historical R68/R68A regulator and adapter snapshots are retained, but their purchase-register consistency checks now require the **current R23** nRF D24V10F3 and sensor D24V10F3 rail selection with E22 on 5V main and Ardboard socket optional, not the superseded E22 3.3V socket power plan.
- CI result is a **source / document consistency test only**, not electrical pinout/level certification.
