# Flytec V080 physical commissioning and acceptance (R1)
Status: NOT RUN — no physical hardware or bench measurement evidence has been supplied. The software snapshot is diagnostic only. Neither successful compilation nor host simulation authorizes connecting actuator power or water use.

## Record before testing
- Date, operator, hardware revision and manufacturer part numbers; actual remote Waveshare standard SKU, Boat Tracker revision, radio boards, carrier revisions, motor driver, four release coils, battery/BMS and fuse ratings.
- Frozen source commit, both target-build URLs, toolchain, firmware hashes, archive SHA256 and source manifests.
- Exact schematic and keyed connector pinout: power/ground, voltage limits, boot/strapping pins, SPI/I2C/UART, GNSS, compass, motor and coil paths, protection and hardware STOP. Verify against actual board documentation and continuity with power off.
- Test equipment: current-limited bench supply, DMM, oscilloscope or logic analyzer, independent output/current feedback and emergency electrical cutoff. Disconnect motor/solenoid power for early tests; remove propellers before powered motor bench checks. Do not assume the existing release-test fixture pulse durations are coil ratings.

## Mandatory staged acceptance matrix

| ID | Test | Required evidence / pass condition | Result |
|---|---|---|---|
| P01 | Inspect mechanical/electrical assembly | Correct connector polarity, fuse, clearance, no shorts, separated power/sensor wiring; signed pinout | NOT RUN |
| P02 | Logic power, no actuator supply | Safe boot/reset outputs; rails within actual part ratings; no GPIO glitch enabling output | NOT RUN |
| P03 | Independent cutoff | Verify physical STOP and independent watchdog remove power with MCU deliberately unresponsive; oscilloscope trace and measured latency <= 100 ms design requirement, or redesign before enabling | NOT RUN |
| P04 | Local input provider | Invalid/missing sensor, stationary/current/idle feedback and safety sample older than 100 ms inhibit outputs; measured timestamps from local sensors only | NOT RUN |
| P05 | Secure provisioning | Review exact SDK configuration, secure storage, entropy source and irreversible-security implications; owner approval before any production security changes; never log/share root key | NOT RUN |
| P06 | Pairing and replay | On disconnected-power bench: correct and wrong key, lost handshake confirmation, duplicate/reordered frame, cross-link injection, independent boot and rekey; reject unauthenticated commands | NOT RUN |
| P07 | Touchscreen | Verify 800x480 layout, touch calibration, all physical buttons, joystick calibration, STOP priority, EN/BG lower/upper keyboard including all 30 letters and lake names | NOT RUN |
| P08 | SD | With actual formatted SD and representative eight lake profiles, verify map import, rollback, tile rendering, pan/zoom, 16 points per lake, removal, boot and interrupted reads; log p50/p95/max tile and UI loop latency, PSRAM usage and packet gaps; no missed STOP deadlines | NOT RUN |
| P09 | GNSS/compass | Confirm 3D fix, satellites, measured accuracy, known latitude/longitude and heading; calibrate compass away from motor-current interference. Reject invalid or stale GNSS/heading (2 s / 300 ms maximum software sample ages) | NOT RUN |
| P10 | Battery | Compare reported millivolts against calibrated meter; verify divider/ADC range, low battery and BMS behavior under representative load. Commission state-of-charge estimate and brownout behavior | NOT RUN |
| P11 | Motor test, propellers removed | Check independent left/right direction, reverse, neutral, current and safe demand limits. Test 200 ms receive lease, link loss, reset and STOP. No movement until neutral and verified safety | NOT RUN |
| P12 | Release test individually | Measure each coil's current, inductive suppression and safe pulse duration using current-limited supply and actual driver; verify correct channel, one-at-a-time interlock, motors off, measured idle feedback after cutoff; confirm mechanical release separately | NOT RUN |
| P13 | Release fault tests | Held button, duplicate command, wrong channel, overlap, STOP, link loss, stale idle sample, failed power-off and thermal/current cutoff; no repeat pulse or simultaneous motors | NOT RUN |
| P14 | Bench system integration | Verify actual authenticated telemetry, ACK vs physical completion, manual transitions, temporary override/cancel, new session invalidating old commands; no unexpected output | NOT RUN |
| P15 | Restrained supervised water trial | Safe test zone, nearby retrieval method and observer; manually verify steering, GPS/heading stability, waypoint arrival and STOP before testing autonomy | NOT RUN |
| P16 | Autonomous trial | Verify outbound and return route stays within commissioned water geofence, selected release sequence, no release on link loss, arrival radius and manual override | NOT RUN |
| P17 | Radio loss/reconnect | Deliberate controlled nRF loss, LoRa loss and both links lost, session timeout/rekey while mission active; only safe locally authorized Home return; STOP and unsafe sensor/actuator states dominate; no automatic restart of outbound | NOT RUN |
| P18 | Final regression and release | Freeze source; all Remote/Boat host groups and matching targets passed, physical evidence above signed, archive manifests and hashes independently checked, one version increment each | NOT RUN |

## Stop conditions
Any unexpected motor or coil movement, failed hardware watchdog, overheating, overcurrent, loss of reliable sensor samples or unverified electrical feedback ends the test immediately. Disconnect power and record the defect. Do not bypass the default-deny key or driver gates to make a test pass.

The current driver contract is not an implementation of specific sensor or power electronics. Install/verify the board-specific drivers and hardware cutoff before registering them. Manual release ACK confirms an observed deenergized completion of an accepted pulse, not proof of a bait/hook mechanically separating; verify the mechanism independently. The full water-return path and geofence are not obstacle avoidance. Do not perform an unsupervised first-use test.

## Evidence record template
Test ID | source commit / firmware SHA256 | board/driver rev | setup | actual measured values | PASS/FAIL/BLOCKED | log/trace/photo path | date/operator | corrective action and retest.

Official versions remain Remote 028 and Boat 014 until acceptance. The next planned versions are Remote 029 and Boat 015. No test above is marked passed based on a simulated driver.


## Owner design decision — Heltec-only MDD20A restoration (R5)
The owner explicitly rejected the RP2040 companion and restored the original Heltec Tracker + Cytron MDD20A direct PWM/DIR design. This supersedes the R4 companion recommendation and all companion development commits. Removed companion Pico SDK source, wired protocol/master/safety core, companion-only tests and Actions workflow from the active branch; Git history retains them. Kept Tracker GNSS/IMU/ADC acquisition, output planner, existing radio/session/mission/release/telemetry and independent electrical watchdog/STOP requirements. Added a fail-closed direct GPIO profile validator and regression; diagnostic sketch remains unassigned. New active R5 design and pin schedule supersede R4. No MCU pins, expander part, high-current cutoff, or PCB frozen: authoritative V1.2 pin map and exact GPIO budget unresolved. Four independent solenoids retained. Remote source unchanged; release version markers Remote028/Boat014 retained. Re-run Boat target, 21 host groups and ZIP packaging; record actual results. All physical acceptance gates remain open.


## R6 continuation — direct MDD20A HAL, pin budget, sonar deferred (2026-11-21)
- Boat target #20 (run 44814101124, job 136557839493, prior approved R5 source) PASSED. Program 358,009/1,310,720 bytes, global RAM 20,600/327,680 bytes (307,080 remaining); tooling emitted only external esptool/Node warnings.
- 3fa3799 adds `mdd20a_arduino_stage.*` (Arduino-ESP32 3.x LEDC 20 kHz/10 bit, two channels, direct PWM/DIR, initial PWM/permit/WDI LOW), and `mdd20a_direct_safety.h` with local measured feedback freshness and latched failure; **driver is NOT instantiated or attached to active Boat firmware, no GPIO selected and no power output authorized**. 3d1a755 makes motor→coil interlock enforce the existing shared output planner; d70028f corrects a test expectation at exactly the 30 ms post-release deadtime boundary. Full target and host verification of these new changes must be checked separately before asserting pass.
- Added R6 direct GPIO budget and electrical capture checklist: independent TPS3431/safety cutoff, Heltec native MDD PWM/DIR/WDI/permit, optional non-programmable I/O expansion only for coil/feedback, and strict signed pin map gate. Exact V1.2 PCB/schematic not supplied, final GPIO allocation blocked; current work is an interface skeleton, **not native KiCad schematic or ERC**. Power, motor and coil values remain commissioning candidates only.
- **Sonar is intentionally only planned** in Boat/docs/BOAT_SONAR_INTEGRATION_PLAN_UNDECIDED_R1.md. Owner has not selected a model. No sonar code, new display, GPIO, transducer wiring or chosen transport. One existing 4.3-inch 800×480 Waveshare screen, real CHIRP-like fish arch echogram, manual/autopilot, 50/50 map/sonar view and optional full-screen mode remain requirements; actual data protocol, electrical budget and high-throughput link depend on selection.
- Prior Remote #87 and unchanged Remote software remain verified; official markers Remote VER028 / Boat VER014 unchanged, no physical acceptance or release authorization. Next: check latest target and host CI, correct errors, freeze the exact Heltec V1.2 pin map and safety hardware/feedback, and capture a native MDD20A/power/coil KiCad design after verified choices. No physical actuation before bench proof.
