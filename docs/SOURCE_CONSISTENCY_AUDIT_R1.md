# Source Consistency & Compile Preparation Audit R1

**Date:** 2026-11-20  
**Branch:** `hardware-ready-dual-radio-r1`  
**Audited head before changes:** `1f75a2ecb2d364d56340e0050c07d50e7e71cbe6`  
**Status:** Source review completed in part; target compilation and complete host test execution not performed. Remote portable host-test CI workflow has now been added; its result is pending.

## Scope and method

Reviewed the current Remote and Boat source inventory, the radio/protocol and transport modules, Remote UI/display/map/mission integration, portable test sources and the Boat host-test workflow. Findings are source-inspection observations; no complete checkout/build environment was available in this review, so this document does not claim compile or runtime validation.

## Findings

### F-01 — No reproducible target build manifest (BLOCKER)
There is no `platformio.ini`, Arduino CLI build configuration, dependency lockfile, or build script that pins the exact ESP32-S3 board/core, LVGL, Waveshare ESP panel library, RF24, and other dependencies. Remote display code explicitly rejects LVGL versions other than 9.6, but that requirement is not installed/pinned by project build metadata. Exact Waveshare/Heltec hardware revisions are also unresolved.

**Action required:** create a reproducible build definition after confirming exact board revisions and the vendor library/API versions; do not guess hardware variants.

### F-02 — Session-start handshake could strand after a lost first packet (FIXED IN SOURCE)
Remote originally sent its session-start marker once at startup. If the marker or the Boat reply was lost, the peers could remain unpaired for the entire boot. Boat also ignored same-session marker duplicates after the first one, preventing it from retransmitting a lost reply.

**Fix applied:** Remote retries authenticated session markers independently per link until that link's response is observed (nRF retry interval 500 ms; E22 interval 3 s and subject to the existing duty limiter). Boat responds idempotently to repeated authenticated same-epoch markers. This is source-level correction only; the retry/response path has not yet been built or integration-tested.

### F-03 — Authentication deliberately unavailable (BLOCKER BY DESIGN)
Both key-store implementations return false and clear output; no secure provisioning or persistent anti-replay mechanism exists. Radio command transmission/authenticated receive remains fail-closed. The session retry fix is therefore dormant until key provisioning is implemented.

**Action required:** design and review provisioning, storage protection, session uniqueness/replay protection, and reboot behavior before enabling command traffic.

### F-04 — Boat receive/queue/ACK path lacks integration tests (HIGH)
Portable tests cover codecs, sequence guard, queue, dedupe/policy and ACK/payload utilities. They do not exercise `BoatDualRadioPort` session reset, radio-origin provenance, cross-radio duplicate command IDs, queue-full ACK, ACK envelope sealing, or queue-clearing behavior end-to-end. ACCEPTED remains queue receipt, not execution.

**Action required:** add fake transport tests around the dual-radio port and explicitly verify ACCEPTED, DUPLICATE, BUSY and future EXECUTED lifecycle.

### F-05 — Host test automation is asymmetric (HIGH)
Boat portable tests have a GitHub Actions workflow with strict warnings. Remote tests are documented but no equivalent host-test workflow is present. Remote release-guard, telemetry, ACK tracker and payload codec test commands are not automatically run by the repository workflow.

**Action required:** add Remote portable-test workflow and ensure both workflows execute on changes to their source/test paths.

### F-06 — UI/library API compatibility and rendering remain unverified (HIGH)
Remote display adapter targets LVGL 9.6.x APIs and Waveshare/Espressif board classes; there is no locked library set or compile job. Main, mission, settings, calibration, map-manager and renderer callbacks are source-present, but they have not been compiled against the exact libraries or rendered in LVGL Pro. Callback propagation, repeated screen lifecycle, resize behavior, Cyrillic fonts, memory use and touch-target dimensions remain verification items.

**Action required:** pin the library set and run compile + LVGL simulator/pro checks; retain a screen/callback test checklist.

### F-07 — Arduino sketch does not instantiate Boat application/radio port (BLOCKER FOR OPERATION)
`Boat/source/firmware/Flytec_Boat.ino` is intentionally a safe placeholder. It does not call `BoatDualRadioPort`, consume the queue, apply policy, produce telemetry, execute commands or drive outputs. The source library files are not assembled into a target build by any checked-in build manifest.

**Action required:** implement the application executor as a separately reviewed stage; keep actuators disconnected and abstract outputs until policy, watchdog, telemetry and HIL gates pass.

### F-08 — E22 airtime accounting is RAM-only (HIGH FOR RF COMPLIANCE)
The 60-minute bucket limiter resets when the MCU restarts and uses estimated airtime. A reboot could reset the duty accounting window. The selected E22 profile and allowed band/duty rules still require country-specific review and actual airtime measurement.

**Action required:** persistent conservative duty accounting or a documented regulatory-safe boot lockout; verify radio profile and actual TX time before RF use.

### F-09 — Online map mode is a preference, not a renderer (KNOWN LIMITATION)
`mapSource_onlineAvailable()` always returns false. ONLINE can be selected in the UI, but no authorized online tile renderer/API credential integration exists. Offline rendering and staged import are still scaffolding and require SD pack validation.

**Action required:** keep ONLINE clearly unavailable in UI until provider integration and credentials are configured; do not represent it as live map data.

### F-10 — Unused compatibility hooks and placeholders (LOW / CLEANUP)
Remote radio adapter includes compatibility hooks such as `boatRadio_setTransport()` that are no-ops while the adapter uses fixed transports. UI physical controls and several telemetry/sonar/map functions intentionally display placeholders. These are not necessarily compile errors but should be annotated or removed when the architecture is frozen to prevent callers assuming the hooks work.

## Source-level consistency observations

- The 32-byte packed frame layout and CRC/HMAC field ordering are mirrored in Remote and Boat source. Portable codec tests use authentication/freshness stubs; they do **not** validate cryptography.
- Radio command encoder/decoder policy mirrors backup-link restrictions and rejects combined legacy releases.
- Remote display source has a strict LVGL 9.6 check; there is no manifest to guarantee that version.
- The map import validates staged manifest/index shape, ordered rows, declared count and RGB565 tile byte size, then copies in chunks. Failure paths are fail-closed; import integrity/atomic activation has not been tested on SD hardware.
- Mission storage uses CRC validation and SD/NVS fallback/migration source paths; filesystem power-loss behavior and actual SD hardware are not tested.
- No duplicate source definitions were proven by a complete compiler/linker pass. A source-level audit cannot substitute for compilation.

## Session-start retry verification plan

1. With a test harness and provisioned test key, drop the first Remote SESSION_START frame; verify retry causes Boat to establish the session.
2. Drop the first Boat SESSION_START reply; verify duplicate Remote marker causes Boat to resend and Remote marks that link confirmed.
3. Drop all nRF traffic while E22 is available; verify E22 marker retries obey duty limiter and the backup session is confirmed.
4. Drop E22 reply but keep nRF functioning; verify per-link state does not cause repeated resets of the established epoch.
5. Inject a different authenticated epoch and stale sequence; verify expected session-reset/replay policy is explicit.
6. Confirm no actuator path is connected and no command is executed during these tests.

## Gate before target compilation

- [ ] Exact Remote display SKU/revision and board package confirmed.
- [ ] Exact Boat ESP32-S3 board revision and pin/resource audit frozen.
- [ ] Dependency versions and build configuration pinned.
- [ ] Remote host tests run and CI workflow added.
- [ ] Boat host workflow observed green on current source.
- [ ] Session retry fix integration test added.
- [ ] Compiler build of all Remote source modules and Boat transport modules attempted.
- [ ] Compile diagnostics corrected and retained in build log.
- [ ] No claims of target/hardware validation until an actual board test is recorded.

## Change made during this audit

- Remote session-start markers now retry on each configured radio until that radio's reply is seen.
- Boat same-session marker handling now re-sends its response for idempotent retries.
- No release marker was bumped; these are development-branch source changes, not a hardware-ready release.
