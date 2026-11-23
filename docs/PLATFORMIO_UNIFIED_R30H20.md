# R30H-20 · Unified VS Code + PlatformIO development branch

**Date:** 2026-11-23  
**Branch:** `platformio-unified-r30h20`  
**Preserved parent:** `hardware-ready-dual-radio-r1`, base commit `e0b6220d6afaf8721fbb8dc1f2424f6abfd9b726`.

## Project files carried into the new branch

The Git branch starts as an exact tracked-file snapshot of the complete parent tree. It contains Boat firmware/hardware, Remote firmware/UI/CAD, ready-made module wiring, docs, SD map tools, CI, existing ZIPs and the previous release archives. No historical code was silently deleted or re-generated from approximations. Active source roots remain `Boat/source` and `Remote/source`, not the archived legacy versions.

The reproducible all-project ZIP is now generated with `python tools/flytec.py snapshot`, which reads *every* Git-tracked file and creates `out/Flytec_Project_PlatformIO_R30H20_source_snapshot.zip`, its SHA-256 digest and an embedded `FLYTEC_SOURCE_MANIFEST.json` listing the file paths, sizes, SHA-256 hashes and exact Git commit. The project archive is published as a GitHub Actions artifact by the **Package Flytec project ZIPs** workflow on this new branch. The existing Boat and Remote development archive artifacts are kept, without increasing the frozen official version numbers.

Security: Git-untracked local keys, commissioning profiles, `.pio` output, generated font binaries and temporary files are deliberately not packaged. The source ZIP is **not** a signed firmware release.

## Developer workflow (Windows · VS Code)

Open `Flytec_V080.code-workspace` in VS Code with the PlatformIO IDE and C/C++ extensions. The real build configurations are:

| Target | PlatformIO environment | Hardware profile | Output restriction |
|---|---|---|---|
| Boat | `boat_n32r16v_diagnostic` | ESP32-S3-DevKitC-1 N32R16V | compile only; upload denied |
| Remote | `remote_waveshare_43_diagnostic` | Waveshare ESP32-S3-Touch-LCD-4.3 Standard | compile only; upload denied |

Both target configurations retain pinned Espressif/Arduino/LVGL libraries, build filters and PSRAM settings. Old Arduino CLI build workflows are **reference regressions** only; development does not create another duplicate `setup()/loop()`.

```powershell
python tools/flytec.py audit
python tools/flytec.py test-host
python tools/flytec.py build-boat
python tools/flytec.py build-remote
python tools/flytec.py build-all
python tools/flytec.py snapshot
```

`audit` validates both real compile roots, the named target environments, sources/safety checks, upload denial, version files and the new-branch workflows. `test-host` compiles the portable R30H-18, R30H-19 and R30H-20 STOP matrices. `build-all` audits and invokes `pio run` on both endpoints; **never flashes**.

Remote's Bulgarian LVGL font still requires a locally licensed source; run the VS Code font-staging task or the repo's font script. CI obtains a licensed Linux font and converts it during the Remote target build. Do not commit/upload raw font files.

## R30H-20 — actual 32-byte authenticated STOP frame tests

`tests/r30h20_radio_frame_fault_matrix_test.cpp` compiles once against Boat's actual `radio_protocol.cpp` and wire payload decoder and once against Remote's exact equivalents, rather than testing only a mock frame. The small test-only `tests/r30h20_stubs/mbedtls/md.h` adapts mbedTLS HMAC to OpenSSL **solely in the host test**; target PlatformIO builds use their production cryptography implementation.

Covered source-only fault paths: CRC corruption, HMAC corruption despite repaired CRC, peer source/destination mismatch, radio flag mismatch, epoch and key rollover, duplicate/replayed/out-of-order sequence, dropped ACK, wrong STOP command ID, stale primary STOP status, unknown physical status, and authenticated matched ACK plus subsequently fresh stopped STATUS. No actual RF or hardware measurements are asserted.

**Additional bug corrected:** previous Boat `boatHardware_status()` set `safetyFlags |= 0x80` on critically low battery, but both Boat and Remote status codecs mark bit 7 reserved and *reject* that status. The critical battery still sets existing low-battery flag bit 5 (`0x20`), percent indicator and independent local battery/RTH logic. Removed only the invalid reserved bit. R30H-19 STATUS outbox rejects invalid fields before queueing, **clearing any previously queued STOP state when a newer sample is invalid**, preventing a stale or invalid/renewing status from starving the control TX slot. The new pure-host test builds the **actual BoatHardware_status()** for a CRITICAL battery and verifies LOW-bit wire encoding; invalid or watchdog-failed output feedback must still report UNKNOWN, never a fake stopped reading.

The branch workflows `boat-host-tests.yml`, `remote-host-tests.yml`, `source-audit.yml`, `platformio-migration-build.yml` and `package-flytec-zips.yml` now target `platformio-unified-r30h20`. Read the **final** CI outcome for the exact code SHA. An earlier parent-branch PASS cannot be presented as acceptance of the new commit.

## Hardware and release gates preserved

- **R30H-10:** 12 NOT_RUN / 6 BLOCKED.
- **R30H-12:** 7 NOT_RUN / 5 BLOCKED.
- Independent STOP/current-zero, E22/nRF bridge, GPIO, RF-only latency and motor/solenoid loads remain unverified.
- GPIO signoff stays disabled. Firmware upload and energizing propulsion/solenoid hardware remain disallowed.
- **Boat VER014 / Remote VER028** — unchanged.

## Next stage after compilation

Continue R30H-20 with state-machine-level end-to-end STOP receipt/feedback tests, including high TX load and replay/epoch rollover. Run the real passive GPIO and RF-only bench test **only after** the designated physical modules are assembled and inspected. Do not infer physical STOP or zero-current from source-only ACK/STATUS simulations.

## R30H-21 follow-on (2026-11-23)

The active PlatformIO branch now includes R30H-21 primary TX pre-admission sequence guarding, an authenticated 1,024-frame simulated load matrix, regression source mutation and safe source snapshot packaging. [Details](R30H_21_PRIMARY_TX_BUSY_SEQUENCE_BUDGET.md). No hardware permission or official release change.

## R30H-22 two-endpoint STOP/ACK/STATUS (2026-11-23)

The new branch adds a single timeline simulator using both actual endpoint wire codecs and Boat/Remote STOP retry/ACK/STATUS classes, with injected link faults and physical-status UNKNOWN. Boat/Remote host and source audit passed on `e1475248`; see [R30H-22 engineering report](R30H_22_DUAL_ENDPOINT_STOP_SCHEDULER.md). Do not treat this as a real RF/STOP current-zero measurement or commissioning permission.

## R30H-23 failsafe integration (2026-11-23)

The branch adds Boat local dual-radio-loss conditional Home, GPS/heading/Home, critical battery/INA231, physical-safety feedback and interrupted-coil simulation, plus host and PlatformIO CI jobs and source mutation audit. No GPIO admission or official release change. [Read the R30H-23 test report](R30H_23_LOCAL_FAILSAFE_SIMULATION.md).

## R30H-24 (2026-11-23): last-actuator navigation witness and clock-wrap/real adapter tests

The code branch now includes a conservative Boat output-gating fix, two portable tests, host/PlatformIO steps, source audit and mutation regression. [Technical report](R30H_24_WRAP_NAV_INPUT_APPLICATION_BATTERY.md). No live motor/coil/antenna or physical acceptance claim.
