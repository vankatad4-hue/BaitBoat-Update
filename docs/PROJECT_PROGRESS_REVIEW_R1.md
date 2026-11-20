# Flytec V080 Project Progress Review R1

**Review date:** 2026-11-20 (updated after cross-radio replay-policy milestone)  
**Branch:** hardware-ready-dual-radio-r1  
**Overall classification:** design/development scaffold; not hardware-ready or operational.

## Progress confirmed from repository and release records

### Remote
- 4.3-inch Waveshare ESP32-S3 touchscreen concept, two joysticks, two 2×4 button banks, separate OK and single Power key.
- Physical scanner, joystick acquisition/calibration, LVGL screens and command safeguards are partially integrated.
- nRF24 primary and E22 LoRa backup transport scaffolds; packet validation/authentication framework.
- Remote-side ACK tracker, authenticated receive dispatch and partial telemetry freshness/safety state application.
- R6.1 parametric enclosure/grips/battery cradle and fit coupons are draft assets; component fit remains unverified.

### Boat
- nRF24 and E22 transports, command decoder, bounded queue and duplicate-action cache.
- Fail-closed key-store scaffold; no provisioned key.
- Added portable heartbeat/ACK/telemetry payload encoders with host tests.
- Added remote-session-epoch echo, per-radio sequence guards, cross-radio filtering, pending-command deduplication and queue-level ACCEPTED/DUPLICATE/BUSY ACK scaffolding.
- ACCEPTED is queue acceptance only. There is no connected command executor, telemetry source, EXECUTED ACK, actuator output or navigation/failsafe application.

## Release/archive review
- Current markers: Remote ver024 / Boat ver014. Boat ver014 ZIP checksum and structure independently verified; Remote ver024 binary remains unverified. Older Boat archives ver011–013 are superseded.
- Generated ZIPs and manifest are present for Remote ver024 / Boat ver014 snapshots. Boat ver014 archive is independently verified.
- Remote ver024 ZIP binary was not retrievable and remains unverified.
- Remote ver024 ZIP: present, but the binary could not be retrieved through the available file fetch path; checksum is manifest-only and not independently verified.
- Archives are source snapshots, not fabrication or operational approvals.

## Decisions to retain
- nRF24 PA/LNA is primary; matching E22-900T22S UART LoRa is backup.
- LoRa must remain duty-limited and restricted; no releases/mission start on backup.
- Physical release controls remain four independent channels; no combined bait/hook release.
- Keep key provisioning disabled until secure provisioning/session/replay design is reviewed.
- No motor/solenoid power or field operation.

## Critical blockers
1. Boat/Remote firmware not fully compiled against the exact ESP32/Waveshare/Heltec board libraries.
2. Boat radio queue and ACK/session code remains not host-tested. New portable policy/dedupe/codec/sequence test files are committed, but tests could not be executed in this environment.
3. No secure session challenge/response or replay protection across reboot.
4. No Boat application loop/command executor/interlock/watchdog/EXECUTED ACK.
5. No Boat telemetry source (battery, STOP, GPS, heading, navigation, sonar) or Remote warning/status integration completed.
6. Exact Heltec and Waveshare revisions/pin maps are not confirmed; radio GPIOs remain disabled.
7. Native KiCad schematics, ERC, verified footprints, PCB layout and DRC are not completed.
8. Power path/charger/battery compatibility and converter thermal design remain unverified; Remote's proposed 1S4P pack needs special review against the Waveshare board's charging guidance.
9. Motor startup/stall current, motor driver fit, solenoid current/duty/flyback and thermal measurements are missing.
10. RF range, antenna placement, legal EIRP/duty-cycle, HIL and controlled-water testing remain unperformed.

## Recommended next work sequence

### Gate A — Software protocol tests (in progress)
- Portable host tests now cover command backup policy, throttle clamping, command dedupe cache, failsafe truth table, sequence wrap/stale rejection, and cross-radio duplicate classification. They are committed but not executed.
- Remaining: fake-radio integration tests around BoatDualRadioPort for session markers, same command crossing radios, stale epochs, queue-full BUSY, and ACK wire format.
- Test ACK frame envelope/HMAC/CRC plus payload round-trip; prove that ACCEPTED never becomes execution state.
- Test failsafe and command policy with a portable state model, including STOP, clear-STOP, manual, return-home, and independent release interlocks.
- Review session recovery/replay behavior. Prefer a challenge/response session protocol before enabling any key.

### Gate B — Application scaffolding (outputs still disabled)
- Create Boat application state model and telemetry publisher.
- Integrate the receive queue with a fail-closed policy executor.
- Establish an ACK lifecycle: ACCEPTED → EXECUTED or REJECTED, with timeout/failure handling.
- Add watchdog and explicit safe reset/link-loss output states.
- Continue host tests; do not map outputs to real pins yet.

### Gate C — Hardware identification and electrical capture
- Obtain and identify exact controller/display revisions.
- Complete pin crosswalk/continuity and onboard-resource audit.
- Capture native schematics, verify part footprints, run ERC.
- Resolve motor driver placement, solenoid drivers, fuses, wire/connector ratings, battery/charger compatibility and thermal calculations.
- PCB layout and DRC only after netlist and pin freeze.

### Gate D — Bench bring-up
- Current-limited logic-only power; verify rails and transient behavior.
- Radio-only tests with antennas attached; key/session and packet logs.
- Sensor and telemetry tests.
- Separate dummy-load motor-driver and solenoid tests, with no bait mechanisms connected.
- Safety review, then HIL; controlled water tests only after independent failsafe validation.

## Latest engineering findings
- Added a portable fail-closed Boat command policy: boot STOP latched; clear STOP, mode transitions, link-specific freshness, backup steering cap and independent release interlocks are evaluated without touching hardware. Test source added to CI; no passing run has been observed yet.
- Duplicate payload encoder definitions were identified during review of the host-test link command and removed. A CI workflow was added for the portable host tests; its pass/fail has not yet been observed.
- The Boat port previously declared but did not define pending-command lookup and ACK sender; these definitions are now implemented. The queue was extracted into a portable four-entry utility and the port delegates to it. This has not been compiled.
- Cross-radio command sequence bypass was narrowed to exact pending/executed command IDs only; stale sequence/new ID is rejected.
- Initial checksum review used an incomplete SHA-256 script; corrected implementation passed the standard `abc` test vector and confirmed ver012 ZIP checksum matches manifest.
- Previous cross-radio logic let CONTROL/ACTION frames bypass the cross-radio sequence guard entirely. This was narrowed: sequence-rejected commands now pass only if exact command ID is pending or executed; stale sequence/new ID is discarded. This reduces replay risk but is not complete anti-replay protection.\n\n## Stop conditions
No propulsion, release, live battery charging in the proposed enclosure, or water operation until the exact hardware is identified, secure key/session is implemented, command executor/interlocks/failsafe are tested, electrical design is reviewed, and firmware/hardware test evidence is recorded.


## Remote touchscreen/input progress — ver024
- Main footer has touchscreen MANUAL, AUTOPILOT, RETURN HOME request buttons.
- Live stick diagnostics now expose ADC failure, calibration-required raw values, or normalized stick values and neutral/active state.
- Safety status strip summarizes STOP confirmation, failsafe/low-battery/interlock and pending command ACK count.
- Stale/absent battery percentage is rendered unknown.
- These are command request/UI feedback paths only. Boat-side execution is not integrated; authentication key is unprovisioned. Target compile/hardware validation pending.


## Remote mission UI — ver025
- Added MISSION touchscreen page with four waypoint slots, save/select/clear, set HOME and planned BAIT/HOOK markers.
- Waypoint and HOME save require fresh valid GNSS (2-second age limit). Data is RAM-only and clears on reboot.
- START MISSION remains blocked; this screen does not transmit waypoint/release/autopilot commands.
- Mission-screen preview committed. Missing ver024 footer status-label declarations were corrected.
- Build and hardware tests pending; ZIP/package output has not been independently verified.
