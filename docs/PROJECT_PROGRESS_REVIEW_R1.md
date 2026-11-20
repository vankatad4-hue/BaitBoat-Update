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
6. Remote display model is user-confirmed as Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU 25948; physical board revision and pin maps remain unconfirmed until receipt. Boat Heltec revision/pins unresolved; radio GPIOs remain disabled.
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

## Latest build preparation — 2026-11-20

- User confirmed the exact Remote target: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948.
- Source compatibility audit added for LVGL 9.6 display adapter.
- Arduino CLI target compile workflow committed with candidate pins. The run is not yet complete; no full Remote target build result is available.
- Remote host CI has a known first-run failure: test fixture omitted fail-closed state fields. Corrected fixture committed, follow-up result pending.

## Remote build preparation update — 2026-11-20

- User-confirmed target is Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948.
- LVGL 9.6 adapter audit and reproducible-build attempt are documented under `Remote/docs/`.
- Remote portable host CI run #11 passes all current host tests.
- Full target compile has reached the source compiler. Initial compiler findings (LVGL 9.6 precise line point type and missing footer-label declarations) were fixed; current target run #11 is pending.
- Dependency pins now include RF24 1.4.11; LVGL configuration is tracked in source.


## Target compile follow-up — 2026-11-20
- Remote target compile workflow run #11 failed at C++ compilation after installing the pinned toolchain/libraries and compiling source modules.
- Confirmed installed versions from compiler output: ESP32 Arduino core 3.0.2, LVGL 9.6.0, ESP32_Display_Panel 1.0.0, ESP32_IO_Expander 1.0.1, esp-lib-utils 0.1.2, RF24 1.4.11.
- Three compile findings were corrected: LVGL event-target API use in joystick calibration; map renderer line-point type for LVGL 9.6; escaped-newline comment accidentally suppressing Map Manager `copyChunk()` declaration.
- Corrective commits: `1afd17d5`, `00840087`, `7213f04e`. Target rebuild pending. Do not claim full compilation.


### Remote target compile #14 follow-up
Target Compile #14 failed due to undeclared module variable `g_map_attribution` in `offline_map.cpp`. Added the missing private 256-byte buffer in commit `9aa20b0d987f94ea5b49e46509ae0ba10cc9d36b`. Compile #15 is the verification run; status pending.


## Latest Remote CI compile — 2026-11-20
Remote target compile run #19 (44781879101) succeeded on commit 3f189cb1ac65a764aec678893f1df857d113c815: compile/link and artifact upload passed. ESP32-S3 16MB/OPI PSRAM with huge_app. Firmware 1,430,957 bytes (45% app); global RAM 178,136 bytes (54%, 149,544 bytes remaining). No C++ errors/warnings reported. This does not establish hardware functionality. Host tests #44779439310 and package run #44779439396 passed on source SHA 2289e84ad73e1931f18f357d0e437fed5e0d6c12. Next audit: settings/Wi-Fi timers, Map Manager import/index validation, renderer cache/PSRAM and LVGL object lifecycle. Boat firmware remains uncompiled and non-operational; ZIP snapshots remain unverified.


## Eight-lake development checkpoint — 2026-11-20
Remote package marker remains 028; Boat 014. Source now has eight SD lake profiles, sixteen mission slots and separate Home per profile, checked active selection, recoverable name and mission records, and A/B offline map installation. Lake selection UI uses standard LVGL keyboard and confirmed STOP/manual/connected-radio/inactive-motor gates. Mission execution remains blocked; boat-side synchronization is not implemented.
Latest observed before this update: host #41 passed (6a3d88c), packaging #977 passed, target #42 running. Host #41 did not exercise the new SD storage. This update adds a mock SD host test compiling actual lake_storage.cpp: eight profiles, safe switching, name persistence, isolated paths, map marker rename failure, short writes and unavailable card. Results pending. The mock does not test actual FAT/SD durability, mission serialization or LVGL. Verify new CI and fix real failures before bumping the release marker. Hardware remains untested.


## Verified test handoff — 2026-11-20
Source SHA 8030ecab6cbccb64204e638b4c398b588681515e. Host #43 failed on missing sdMissionAvailable reference after extraction; corrective commit 8030eca replaced it with lakeStorage_ready. Host #44 passed (run 44793207359), including actual mission serialization for 128 waypoints, per-lake Home and CRC/rename/short-write simulation. Packaging #980 passed (44793207717). Target compile #45 (44793207383) is running; previous #43/#44 were cancelled by newer pushes, not passed. Do not change firmware merely to retrigger CI while the current target run is active.
Next: inspect #45 compiler result and fix actual diagnostic if any; extend coverage/import parser and fault-injection tests, audit remaining source modules in manageable batches (repository inspection is not an exhaustive line-by-line audit), validate UI lifecycle and SD runtime, then bump Remote package marker 028 to 029 only after software gates pass. Boat remains 014 and hardware/actuation unvalidated. Repository project marker docs/PROJECT.md is not a ChatGPT Project. Existing root legacy map and NVS-only legacy data require follow-up; standard keyboard is not a dedicated Bulgarian keyboard.


## Eleven-stage execution handoff — 2026-11-20
**Current firmware source:** 3d363f07c84c24cb12c5ecd7be2c52b5f15e8897; test-only follow-up 7ec7a7a5f313f8d330233d1941ca7ff4ff0ca6e5.
- Remote host #50: PASSED, all ten groups (44795330911). Includes actual map manifest/coverage parsers, projection/pan, 128-waypoint mission storage, matching portable Remote/Boat mission codecs and lifecycle, physical input/OK/release arbitration and lake zoom persistence. Boat portable #15 PASSED (44795188700). ZIP snapshot #986 PASSED (44795188825). Target #50 (44795188918) was still installing pinned ESP32 core; no full compile result yet. Do not push unnecessary firmware changes that cancel the target run.
- 1: #45 escaped duplicate Wi-Fi state declarations fixed (2db6318); complete latest target compilation and remainder of branch audit still open.
- 2: shared strict map manifest and coverage parser/count checks tested (bec5550); mock recovery passed, real SD and runtime/cache/import failure testing outstanding.
- 3: physical waypoint, settings, map, sonar, back and OK actions connected; scanner STOP/fault/release arbitration tested (70c7f9e, 7ec7a7a). Hardware and UI integration validation outstanding.
- 4: mirrored eight-byte candidate mission protocol/assembler (81dd926), host tests passed. Authenticated live dispatch, ACKs, transaction and release-mask UI integration still open.
- 5: non-actuating mission feedback state machine tested. No live autopilot, pause/resume/return execution, accuracy gating or override integration. Non-manual command requests explicitly blocked pending synchronization.
- 6: diagnostic screen and Boat unsynchronized-clock bug corrected (3d363f0). Secure key provisioning, authenticated session/replay redesign and radio bench testing open; key store deliberately fails closed. Boat command execution must enforce local queue age.
- 7: north-up touch pan/recenter, per-lake SD zoom, panned route origin, enlarged 4x3 tile footprint, descriptor fixes and retry cooldown implemented. Boat-up rotation, async tile loads, full preferences and hardware display tests open.
- 8: diagnostics, settings back/timer cleanup, sonar demo point lifetime corrected; Cyrillic font asset/keyboard, full localization and display layout validation still open.
- 9: Remote release remains 028; 029 NOT released. Boat remains 014. Snapshot workflow success is not release certification. Bump only after software gates.
- 10: physical unit not available; boat integration/actuator/range tests not performed.
- 11: sonar module/interface not selected or verified; demo only; no real driver or data.
Next action: read target #50 result/log and correct actual errors, then continue stages in the approved order. Recheck CI on exact new source and update this handoff; no claim that all eleven stages are complete. 
