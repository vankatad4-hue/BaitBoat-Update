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
- Last packaged snapshot: Remote ver024 / Boat ver014. Current source is Remote ver027. Boat ver014 ZIP checksum and structure independently verified; Remote ver024 binary remains unverified. Older Boat archives ver011–013 are superseded.
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


## Remote ver027 — persistent waypoints and map markers
- Waypoint/home/BAIT-HOOK plan flags stored in NVS Preferences with CRC32 integrity and coordinate validation.
- Relative waypoint/home markers added to the main map placeholder; only nearby markers within ±150m are shown and GPS invalid hides them.
- This is a relative visualization, not a true map or safe navigation display. Mission execution is disabled.
- Remote source marker ver026; package rebuild/checksum and firmware compile/hardware validation not yet verified.

- Hardware verification checklist: `Remote/docs/MISSION_NVS_TEST_PLAN_VER026.md` (not yet run).


## Remote ver027 update
- Waypoint capacity increased to 16 (NVS) plus Home. SD/microSD card is designated for offline map packs. Existing map tile reader path documented; no renderer/map pack generator yet. Target compile, SD-card test and NVS test remain pending.


## Updated checkpoint summary — 2026-11-20
The latest UI development ver029.9–ver029.13 is recorded in Remote consolidated changelog VER021 and `Remote/docs/UI_BUILD_VER029_9.md`. It includes a Bulgarian main UI, mission planner 16 slots, calibration/settings/map-manager polish and a static DEMO echogram; none is LVGL Pro-rendered or target-compiled. The current canonical restart document is `CONTINUATION_CHECKPOINT_VER001.md`.

Archive correction: the former packaging workflow regenerated version-named ZIPs during routine pushes. Existing branch ZIPs must be treated as unverified snapshots. Guarded workflow run #880 succeeded, but this does not restore historical release integrity. Documentation sync subsequently triggered runs #888 and #889; both succeeded, with #889 latest. A clean release-marker bump/package/hash/inspection is required before issuing a release.


## Source consistency & compile-preparation audit R1 — 2026-11-20
- Source consistency findings and blockers from this review pass are documented in `docs/SOURCE_CONSISTENCY_AUDIT_R1.md`.
- Found a session-handshake reliability issue: startup marker and reply could be lost without retry. Remote now retries each configured link until per-link confirmation (primary 500 ms, E22 3 s and duty-limited); Boat resends same-session response idempotently. Source changes committed, not compiled or integration-tested.
- No reproducible target build manifest/dependency lock is present. Remote requires LVGL 9.6, but exact display library versions are not pinned.
- Remote host-test workflow is missing; Boat portable tests do not exercise full dual-radio port/session/ACK integration.
- Boat sketch remains a placeholder and does not instantiate the dual-radio port, executor, telemetry, interlocks, watchdog or actuators.
- Authentication provisioning is intentionally disabled/fail-closed; session retry is dormant until keys are provisioned.
- E22 duty accounting is estimated and RAM-only, so reboot reset and actual airtime/regulatory verification remain open.
- ONLINE map source is a preference only; no renderer/provider is implemented and availability returns false.
- This audit does not claim a build or test run. Target compilation is the next gate after exact hardware and dependencies are pinned.


## Remote host-test CI added — 2026-11-20
- Added `.github/workflows/remote-host-tests.yml` to compile/run release guard, physical button scanner syntax, payload codec, ACK tracker and telemetry tests with strict compiler warnings.
- Workflow trigger is branch push and PR to main on Remote source/test changes.
- First CI outcome not yet observed. This workflow does not compile the complete ESP32/LVGL firmware.
