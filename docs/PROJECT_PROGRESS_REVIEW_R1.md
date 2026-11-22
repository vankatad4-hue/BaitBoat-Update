> **R63 confirmed-endpoint code review (2026-11-22):** Latest engineering status, audited code fixes, verified Boat/Remote host tests and **both endpoint target-build PASS results** are in [R62 endpoint audit](R62_ENDPOINT_CODE_AUDIT_AND_NEXT_GATES.md). Active Boat board **ESP32-S3-DevKitC-1-N32R16V**, Remote **Waveshare ESP32-S3-Touch-LCD-4.3 Standard**; both modules and independent safety hardware remain unpurchased/uncommissioned. Prior R1/Heltec/PCB descriptions below are historical. Current implementation is **not operational**; no changes to official Boat VER014 / Remote VER028.

> **R15 UPDATE:** Historical custom electronic PCB plan superseded by the owner's **premade modules only** decision. Active hardware policy: [R15](HARDWARE_MODULE_ONLY_POLICY_R15.md); Boat and Remote module wiring plans take precedence over this prior R1 review.

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


## Mission preparation transport and navigation continuation — 2026-11-20
- Remote/Boat mirrored `mission_wire.h` and `radio_protocol.h` define dedicated mission PREPARE (8) and ACK (9) messages keeping 32-byte authenticated frames. Boat `boat_dual_radio_port.*` dispatches only primary-link session/replay-checked preparation to a non-actuating assembler with application-owned stationary/STOP gate defaulting to null/deny; Remote `boat_radio.*` drives the upload and accepts matching authenticated ACKs. Both key stores still intentionally deny all TX. Existing 8-bit session epoch is not adequate final replay protection; no secure pairing or hardware certification.
- `mission_ui.cpp` now has preparation-only PLAN action with independent per-upload four-channel selection (all off by default; legacy marks are not mapped), preparation feedback, confirmation, local cancel and edit guards. Prepared plans expire independently; STOP invalidates local/Boat preparation. No mission START or actuator execution enabled. Relevant commits 763221c, 9cb3628, 68cb549, feb84db, f4724be, 6a1ded4.
- Verified locally using strict g++ flags and exact Git blob SHA matches: Remote mission protocol/upload/lifecycle/ACK regression PASS (test 4cdcc547669b7172bf52f7682fd3ede571bcff9a); Release guard PASS (test a2a6662931d0330bc84d1db0402a389d60b994f6); Physical scanner test initially failed at restart OK count because earlier ordinary OK was counted, fixed test baseline f62926985e903a64d8074e370918431804992969 and reran successfully (test blob 712e24efa6615ca190f0fc9472e2ff278f051b77). These are portable tests, not full CI or ESP32 tests.
- Boat `mission_navigation.h` adds pure distance/bearing/heading error and conservative arrival criterion distance + estimated horizontal accuracy <= requested radius (3 m default), requires fresh fix/healthy heading/battery/geofence. It is not a steering controller. Host test registered in Boat CI (6b8b4a3); local C++11 strict run PASSED on exact blobs a260353a7afbdff84e2a9f94ea10024399a45b73 (navigation) and f2004291e2d4119fe244ea24f92b1b1c63b97215 (test), mirrored codec blobs d0077f6727f9ec975d5c2fe95927bab4879999a4 and 651ea8faf239fd41e19f3f3b09668b94be2aae4f.
- Next: retrieve latest push-triggered target compile and full Remote/Boat host CI results, fix diagnostics before more target integration. Local `git ls-remote` fails DNS and Arduino CLI unavailable. No fresh full target evidence (last verified #50); remaining map-copy/loader and LVGL lifecycle tests, authenticated session redesign, Boat application supervisor, mission execution feedback and actual navigation/actuation remain open. Remote 028 / Boat 014 unchanged. No archive regeneration verified. No user design decision required for this software work; build evidence is the immediate blocker.


## Workflow audit and development continuation — 2026-11-20
- Repository-wide Actions run listing now accessible through authenticated GET. Target #74 (44801485451) failed at missing lv_image_cache_drop declaration; commit 0f4c47b added upstream 9.6 header but #75 (44802976345) failed due to Arduino include root already being lvgl/src. Commit 692f959 fixes to <misc/cache/instance/lv_image_cache.h>; #76 (44803266207) running at time of audit. Never treat previous #50 as current.
- Remote host #90 (44803265844, job 136520557058) passed all 12 groups including production SD tile reader/cancellation/short-read/missing-card tests and mission, release, map copy/recovery. Boat host #26 (44803347880, job 136520835967) passed all 8 including new simulation-only execution supervisor. Boat #29 (44803586651, job 136521642320) passed all 9 including actual OpenSSL HMAC domain-separated session transcript and bounded replay counter tests. No physical actuator integration. Session V2 candidate lives in Boat/source/radio_session_v2.h, not integrated; key store remains fail-closed.
- Corrected packaging workflow 74700d2: event SHA checkout, per-side exact +1 checks, immutable existing ZIPs, no rebase onto newer branch; #1044 (44803056623) passed normal snapshot build, and #1048 (44803265820) succeeded with SHA-specific artifacts. Release-publication path not exercised; no bump 028/014. Archive contents/inner checksums not independently downloaded.
- Added separate Bulgarian UI resource generation workflow and script using DejaVu Sans and pinned lv_font_conv 1.5.3. Generated resource not yet linked to LVGL or keyboard and workflow result unverified. Existing default Montserrat font does not cover Cyrillic. Do not claim screen validation.
- Next: confirm #76, inspect new font workflow, integrate tested session handshake and font into both target applications; complete real Boat supervisor/feedback, joystick/UI runtime, SD performance, release verification. Hardware, pinout, secure physical key provisioning and real CHIRP transducer remain external gates. Do not enable actual motor/solenoid execution or bump versions until acceptance criteria are met.


### Target #76 acceptance evidence (supersedes running status above)
- Run 44803266207, job 136520558870, source 692f959fcd76430d21e79c75ddeb183d8be8a8b4: PASSED. Program 1,441,429/3,145,728 bytes; globals 178,968/327,680 bytes; 148,712 bytes remaining. No C++ errors in retrieved logs; toolchain esptool SyntaxWarning and Node deprecation notices remain. Firmware artifact 14662810189 and compiler log artifact 14662560853 uploaded successfully. Remote host #90 and Boat #29 passed 12 and 9 groups respectively; Boat-only additions after source revision do not constitute a new Remote target build. This is compilation evidence only, not hardware certification. Version markers remain 028/014. Next integration gate: session V2 live transport/provisioning, actual Bulgarian font/keyboard integration, execution supervisor wiring and physical acceptance; archive publication remains gated.


### Additional verified workflow results
- Bulgarian font resource #1 (44803930186, job 136522841000): generation and source artifact upload PASSED; not linked or rendered on target. Offline Map Tool Tests #3 (44803930041) PASSED. ZIP snapshot #1053 (44803930029, source bc1ea6fff22e36b60a3e40870864762c0f65982f) PASSED archive creation, integrity tests and upload; artifacts Remote 14662426210, Boat 14662431183, checksum 14662371258. Artifact digests describe outer Actions ZIPs, not inner archive hashes. No release marker bump; immutable publication gate remains untested. Next: wire secure handshake into both endpoints and integrate generated Bulgarian font; verify actual execution supervisor and hardware gates before releasing 029/015.


## Session V2, Bulgarian UI and execution-control continuation — 2026-11-20

- 9ca1af5 integrates generated DejaVu Cyrillic font into the LVGL default theme in target CI, with Montserrat icon fallback, font artifact/license and EN/BG lower/BG upper keyboard layout. Local Arduino build requires generating/copying the font resource per Remote/docs/BULGARIAN_FONT_BUILD_R1.md. Resource workflow #2 (44805083427) and offline map tools #4 (44805083408) passed. Actual rendering/keyboard usability not hardware-tested.
- f249e76 implements mirrored mutual HELLO/CHALLENGE/PROOF/CONFIRM handshake with real OpenSSL HMAC host tests, replay/refused authentication and duplicate-proof confirmation checks. 67e2643 connects both actual adapters to derived session keys, rejects legacy handshake, uses bounded nonwrapping replay counters, resets mission/queue/ACK/telemetry at session loss or rekey, and stops queuing obsolete LoRa joystick commands. 8880fd2 authenticates the physical link flag, rejects copied frames on the wrong link, and adds flag regressions. Primary nRF24 is required to establish new sessions; backup shares an established session only. Secure provisioning reader is guarded by explicit opt-in, encrypted NVS, flash encryption and secure boot; normal target builds fail closed, the opt-in secure SDK path is untested, and no actual key or hardware entropy validation has occurred. See docs/SECURE_PAIR_PROVISIONING_R1.md.
- 0c31980 adds actual non-actuating Boat diagnostic sketch polling the radio and a separate Boat ESP32-S3 target build. Unassigned pins, no mission safety provider or actuator execution; queue acknowledgements are not execution. First Boat target #1 (44805621575) failed missing ManagedLink declaration; f9c6bde corrects it. Remote target #79 (44805520905) failed on LVGL keyboard control enum conversions; 8880fd2 corrects explicit casts. Superseded intermediate builds cancelled. Current Remote target #81 (44806126603) and Boat target #4 (44806126280) are running on 8880fd25b8df9fd84cd3549bc8f8f38d516df1b1; no target pass claimed for new changes yet. Last verified Remote target remains #76.
- c2b565d adds mirrored 8-byte mission-control and feedback codec plus portable BoatMissionController (stage, start, pause, resume, cancel, temporary override neutral confirmation, permanent manual request, duplicate request handling, expiry). Tests are simulation only; no authenticated radio dispatch, physical mode confirmation, manual override motor lease or release safety executor yet. Future live integration must distinguish prepared plan from execution, invalidate on STOP, require current trusted Boat inputs and hardware watchdog, and never infer acknowledged movement from a queued operation.
- Verified latest host CI on 8880fd2: Remote #95 (44806126364) passed 12 groups; Boat #35 (44806126309, job 136530479362) passed 11 groups including actual HMAC, handshake and mission control/override simulation. Packaging #1062 (44806126387) passed snapshot build/upload; archive publication not yet tested, inner hashes not independently checked. Remote 028 / Boat 014 retained; no release bump.
- Continuation: verify both target builds #81/#4 and fix any failures; after compile acceptance wire mission-control request/feedback to authenticated primary radio and UI with a default-deny Boat application provider, add trusted input/STOP and freshness checks and temporary override command lease across actual joystick/Boat executor, then package frozen 029/015 only after acceptance. Hardware and CHIRP remain pending. No physical provisioning or actuator enablement approved or performed.


## QWERTY Bulgarian UI, mission execution radio integration and override lease — 2026-11-20
- 5b638ac and 7cadc20 implement touch-adapted phonetic QWERTY Bulgarian keyboard shared by Settings and lake name editing; lower/upper rows cover all 30 letters, English and symbol/number modes retained. Offline tool tests #5 (44807388840, job 136534858451) passed all 8 including explicit keyboard row and glyph coverage assertions. Actual physical LVGL keyboard usability remains unverified; this is not a BDS compliance claim.
- 215a239 and 8c2377b implement authenticated primary-link mission control, periodic phase feedback and dedicated stick input messages, Remote request/ACK client (bounded retries retaining logical ID; missing ACK unknown), a receiver-local 200 ms override lease (nominal 50 ms Remote samples), neutral rearm before override and resume request after displacement then neutral; physical stationary/STOP and release interlocks still required on Boat. No actual motor output is wired. 7cadc20 adds mission overlay for START/PAUSE/RESUME/CANCEL/temporary override/manual/STOP/clear STOP and guards lake/waypoint changes. d229cb1 requires fresh matching primary telemetry before confirmed manual transition and retains local STOP until fresh confirmation; adds completion token checking and queue clearing at mission start. b1f4926 fixes successful upload handoff and duplicate request side effects/ACK after STOP. 1f0d585 stages Remote client header into canonical shared-wire test directory; cross-peer integration simulates request, lost ACK, override neutral, 200ms expiry and abort. No actual RF or actuator tests; no trusted mission input provider in diagnostic Boat sketch. The live session-reset return-home behavior remains an explicit unresolved requirement: session timeout/rekey currently aborts the supervisor, so prolonged link loss does not yet guarantee autonomous return. Do not enable execution before watchdog, sensor, propulsion, release, session-loss-return and mode telemetry integration are independently verified.
- Previous Remote target #81 (44806126603) cancelled by source update, not a pass; Boat #4 (44806126280) passed 320945/1310720 program bytes, 19968/327680 globals. Boat #5 (44806845960) passed; Boat #6 (44807388836, job 136534858206, d229cb1) passed 333569/1310720 program bytes, 20072/327680 globals. Latest Remote target #87 (44807620544) and Boat #7 (44807620473), both source b1f49260ab29f266d8e0e925209e253465cad162, remain in progress at checkpoint. #76 remains last verified Remote target. Superseded Remote builds cancelled by subsequent firmware changes. No newly observed compile errors in this batch.
- Remote host #101 (44807620465, job 136535625521) passed 13 test groups; Boat host #39 (44807676162, job 136535821192) passed 13 (including the revised cross-peer simulation and stick lease). #39 differs from firmware revision by test/workflow-only commit 1f0d585. ZIP snapshot #1069 (44807620683, source b1f4926) passed creation and upload: Remote artifact 14664446356, Boat 14664526045, checksum 14664511165. These are development snapshots; inner archive contents/checksums not independently downloaded. Remote 028/Boat 014 unchanged; no immutable 029/015 publication until acceptance. 
- Next: verify #87/#7 target completion and diagnostics, reconcile later builds, test secure pairing and real application provider/actuator watchdog with disconnected power, complete autonomous return on prolonged link loss and confirmed manual/mission status paths, verify LVGL/SD real runtime. Release acceptance not complete; no production key provisioning or hardware actuation approved or performed.

### Boat target #7 verification (supersedes running status above)
Boat diagnostic target #7 (44807620473, job 136535626232, firmware source b1f49260ab29f266d8e0e925209e253465cad162) PASSED; program 333597/1310720 bytes, globals 20072/327680 bytes, leaving 307608. Remote #87 still in progress at last check. Release version markers remain unchanged. No actuator hardware or live RF validation claimed.


## Autonomous return-home and local hardware safety bridge — 2026-11-20

- Remote target #87 (44807620544, job 136535790034, Remote firmware source b1f49260ab29f266d8e0e925209e253465cad162) PASSED, superseding #76. Program 1,457,365/3,145,728 bytes, globals 179,464/327,680 bytes, remaining 148,216. Later commits in this batch change Boat source only. No new Remote compiler errors observed. Remote host #101 passed 13 groups; map/keyboard tool #5 passed 8.
- 2452159 implements one-way local diversion to Home of an already-started mission, clears releases and keeps Home navigation authorization across session expiry/rekey without retaining the old keys, transport queues or stick leases. Old mission transaction stays local during detached return; new-session mission/control/stick traffic and ordinary actuator requests are rejected, authenticated/physical STOP takes precedence. Never starts a prepared-only mission. Invalid navigation/battery/geofence, physical STOP or an unconfirmed release pulse prevent continued movement. Full dual-link loss diverts, rekey during active mission also conservatively diverts to Home; no automatic resume of outbound. The return controller test is a software simulation of session expiry policy, not a live transport/driver test. Actual obstacle avoidance and path/geofence sensing remain hardware/algorithm acceptance items; a valid GPS fix alone cannot guarantee a safe water route.
- b239438 implements portable locally timestamped trusted sample validation, normalized differential navigation mixer candidate, 200 ms manual propulsion lease, one-at-a-time tokenized release with explicitly commissioned pulse durations. 21e0ee7 integrates the application bridge and a driver contract that requires independent electrical cutoff/watchdog; no physical driver registered in the default diagnostic sketch. Physical 100 ms output lease is a contract, not measured hardware behavior. Sample limits 100 ms safety, 2 s GNSS and 300 ms heading, and limits in tests are provisional engineering/fixture values, not commissioned coil parameters.
- 9627f72 adds actual-output feedback validity gates, requires a new local sample after deenergization before release completion, confirms manual transition only with stationary/idle feedback and emits authenticated measured status (including detached return failsafe) through the application bridge. Ordinary manual-release execution, full GNSS/heading/position telemetry generation and final board-specific input/output drivers are not yet implemented. No production key provisioning, eFuse changes or actual actuation performed.
- Boat host #40 (44808224304) passed 14 groups with new link-loss tests; #42 (44808489252, source 21e0ee7) passed 15 including watchdog/release tests. Boat target #10 (44808489093, job 136538752483, source 21e0ee7) PASSED: program 336,465/1,310,720 bytes; globals 20,216/327,680, remaining 307,464. Latest source 9627f726a9327b6e2a5ab0e13ecafc3617126766 triggers Boat target #11 (44808772228), host #43 (44808772480), ZIP #1076 (44808772344); pending at time of checkpoint. Last verified snapshot #1075 (44808489113) generated both development ZIPs. No version marker bump (028/014); immutable 029/015 publication not authorized by current acceptance gates.
- Acceptance remains incomplete: actual board design pinout/driver mapping, independent watchdog/STOP electrical verification, calibrated GNSS/compass/stationary detection, motor direction/current and all four coil pulse/idle measurements; actual secure pairing with keys and entropy on selected devices; Waveshare keyboard/touch/live SD timing, complete live telemetry and water trials. See Boat/docs/ACTUATOR_DRIVER_ACCEPTANCE_R1.md and docs/SECURE_PAIR_PROVISIONING_R1.md. No physical evidence available. Next: verify latest Boat target/host/package results and fix errors, complete manual release and live telemetry application paths, test bridge with a mock driver, then obtain exact physical hardware and perform disconnected-power/bench validation; keep releases gated until acceptance.

### Final host and archive evidence
Boat host #43 (44808772480, job 136539600288, firmware source 9627f726a9327b6e2a5ab0e13ecafc3617126766) PASSED all 16 groups including measured status confirmation and strictly newer post-pulse sample. ZIP #1076 (44808772344, job 136539600263, same source) PASSED snapshot creation and integrity/upload checks. Boat target #11 still running; last verified Boat build #10. Archive markers remain 028/014, and inner archive contents not independently downloaded. No hardware validation.


## Manual release, sensor telemetry and application integration — 2026-11-20

- Boat target #11 (44808772228, job 136539599468, source 9627f726a9327b6e2a5ab0e13ecafc3617126766) PASSED; program 338,069/1,310,720 bytes; globals 20,224/327,680 bytes; 307,456 remaining. No new compiler errors. Remote target #87 remains last verified Remote target (Remote source unchanged in this batch).
- 38fb447 integrates all four independent manual releases into the existing shared one-at-a-time pulse supervisor, checks stationary/idle/interlocks and primary authenticated link, prevents motor output during a pulse, sends terminal completion ACK only after post-pulse local idle confirmation, separates transport generation from cached actions, and rejects non-new manual release IDs after queue acceptance. A repeat completed ID may get a duplicate ACK without another pulse. Physical mechanical release is not proven by coil idle. No actual physical driver registered. dd66dda adds neutral rearming after release and STOP on a new session unless a detached return remains active, strengthens GNSS status validity gates.
- 38fb447 adds locally timestamped coherent battery/heading/speed and matching lat/lon pair telemetry from trusted driver samples, periodic status at 200 ms and sensor batch at 500 ms. Invalid/stale fields use unknown sentinels or omit coordinate pairs; no synthetic GPS coordinates are transmitted by diagnostic firmware. Actual GNSS/compass/battery board-specific acquisition remains unimplemented/unverified. Waypoint index currently unknown (255) in sensor batch. Real RF performance and remote status rendering require physical tests.
- Boat host #44 (44810506477) passed 17 groups; #45 (44810626275, job 136545934705, source dd66dda3ddc887c07da8863bb73ef38288c90c94) PASSED 18 groups, including new telemetry codec round trips and actual production application bridge compiled against a mocked radio and physical driver. Test covers all four releases, concurrent request denial, backup denial, old idle sample rejection, post-release neutral rearming, manual lease expiry, loss of primary during pulse, session rotation, telemetry and failed reads. It does not test live RF dispatch, electrical cutoff, actual mechanisms or water.
- Boat target #12 (44810506870) cancelled by later source update, not a failure/pass. Target #13 (44810626250) still running at checkpoint. ZIP #1080 (44810626184, job 136545935490) PASSED exact-source snapshot creation, unzip integrity and artifact upload; immutable publication skipped, no version bump. Inner archives not independently downloaded.
- Added docs/PHYSICAL_COMMISSIONING_R1.md, staged 18 physical acceptance checks and evidence form. All physical results NOT RUN. Existing release fixture timings and steering gains are NOT real coil/driver settings. Separate electrical watchdog and exact board driver implementation remain mandatory; pairing and touchscreen/SD hardware test, real geofence/water trial and CHIRP integration remain open.
- Next: verify Boat #13, fix errors if any; audit remaining actual hardware adapters and real release ACK timeout against measured coil pulse lengths; complete board-specific integration and physical acceptance with locally measured evidence. Do not claim operating readiness or publish official VER029/VER015 before acceptance. Current marker 028/014 retained.

### Final target verification
Boat target #13 (44810626250, job 136545998180, source dd66dda3ddc887c07da8863bb73ef38288c90c94) PASSED; program 339,993/1,310,720 bytes, global RAM 20,280/327,680 bytes, remaining 307,400. Firmware and compile log uploaded successfully. Remote firmware source unchanged since successful target #87. Both target builds and 18 Boat / 13 Remote host groups are verified for the current software revisions; physical commissioning remains NOT RUN, official releases remain 028/014. No physical readiness claim.


## Board-specific integration R4 — 2026-11-20
- Manufacturer public revision documentation reviewed: V1.2-specific authoritative schematic not located; V1.1 and V2 are not safe substitutes. Added conditional GPIO crosswalk and provisional companion I/O candidate with RP2040 GP assignments, sensor/actuator logical connector schedule and independent electrical watchdog design calculations in Boat/docs/BOAT_BOARD_INTEGRATION_R4.md and BOAT_HARDWARE_PIN_CONNECTIONS_R4.csv. No final pin freeze or actual wiring. New ADS1115 and companion controller are proposals; exact module and power cutoff require selection/engineering. TPS3431 150pF candidate timing 66.61ms nominal, approximate 59.8–73.5ms bounds before total cutoff; requires measured <=100ms including gate/feedback. Four separate coil connections documented, pulse/current/fuse ratings pending. No KiCad schematic or ERC completed in this batch.
- e640800 adds bounded local UC6580 checksummed GGA/GSA/RMC/GST parser requiring coherent UTC epoch and actual accuracy estimate, calibrated tilt-compensated heading math and ADC pack conversion. No HDOP fabricated as metres; GST output must be verified/enabled and estimated uncertainty commissioned. 1ebd347 adds actual Arduino UART/I2C register acquisition for UC6580, MPU9250/AK8963 and proposed ADS1115, and normalized reversible motor PWM/one-hot output planner. ca79245 strengthens coil channel-change/fault guards and adds optional read-only locally audited diagnostic sketch path. Default remains no GPIO/sensor or actuator activation. Heading/battery calibration initially disabled, no state-of-charge or safety flag fabrication. No companion firmware or high-current output GPIO adapter yet; no physical testing. Boat host #46 (44811977065) and #47 (44812118555) passed 19 and 20 groups respectively; latest #48 (44812326454), target #16 (44812326290), ZIP #1085 (44812329736) pending at this checkpoint. Last verified Boat #13, Remote #87 unchanged. Version markers Remote 028 / Boat 014 unchanged. Next verify #48/#16/#1085; resolve actual Tracker revision and I/O topology before native schematic, final hardware driver mapping, watchdog gate and current/feedback design, then physical acceptance.

### R4 final CI evidence
Boat host #48 (44812326454, job 136551764834, source ca79245cb6800637192514d3728826449b049593) PASSED all 20 groups. Boat target #16 (44812326290, job 136551916706, same source) PASSED: program 357,917/1,310,720 bytes, globals 20,600/327,680, leaving 307,080. Default diagnostic configuration compiled; locally commissioned optional sketch configuration and actual sensor I/O untested. ZIP #1085 (44812329736, job 136551778130) PASSED snapshot build, integrity checks and upload; archive publication skipped and inner archive contents not independently inspected. Remote #87 and Remote host #101 remain valid for unchanged Remote firmware. Physical tests NOT RUN; versions 028/014 retained. Next resolve V1.2 exact hardware documentation and candidate companion-I/O design approval, then capture native schematic/verify power budget and implement companion peripheral protocol/driver and compile-time signed pin profile.


## Owner design decision — Heltec-only MDD20A restoration (R5)
The owner explicitly rejected the RP2040 companion and restored the original Heltec Tracker + Cytron MDD20A direct PWM/DIR design. This supersedes the R4 companion recommendation and all companion development commits. Removed companion Pico SDK source, wired protocol/master/safety core, companion-only tests and Actions workflow from the active branch; Git history retains them. Kept Tracker GNSS/IMU/ADC acquisition, output planner, existing radio/session/mission/release/telemetry and independent electrical watchdog/STOP requirements. Added a fail-closed direct GPIO profile validator and regression; diagnostic sketch remains unassigned. New active R5 design and pin schedule supersede R4. No MCU pins, expander part, high-current cutoff, or PCB frozen: authoritative V1.2 pin map and exact GPIO budget unresolved. Four independent solenoids retained. Remote source unchanged; release version markers Remote028/Boat014 retained. Re-run Boat target, 21 host groups and ZIP packaging; record actual results. All physical acceptance gates remain open.

### R5 rollback verification
Commit a22886a7c0c59694a5a8c2d7bff3a519d1e4a047 updates 28 paths atomically. Recursive tree audit confirmed no companion-specific source, master link, safety core or workflow files remain. Prior abandoned Boat target #19 (44813593533) failed in carrier_link/carrier_master_port; those obsolete files have been removed rather than repaired for a rejected architecture. Boat host #52 (44814101535, job 136557839658) PASSED all 21 active groups, including direct GPIO profile regression. ZIP snapshot #1091 (44814101036) PASSED; official markers unchanged, inner archives not independently inspected. Boat target #20 (44814101124) still running at verification; last successful Boat baseline #16. No new compilation pass claimed.
Active R5 BOM added; old schematic/BOM references explicitly marked historical for corrected four-channel topology. Remaining: confirm exact Tracker revision, audit native GPIO count and optional non-programmable I/O expansion, capture direct MDD20A driver and independent watchdog/power-gate native schematic, implement physical driver only after pin freeze, then commission with disconnected actuator power. Remote untouched. No physical or ERC acceptance.


## R6 continuation — direct MDD20A HAL, pin budget, sonar deferred (2026-11-21)
- Boat target #20 (run 44814101124, job 136557839493, prior approved R5 source) PASSED. Program 358,009/1,310,720 bytes, global RAM 20,600/327,680 bytes (307,080 remaining); tooling emitted only external esptool/Node warnings.
- 3fa3799 adds `mdd20a_arduino_stage.*` (Arduino-ESP32 3.x LEDC 20 kHz/10 bit, two channels, direct PWM/DIR, initial PWM/permit/WDI LOW), and `mdd20a_direct_safety.h` with local measured feedback freshness and latched failure; **driver is NOT instantiated or attached to active Boat firmware, no GPIO selected and no power output authorized**. 3d1a755 makes motor→coil interlock enforce the existing shared output planner; d70028f corrects a test expectation at exactly the 30 ms post-release deadtime boundary. Full target and host verification of these new changes must be checked separately before asserting pass.
- Added R6 direct GPIO budget and electrical capture checklist: independent TPS3431/safety cutoff, Heltec native MDD PWM/DIR/WDI/permit, optional non-programmable I/O expansion only for coil/feedback, and strict signed pin map gate. Exact V1.2 PCB/schematic not supplied, final GPIO allocation blocked; current work is an interface skeleton, **not native KiCad schematic or ERC**. Power, motor and coil values remain commissioning candidates only.
- **Sonar is intentionally only planned** in Boat/docs/BOAT_SONAR_INTEGRATION_PLAN_UNDECIDED_R1.md. Owner has not selected a model. No sonar code, new display, GPIO, transducer wiring or chosen transport. One existing 4.3-inch 800×480 Waveshare screen, real CHIRP-like fish arch echogram, manual/autopilot, 50/50 map/sonar view and optional full-screen mode remain requirements; actual data protocol, electrical budget and high-throughput link depend on selection.
- Prior Remote #87 and unchanged Remote software remain verified; official markers Remote VER028 / Boat VER014 unchanged, no physical acceptance or release authorization. Next: check latest target and host CI, correct errors, freeze the exact Heltec V1.2 pin map and safety hardware/feedback, and capture a native MDD20A/power/coil KiCad design after verified choices. No physical actuation before bench proof.


## R6 verification and frozen sonar boundary — 2026-11-21

- **Boat ESP32-S3 target #22 PASSED**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865488754 (job 136728159017, source 3d1a75530f7e27dc0132de36a93de999f7ed0a8c). Program 358,121/1,310,720 bytes; globals 20,600/327,680 (307,080 remaining). Arduino core 3.0.2 source with direct MDD20A LEDC output HAL compiles; target #21 was superseded. No physical GPIO configured.
- **Boat portable host tests #55 PASSED all 22 groups**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865543808 (job 136728262617, source d70028f22e73d52b967bb2cb35057d443c2e2cc6). First intermediate test runs #53/#54 failed due incorrect motor/coil state and deadtime test expectations. The source and test were fixed in 3d1a755/d70028f; the final source passes regression. These were software test failures, not evidence of an actuated device.
- ZIP packaging #1096 PASSED on documentation commit 465caea4696c928100734f99919af1f2f0075f31: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44865619611. Boat snapshot artifact 14691667268, Remote snapshot 14691388678, digest manifest artifact 14691647257. Inner ZIP payload and hashes have not been independently inspected outside CI. No official release.
- Confirmed Heltec-only architecture. Direct MDD20A stage remains **uninstantiated** in the Boat diagnostic sketch and gate/STOP/WDI feedback remains uncommissioned. It does not energize the actuator permit or auto-pet the independent watchdog. **Do not claim motor-driver physical integration or circuit safety acceptance.**
- R6 sonar planning is recorded **without selecting any sonar model or implementing sonar**. The one-Waveshare-screen full echogram and fish arch requirements remain, with sonar power/data connector, transport, parser and transducer unassigned. Dedicated model selection gate retained. No code, GUI, firmware, GPIO pin or hardware circuit was added for sonar.
- Unresolved engineering decisions before native KiCad/Electrical Rule Check (ERC): exact Heltec V1.2 authoritative schematic/pin audit; six truly free direct output-capable motor/permit/watchdog pins while preserving GNSS+nRF+E22+I²C; whether a non-programmable I/O expander is necessary for four solenoids plus physical feedback; actual independent watchdog cutoff, coil currents and fuse ratings. **No board purchase, physical acceptance or official release.** Remote #87 and versions Remote 028 / Boat 014 remain unchanged; planned 029/015 gated on hardware acceptance.


## Next-step engineering R7 — direct MDD20A feasibility and KiCad draft (2026-11-21)

- Current Heltec-only MDD20A design preserved. Added V1.2 conditional GPIO feasibility program and matrix. Native minimum for nRF primary + external E22 UART + I²C + MDD20A PWM/DIR + WDI + permit is 12 pins; optimistic unverified V1-family candidate pool 11. A GPIO expander for coils alone cannot resolve the minimum deficit. No actual Heltec GPIO pins allocated. Non-programmable I²C-UART bridge or expander for DIR are contingency studies, not approved selections. Source documentation: Boat/docs/BOAT_MDD20A_ELECTRICAL_GATE_R7.md.
- Captured editable **preliminary native KiCad** MDD20A control/watchdog sheet with custom embedded connector and TPS3431 symbols; first KiCad 9 netlist export succeeded. **First ERC reported 83 violations**, not an electrically approved/fabricable design. High-current switch/gate, 4 solenoid MOSFETs and final rails are not captured. Dedicated KiCad validation workflow and report artifact created; ERC must be reviewed and corrected before board release.
- Added measured-only pack current budget and hardware fault-latch requirements with software verification. No invented motor stall, coil current or fuse selection. Physical watchdog signal must latch actuator power OFF independent of MCU; 150 pF CWD is a candidate (66.61ms nominal, approximate 59.8–73.6ms bounds excluding switch/current decay). Every physical input/driver still default-denies.
- Sonar plan remains **unchanged and model undecided**. No sounder, firmware, UI, connector pin/power or code has been implemented this turn. Existing 4.3-inch screen and future full CHIRP-style fish arches remain requirements.
- No release increment or physical acceptance. Remote #87 and Boat #22 target compiler baselines remain last verified for unchanged application code. Host regressions and ZIP snapshots run in GitHub Actions; check the latest jobs for exact SHA before claiming results. Next: authorize a verified GPIO resource solution and complete KiCad/ERC, physical watchdog, power and four-coil circuits.


### R7 completion checkpoint — 2026-11-21

**Active architecture unchanged:** single Heltec Tracker + external Cytron MDD20A, nRF24 primary/E22 backup, four independent solenoid channels, no RP2040. Sonar model explicitly **undecided**, sonar code/wiring **not implemented**.

**Verified software and project checks:** Boat host #59 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714160) PASSED all existing portable regressions plus GPIO feasibility, pack-current safety and gate truth-table tests (24 groups by workflow stages). ZIP packaging #1103 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714149) PASSED for source SHA 358ed10687fbd09606aafbaf18a730cc0ef80e49. Boat target #22 remains last verified ESP32-S3 diagnostic source build; subsequent commits modify tests, docs, and preliminary CAD only. Remote target #87 remains last verified Remote build. Official Remote VER028 and Boat VER014 unchanged; 029/015 withheld.

**Native preliminary KiCad progress:** KiCad workflow #6 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866942940, source e2ec4f6c8cfc3852c78d1f475a7f57af1c7a6ed4) PASSED parser/netlist export and geometry/project-symbol checks after adding project-local `Flytec.kicad_sym`, `sym-lib-table` and `.kicad_pro`. Unresolved ERC violations reduced from 83 to 26 after grid alignment, then to **15** after project library setup (13 unimplemented release/feedback net connections and 2 unpowered TPS3431 power pins). **ERC production acceptance FAILS/NOT COMPLETE**. Remaining physical power gate, watchdog fault latch, high-current motor/coil circuits, fuse/protection, final footprints, true V1.2 pin assignment and PCB are not captured. A successful *preliminary* KiCad check must never be represented as zero-violation ERC.

**Verified GPIO shortfall:** provisional best-case pin pool 11, minimum native demands 12 with both external radios retained; even this assumes contested oscillator/LED/UART/flash pins are available. New active `Boat/tools/gpio_feasibility_r7.py` and matrix explicitly block unsafe assignments. I²C-UART bridge or direction expander remain optional proposals, neither selected or implemented. All physically driven GPIOs remain unassigned/disabled. No hardware or physical acceptance evidence; no sonar model or implementation. Next: decide a safe extra I/O topology after exact Heltec V1.2 pin documentation, fill unresolved nets/rails in native KiCad, verify a separately latched electrical cutoff, run zero-violation ERC and physical commissioning before releases.

## R8 follow-up — integrated Heltec SX1262, 2026-11-21

Owner-selected architecture: one Heltec Wireless Tracker ESP32-S3, external Cytron MDD20A with native two-channel PWM/DIR, nRF24 primary, and **Heltec onboard SX1262 SPI LoRa** (Boat E22/E227 UART retired; no Boat SC16IS750). No RP2040. Four independent bait/hook solenoid drivers and separately latched physical watchdog/STOP power gate remain required. Sonar stays on hold: no chosen model, hardware, firmware or circuit.

New Boat source: heltec_sx1262_link.* and heltec_sx1262_airtime.h; transport construction now selects integrated LoRa. All GPIO remain -1, V1.2 board audit/remote PHY/regulatory sign-offs false: no radio TX, motor or solenoid activation authorized. RadioLib 7.3.0 pinned for compile attempts, host GPIO/airtime tests added. No compile evidence for this R8 source until Actions checks finish.

GPIO demand drops from historical R7 12 to **10 external native controls**; 11 optimistic provisional candidates yields a count-only pass, but conservative uncontested pool of five still fails. Onboard SX1262 pins 8–14 remain internally occupied, and shared nRF SPI routing/header access is not signed. No pin freeze.

**Critical Remote incompatibility:** Remote remains E22 UART via SC16IS750. Its embedded E22 transparent RF packetization is not automatically compatible with a raw SX1262 SPI radio. The Boat's LoRa backup is intentionally unavailable until a matching RF protocol/remote revision is validated. At SF9 BW125, estimated 32-byte airtime ~247ms; 3s heartbeat and 6s freshness are not certified compatible with 1% duty. Regulatory and reset-persistent airtime enforcement unverified.

Remaining R7 KiCad ERC: **15 violations**, not production-ready. High-current latch, four MOSFET/coil drivers, rails, footprints, PCB and physical testing still outstanding. Current release markers Remote 028 / Boat 014 retained; 029/015 withheld. See Boat/docs/BOAT_INTEGRATED_SX1262_R8.md and Boat/docs/BOAT_LORA_HARDWARE_DECISION_R4.md.

Commit evidence: source 975088133dbd8c4e9cb81d3760a5b65292f7426f; GPIO/CI/documentation 557d601b1a561fd0a908e90d24839e12b3ab3072.

### R8 integrated SX1262 verification (2026-11-21)
- Boat ESP32-S3 **target compile #27 PASSED** on source SHA `9c96f937011b4af15179b2cb3c172df2bf0b1b83`; https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167311 . Arduino-ESP32 core 3.0.2, RF24 1.4.11, RadioLib 7.3.0. Program 417,037/1,310,720 bytes (31%), global memory 22,120/327,680 bytes (6%, 305,560 remaining). Diagnostic only: GPIO/radios uncommissioned, actuator power disconnected.
- Boat portable host tests **#64 PASSED** on same source SHA: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167361 . New test includes 10-signal GPIO feasibility, LoRa airtime and 1% interval model; simulated checks only.
- Snapshot ZIP CI #1110 **PASSED** on source SHA 9c96f937: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167366 . Newer docs-only snapshots also generated; official release markers remain 028/014 and their immutable releases were NOT authorized.
- Code fix after transport insertion: shared SPI selects inactive before bus init, then RF24 init and SX1262 init; RadioLib constructor requires `SPIClass&`, corrected. Commits `ca82b7dc`, `9c96f937`.
- R8 superseding purchasing list and pin schedule: `Boat/docs/BOAT_BOM_CANDIDATES_R8.md`, `Boat/docs/BOAT_HARDWARE_PIN_CONNECTIONS_R8.csv` (commit `2ed54e3`). Electrical capture drafting spec: `Boat/docs/BOAT_SAFETY_SOLENOID_CAPTURE_R8.md` (commit `7bb4968`). These are *not* native KiCad power/coil circuits.
- The **remaining blocker** is an authoritative V1.2 signed pin map and verified Remote-to-Boat backup LoRa PHY. Remote still runs E22/SC16IS750; Boat raw SX1262 is intentionally disabled. 10 vs optimistic 11 GPIOs is count-only. Independent physical fault latch, all four coil driver circuits and measured power budget are NOT complete; R7 KiCad has **15 open ERC violations**. No PCB/DRC, hardware acceptance, or sonar integration. Retain single Heltec + direct MDD20A without RP2040.


## R9 engineering continuation — native four-coil KiCad and independent safety model (2026-11-21)

**Completed and verified in connected GitHub branch:**
- Native editable schematic `Boat/hardware/Coil_Drivers_R9/Boat_Four_Coil_Drivers_R9.kicad_sch` with 26 parts: four independently fused 12V coil plugs, four low-side NMOS stages, four cathode-to-positive flyback clamps, four input series gate resistors and four pulldowns; separate supply J1 and hardware-limited one-hot pulse input J2. Project-local KiCad symbols and `.kicad_pro` are included. All footprints/rated power device selections explicitly UNASSIGNED.
- KiCad R9 workflow **PASSED**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44874476930 (`546eacf83a0cf3df24f2d1c6f087368505d146df`). Native KiCad XML netlist exported, exact device pin/net checks passed for **18 distinct nets**, and this **standalone low-side coil sheet reports zero ERC violations**. Previous run failed solely because the test needed to normalize KiCad's root sheet `/` local-label prefix, corrected on `546eacf`. This says NOTHING about approval of the entire electrical system.
- New software-only reference model `Boat/tools/safety_gate_latch_r9.py` enforces fault-latched stop with physical ARM **released-then-pressed** under healthy stationary feedback and tests a 4-channel one-hot non-retriggerable pulse limiter with time cutoff and stuck-high rejection. **Not an implemented physical latch/monostable circuit.** Boat portable host CI **PASSED**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44874405863 (`88bb983f488b6778e88222f9eaa76d1612e5f446`).

**Remaining blockers unchanged:**
1. Capture actual 3S pack fuse, verified 3S-to-5V supply, all motor/coil high-current rails and **normally-OFF load-rated high-side cutoff** in native KiCad, independent of Heltec; derive part ratings from measured two-motor stall, all four solenoid currents, pack/BMS waveform and temperature.
2. Capture actual TPS3431-driven hardware fault latch for STOP, WDO, UVLO, overcurrent, thermal; force latched OFF at boot, and make rearm require a distinct physical button edge, healthy rails, verified motor+coil zero output. An abstract simulator does not implement this.
3. Capture real hardware one-hot gate selector, non-retriggering maximum coil pulse, and current-zero feedback; never join R7 `SOL_REQ_*` straight to R9 `PULSE_*_LIMITED_TBD`. No direct MCU-to-coil gate!
4. Integrate native KiCad hierarchy and clear **15 old R7 ERC violations**. R9's standalone zero ERC does not reduce those. Next PCB/DRC, measurements, controlled dummy-load STOP/watchdog verification, and physical acceptance.
5. Authoritative Heltec V1.2 GPIO/shared-SPI physical audit and Remote E22↔Boat raw SX1262 radio PHY compatibility remain unverified. Boat LoRa backup and actuator GPIO remain disabled. Sonar unselected and untouched. No rudder, no RP2040. Official release markers remain Remote 028 and Boat 014; 029/015 remain withheld.


## R10 engineering checkpoint — independent STOP/witness, high-side candidate and cross-sheet safety (2026-11-21)

- Added native editable `Boat/hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch` with **23 generic component instances** and **35 exported KiCad electrical nets**. It comprises SN74HC08 safety-input AND tree, SN74HC74 positive-edge ARM/asynchronous-clear latch, SN74HC14 redundant STOP witness, a candidate PMOS high-side branch to BOTH MDD20A and four-coil supplies, a low-side MOS gate driver, main/motor/coil fuses, physical NC STOP source-return contact and second independently monitored NC witness. Actual monitored J3 STOP_OK requires a third mechanically coupled contact and independent conditioning. **Hardware is draft/not powered/not a signed safety system.**
- STOP single-monitor stuck-good hazard was caught in review: S1 opening Q2 source alone could allow automatic reenergization after physical STOP release if J3 STOP_OK stuck HIGH. Added S2 separate NC witness, candidate R5 pullup and SN74HC14 to clear the latch even with the other contact input stuck HIGH. Hardware triple-pole STOP mechanical selection/synchronism/required NC behavior and actual ARM debounce/release-qualified pulse remain unverified.
- Native R10 KiCad target run **#(44890983595) PASSED structural tests** on `e77facdc4edb789906429cbd215404858374e266` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44890983595): **35 nets**, no geometry warnings, **exactly TWO unresolved `power_pin_not_driven` ERC errors** (3V3 safety rail and return; no regulator approved). CI now **checks for exactly those two blocking power-source errors** and rejects new violations; this is not a zero-ERC or fabrication release.
- All **1,024 combinations** of the ten external conditioned inputs and the redundant STOP-contact scenarios are covered in host-only truth-table tests, including stuck ARM and permit. Boat host CI **passed** on `7d515e89` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44890701527), as did the cross-sheet interface host CI `7d894bf` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44891024408). A Python model is not hardware fault containment.
- `Boat/docs/BOAT_R7_R9_R10_INTERFACE_CONTRACT.csv` enumerates critical unverified signals. R7 has its own 10k WDO_N pullup; R10 R1 is now **DNP if R7 pullup fitted** to avoid an undocumented parallel pull-up. R10 coil rail connects to R9 `ACT_SOL_POS_AFTER_LATCH_TBD`; return mapping is PACK_RETURN to R9 ACT_RETURN only through a separately rated harness. R7 solenoid requests MUST NOT directly drive R9 gates: the independent one-hot hardware limiter and pulse time-out do not exist yet.
- Remaining ERC by **standalone sheet**: R7 control/watchdog **15 unresolved**, R9 low-side coil **0** (but not a complete actuator system), R10 STOP/power candidate **2 deliberate external-power-source errors**. These are separate schematics; no combined hierarchy, system-wide ERC, PCB DRC, measured thermal/stall/SOA, physical cutoff latency, final 3S fuse/regulators, ARM hardware, overcurrent/thermal/UVLO comparators, or production footprints.
- Keep exactly one Boat Heltec Wireless Tracker + separate MDD20A dual PWM/DIR differential thrust; **no rudder**, no RP2040, onboard Boat SX1262 currently blocked by unverified Remote E22 PHY interoperability. Sonar remains unselected/held with no implementation. No physical actuator activation. Official markers Remote VER028 and Boat VER014 unchanged; 029/015 withheld.


## R11 physical-timing candidate and verified KiCad continuation — 2026-11-21

- Created an actual editable KiCad schematic at `Boat/hardware/Pulse_Limiter_R11/Boat_Pulse_Limiter_R11.kicad_sch` with 17 components: CD74HC221 monostable, SN74HC175 channel latch, six SN74HC08 quad AND ICs, two SN74HC32 quad OR ICs, one SN74HC14 Schmitt inverter IC, three R/C passives, and three harness connectors. All footprints and timer R/C ratings remain unassigned.
- Native KiCad **PASS** run https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201259 on `cb8ed8546bff1c5555ddecc667c052365cbe61d1`: 54 named exported XML nets checked against exact pin numbers, 9 explicit no-connect pins, 1,024 static one-hot/window combinations and cross-sheet logical-name checks passed. The isolated R11 sheet has **exactly two outstanding power_pin_not_driven errors**: the external unqualified 3V3 safety rail and return. No additional ERC errors; neither 2 ERC errors nor isolated ERC proves system safety.
- Existing Boat host CI PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201349; ZIP packaging workflow PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201366. Previous R11 KiCad run failed a test assertion pin number (pin 13 vs real pin 12), corrected before passing.
- The new interface contract `Boat/docs/BOAT_R7_R9_R10_R11_INTERFACE_CONTRACT.csv` maps R7 raw release, R9 MOSFET gates and R10 safety, but **all physical inter-sheet connections remain UNBUILT**. R10 internal LATCH_Q is not wired out to an approved connector.
- **Blocking unsafe failure mode:** The external `RELEASE_EDGE_TOKEN_TBD` is **not implemented**. If this input remains HIGH, a held release request can trigger a second pulse after the 74HC221 window ends. Analog motor-/coil-current-zero comparators, independent 3V3/POR and rail-good logic, R/C pulse bounds, input filtering/setup/hold and actual actuator STOP/current-off testing are also missing. **Never connect/energize motors or solenoids from this schematic.**
- Remaining standalone ERC blockers: R7 **15**, R9 **0** (isolated low-side stages), R10 **2** (external power), R11 **2** (external power). No combined ERC, PCB DRC, validated high-current path, physical acceptance or release.
- Next: capture the *consumed* hardware release/zero-current rearm token (no autorepeat), independently qualify current-zero and logic supply, analyze race/glitch/fault timing, establish manufacturer pulse width/fuse/MOSFET ratings from measured hardware, integrate four sheets into native KiCad hierarchy and run system ERC. Sonar remains **planned only**; no sonar module selected. Single Heltec Boat controller with MDD20A differential motors/no rudder, no RP2040 and no Boat E22. Official Remote VER028 / Boat VER014 unchanged; 029/015 withheld.

## R12 physical consumed-release token — verified isolated KiCad candidate (2026-11-21)

- New native **editable** schematic `Boat/hardware/Release_Token_R12/Boat_Release_Token_R12.kicad_sch` (23 generic components) and project-local symbol library. Two flip-flops in SN74HC74 implement **RELEASE_ARMED** (set only after a separately RC/Schmitt-qualified all-requests-released and current-zero condition) and **FIRED_Q** (held during a pressed request, consuming RELEASE_ARMED asynchronously). CD74HC221 supplies a *separate short token* to existing R11 `RELEASE_EDGE_TOKEN_TBD`; six pairwise overlap terms reject multi-request patterns. R1-C4 timing/filters unspecified, all footprints blank. R12 output connector J3 is **NOT physically wired to R11 J2** yet.
- Actual KiCad CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745446 on `ee9ff4fbb122ce59ce2628007c3ad2f05d24ace8`: **53 precisely matched electrical nets, 11 explicit no-connect output pins, 256 static mask/ready tests and 7 synthetic temporal-model tests**. Only **2 unresolved isolated-sheet `power_pin_not_driven` ERC violations** remain for the unapproved 3V3/ground source; this is NOT zero ERC on full Boat. Boat portable host CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745373; ZIP packaging CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745305.
- Detailed interface mapping: `Boat/docs/BOAT_R7_TO_R12_INTERFACE_CONTRACT.csv`, with every unbuilt harness/status explicit. R7 SOL_REQ_x to R12 J1; R12 J3 pin1 to R11 J2 pin6; R11 J3 4 gates to R9 J2; R10 internal LATCH_Q is NOT exposed on an approved connector. R10 feeds R9 coil rail only by a proposed independent fused power connector.
- Critical remaining hazards listed in `Boat/docs/BOAT_R12_HAZARD_REVIEW.csv`: external POR may assert before 3 separate RC filter nodes settle, request bounce/race, non-zero coil current or failed current sensor, R12 token width longer than R11 main time, stuck-high token that can retrigger R11, U2 or high-side MOSFET failure, motor and coil real flyback, unverified 3S high current. **No actuator power may be connected.**
- Component gate: qualify *actual* 3S-to-logic regulator/POR, R/C release/debounce/reset-to-CLK and token-vs-main timing over tolerance+temperature, independent motor/coil current-zero comparators, physical R10 STOP latch and output connector, and a power-rated high-side actuator disconnect for both motor and solenoid supply. Integrate actual combined KiCad hierarchy/ERC/PCB/DRC and perform dummy-load fault acceptance.
- Standalone ERC status: **R7 15, R9 0 (partial low-side only), R10 2, R11 2, R12 2**. No system-wide zero ERC or fab authorization. Existing Boat and Remote pin maps remain uncommissioned; no RP2040, no rudder, no Boat E22. Sonar has no selected model and remains **planning only**. Official versions Remote VER028 / Boat VER014 unchanged; VER029/VER015 withheld.

## R13 RECOM ready-made 3.3 V safety supply (2026-11-21)

**User choice: use a premade regulator module instead of a custom 3.3 V feedback/inductor buck design.** Selected candidate **RECOM R-78E3.3-0.5** SIP3, datasheet REV 9/2025 https://recom-power.com/pdf/Innoline/R-78E-0.5.pdf. Vendor pinout **pin1 VIN, pin2 GND, pin3 VOUT**; input **6–28 V**, fixed output **3.3 V**, max **0.5 A**, **11.6 × 8.5 × 10.4 mm**, non-isolated. 3S 12.6 V maximum nominal charge is in range assuming measured input spikes stay within 6–28 V.

Created native **R13 KiCad** power interface `Boat/hardware/Safety_Power_R13/Boat_Safety_Power_R13.kicad_sch` with 9 components: fused protected battery-to-module VIN, supply/common-return input, VLOGIC_3V3/GND outputs to R10-R12, test interface, optional 10uF/35V input and 10uF/10V + 100nF output decoupling, optional 300 ohm/0.125 W preload (~11mA; RECOM recommends >2% min load for specified ripple). No fuse value or footprint approved; optional parts may be omitted only after ripple and EMC tests. Vendor 220uF maximum output capacitive-load condition includes all downstream decoupling. `Boat/docs/BOAT_R13_READYMADE_REGULATOR_BOM.csv` lists choices. Native exported KiCad netlist / ERC CI is `boat-kicad-power-r13.yml`.

**Safety limits:** this preassembled non-isolated regulator is **only for independent safety logic**, not nRF24 PA/LNA burst supply, not Heltec ESP32 input (which needs separately approved 5V), and never motor/solenoid load. Source is MASTER-switched pack protected by BMS/fuse, electrically **before** the R10 actuator high-side cutoff. Its output ground shares a low-current star reference with the battery; prevent high-current motor/coil return in safety harness. The RECOM device has **no separate PG pin** and short-circuit protection **automatically restarts**. A separate physical POR/brownout supervisor must assert reset on loss and require manual physical fault-latch rearm after recovery. The regulator alone is **not** an independent safety interlock. Input 3S pack transient, fuse, mounting/socket retention, load/thermal, EMI/RF noise, real 3.3V ramp/current and POR/UVLO timing are **unverified**.

The new R13 power sheet does NOT automatically clear older separate-sheet ERC: R7=15; R9=0 (partial low-side); R10=2, R11=2, R12=2 (external supply); combined KiCad hierarchy, PCB, rated high-side cutoff and physical loads remain unfinished. No sonar implementation, no RP2040/rudder/Boat E22. Keep official Remote VER028/Boat VER014, no physical actuation permission.

### R13 native KiCad completed structural verification (2026-11-21)

- Manufacturer ready-made **RECOM R-78E3.3-0.5** safety-only 3V3 regulator; native sheet `Boat/hardware/Safety_Power_R13/Boat_Safety_Power_R13.kicad_sch`, module pins **1 VIN, 2 GND, 3 VOUT**, input fuse F1, optional RC supply components, separated 3.3V safety connector/test connector, no production footprints.
- Actual CI KiCad 9 XML+ERC **PASS**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44931874988 on commit `0b5eb1f149a62c5c951429a5ab37a57f1b485847`. Validated **9 component instances / 4 exact electrical nets**, including series fused input and vendor supply pin numbers. **Exactly two remaining blocking external power-source ERC errors** (upstream switched 3S supply and logic common return); **zero extra ERC warnings**, no off-grid endpoints. The first R13 runs failed missing connector endpoints and off-grid placement; native 1.27 mm grid correction `0b5eb1f` supersedes them.
- R13 is physically unassembled and its source BMS/fuse, transient protection, 500mA load budget, ripple, PG/POR supervisor, EMC, connector/socket, RF separation and physical R10/R11/R12 connections have not been commissioned. RECOM module has **no PG pin** and can restart after a short-circuit; independent reset/latch must stay OFF and require deliberate manual ARM. The R13 sheet does not imply zero whole-Boat ERC and **is not fabrication-ready**. No servo/rudder/RP2040/Boat E22; CHIRP sonar choice remains open, no sonar firmware. Official release markers Remote VER028 / Boat VER014 unchanged.

## R16 owner-confirmed four-channel module (2026-11-21)

**HARDWARE SELECTION FROZEN:** Electrokit **EKM014**, manufacturer/article **41024394**, is the final *model choice* for Boat LH/RH bait and LK/RK hook solenoid **low-side** switching. Exactly **ONE** four-channel preassembled module. It **replaces R9 custom-FET PCB permanently** in the active module-only project. This choice does **NOT** mean purchased, electrically verified, qualified, or safe to energize. No substitution to NCD or other coil driver without owner approval.

Read the dedicated [EKM014 R16 hardware and wiring contract](Boat/docs/BOAT_EKM014_SELECTED_R16.md), [R16 channel/wiring CSV](Boat/docs/BOAT_EKM014_PIN_WIRING_R16.csv) and [R16 commissioning test plan](Boat/docs/BOAT_EKM014_ACCEPTANCE_R16.md) (relative paths from repository root). R15 module-only policy remains mandatory for both sides. A dedicated *commercial* hardware one-hot and no-repeat maximum pulse path, independent DC power cutoff/STOP, rail/brownout supervision and independent current-zero verification are still required before connecting coils. Sonar still planned only; no RP2040/rudder/custom PCB. Remote VER028 / Boat VER014 remain held.


## R63 — cross-endpoint radio fail-safe and Home application execution (2026-11-22)
Developed and host-tested an aligned backup-radio control policy: STOP and active-mission Home only, **no backup steering or release**. Boat primary loss triggers RTH only for an existing valid plan with healthy local navigation; otherwise commands are stopped. Fixed previously queued-but-ignored `CMD_SET_MODE RETURN_HOME` in Boat application with originating-link ACK and independent no-plan rejection. Remote host run 44991125196 PASS; Boat host run 44991241882 PASS. Both latest target compiles and physical RF/stop hardware are still commissioning gates. [R63 detailed report](R63_BACKUP_CONTROL_AND_RTH_INTEGRATION.md). Official release markers remain Remote 028 / Boat 014.


## R65 — safety module research and selected Boat pin-resource budget (2026-11-22)
- Verified manufacturer and specialist data for **Duelco NST-3.2 42042237** 8–36V STOP monitor; **Brentek P8D-ISM5** 4.5–5.5V rail+heartbeat watchdog candidate; **Albright SW60** 12V DC family contactor (exact monitored aux variant unresolved). Brentek P8D-ISM12 10.9V input floor is incompatible with full raw 3S operation. Reviewed **Pilz PNOZ m B0 772100** 24V configurable safety PLC and **Mean Well DDR-15G-24** 9–36V input/24V 15W candidate at Bulgarian specialist; full logic/timer, 24V-to-3.3V interface, load and stock checkout still unverified. **No complete 4-way onehot/no-repeat hardware chain selected or purchase-approved.**
- Official [Espressif N32R16V pin constraints](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html) and [R65 44-pin crosswalk](../Boat/docs/BOAT_R65_ESP32S3_PIN_RESOURCE_CROSSWALK.csv) impose GPIO35–37 memory, GPIO47/48 1.8V, board-revision RGB LED/strapping/USB reservations. Preliminary 22-signal pin budget is fully used by nRF, E22, GNSS, I2C, motors, watchdog and feedback. Four coil requests require premade I2C expander + independent physical safety gate; no direct energized EKM014 connection.
- Added R65 static regression in Boat host workflow; repository content cross-check passed 44 rows/22 proposals/0 reserved pins/0 duplicate pins. **GitHub CI outcome pending**, no firmware actuator pin map changed. No commissioning or operational release; Boat 014 / Remote 028 preserved. [R65 report](../Boat/docs/BOAT_R65_SAFETY_AND_PIN_BUDGET.md).
