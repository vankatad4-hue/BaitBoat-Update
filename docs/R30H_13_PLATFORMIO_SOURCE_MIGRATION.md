# R30H-13 — VS Code + PlatformIO Boat/Remote source migration audit

**Date:** 2026-11-23. **Mode:** uncommissioned source-only compilation; **not** a hardware acceptance, no radio transmissions, no actuator enabling. Boat **VER014**, Remote **VER028** remain locked. This is the latest authoritative software layout handoff; archived older notes are historical.

## Scope and verified source inventory

The source auditor `Boat/tools/verify_platformio_source_layout_r30h13.py` scans **all active source files** (`.c/.cpp/.h/.ino`) in both endpoint trees: **104 Boat + 104 Remote = 208**, with **60 top-level compile roots** in the existing static source inventory. It verifies **17 intentionally mirrored common-source pairs** byte-for-byte, including authenticated frame protocol, mission wire codec, session state, UTC policy, emergency diagnostic schema and E22 header. These duplicates remain intentionally in separate endpoint trees so each PlatformIO project and Arduino reference can build without a fragile cross-project symlink. They are **not** unneeded duplicates to delete blindly.

The static reference inventory `Boat/tools/audit_source_inventory_r30h10.py` reports just one unused header as a **review candidate**: `Boat/source/tracker_sensor_profile.example.h`. It is an intentionally disabled/example file needed as documentation and MUST NOT become a physical commissioned config. Its status is not a compile blocker. Five quote-style includes remain under manual review (third-party/externally supplied header paths); verified PlatformIO target builds are the arbiter of actual compatibility.

**Meaning of "rewrite all files for VS Code + PlatformIO":** all 208 active sources are part of the audited independent target projects and compiler/CI workflow; the files with a concrete incompatibility or dead dependency were refactored. We deliberately did **not** mechanically reformat/replace 208 valid files: that would introduce regression risk in STOP, release, RTH, authentication and LVGL, without solving any target issue. Every changed source is versioned in GitHub; existing working modules and their regression tests are retained.

## Active project layout

| Endpoint | PlatformIO project | Active source entry | Board/flash | Radio | Status |
|---|---|---|---|---|---|
| Boat | `Boat/platformio.ini` | `Boat/source/platformio_entry.cpp` includes the guarded nested `firmware/Flytec_Boat.ino` | ESP32-S3 N32R16V, 32 MB OPI / 16 MB PSRAM, diagnostic only | Boat `RF24@1.4.11` direct SPI; GPIO/physical audit still unsigned | Source compiled in PIO; no physical drivers |
| Remote | `Remote/platformio.ini` | `Remote/source/Flytec_Remote.ino` at PIO source root; no second adapter | Waveshare 4.3 Standard, 16 MB QIO / OPI PSRAM | I²C `0x28` SC18IS606 external SPI + existing MCP23017 `0x21` GPA0 CE; no RF24 direct dependency | Source compiled in PIO; all radio gates 0 |

**Defunct Remote RF24 source:** removed `Remote/source/nrf24_link.cpp` and `nrf24_link.h` from compilation and preserved byte-identical historical files under [retired Remote RF24](../Remote/archive/2026-11-23/R30H_13_direct_RF24/README.md). Dropped dead Remote `nrf24/RF24` PlatformIO dependency and old Remote Arduino CLI `RF24@1.4.11` install. The Boat remains on active RF24 because its pin plan is different. Remote GPIO6 is still TF dummy CS, GPIO15 RS485, TF SPI11–13 is never shared with bridge SPI.

**Boat transport fault hardening:** `Boat/source/nrf24_link.*` now limits indefinite `txPending_` to **120 ms** of monotonic elapsed time (including millis wrap). Upon failure it rejects subsequent frames and clears radio readiness; `boat_dual_radio_port.cpp` immediately revokes primary freshness and discards queued primary-only commands/stick lease. Test `Boat/tests/r30h13_boat_nrf_timeout_test.cpp` runs with a fake RF24 interface, checking normal ACK, MAX_RT, stalled TX and wrap. **120 ms is a source policy, not measured RF STOP response time.**

**R30H-12 bridge code retained:** bounded post-call I²C response guard, no duplicate non-idempotent SPI writes, independent prebuilt SC18IS606 SPI. The MCP23017 active-HIGH CE latch can remain HIGH through MCU reset/I²C failure, so hardware reset supervision is still **BLOCKED**. Timeouts never count as an independent hardwired STOP.

## VS Code practical workflow (Windows/Linux)

1. In **VS Code**, open [Flytec_V080.code-workspace](../Flytec_V080.code-workspace) with installed PlatformIO IDE / Python; this is a **multi-root** workspace with Boat, Remote and docs. Built-in tasks run source audit, locked safety preflight, independent Boat build, independent Remote build or a sequential combined diagnostic build. Every task names **NO UPLOAD**.
2. To generate the Bulgarian LVGL font **locally**, run the `Flytec: generate licensed Remote LVGL font` task with **your own licensed TTF/OTF and the relevant license text file**, or:
   ```powershell
   python Remote/tools/stage_platformio_font.py --font "C:\\path\\to\\licensed-font.ttf" --license "C:\\path\\to\\FONT_LICENSE.txt"
   ```
   The source font is never committed. Only locally generated `Remote/source/flytec_bg_14.c` and `.generated.h` are staged; they are `.gitignore`d. **Do not share a proprietary font file**.
3. From repository root:
   ```sh
   python Boat/tools/verify_platformio_source_layout_r30h13.py --json out/platformio_source_audit.json
   python Boat/tools/verify_r30h10_physical_lock.py
   python Boat/tools/verify_r30h10_gpio_map.py
   pio run -d Boat -e boat_n32r16v_diagnostic
   pio run -d Remote -e remote_waveshare_43_diagnostic
   ```
   Both project roots intentionally deny `pio upload`. Builds are diagnostic, not production flashes.
4. GitHub Actions [Primary PIO Boat+Remote](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/platformio-migration-build.yml), [Boat host tests](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/boat-host-tests.yml), [Remote host tests](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/remote-host-tests.yml) and [source audit](https://github.com/vankatad4-hue/BaitBoat-Update/actions/workflows/source-audit.yml) provide source-only evidence.

## PIO audit invariants

- Both endpoints pin `espressif32 @ 8.0.0`, Arduino framework, board-specific flash/PSRAM settings and locked custom upload-denial command; `BOARD_HAS_PSRAM` is configured.
- Only one `setup()/loop()` per endpoint. Boat compiles its .ino through **one guarded adapter** while excluding the nested sketch. Remote compiles its own .ino directly and **must not** add a second `platformio_entry.cpp`.
- Source checks detect missing/reintroduced direct Remote RF24 code/library, unsigned RF pin/audit flags, removed Boat nRF TX watchdog, active source integrity and shared protocol drift.
- CI builds both endpoints independently and runs portable hardware-free tests. The Arduino CLI core **3.0.2** remains a separate **reference** compatibility workflow, not the primary development target.
- Existing hard physical evidence ledgers remain exactly **R30H-10: 12 NOT_RUN + 6 BLOCKED**, **R30H-12: 7 NOT_RUN + 5 BLOCKED**. Security keys and board-specific commissioned configs are not staged. Actual STOP-current-zero/independent watchdog, GNSS/PPS, LoRa and nRF RF measurements are still missing.

## Release policy / remaining issues

**No official VER015/VER029**, no RF pin audit unlock, no `pio upload` or live boat motor/solenoid enable. Future engineering can use the existing PIO codebase, but requires physical evidence of board pin/voltage/CS, CE reset-safe behavior, antenna and RF airtime, dual-radio loss/STOP latency with one analyzer clock, independent Boat current-zero STOP/pulse-limiting, sensors and GNSS PPS (if physically available). The selected CHIRP sonar remains deferred.

Current target CI **must be checked at the actual final commit** before claiming a fresh PASS; older successful builds do not automatically certify a newer diff. Revalidate the safety CSVs with `Boat/tools/verify_r30h10_physical_lock.py`, not by fabricating review dates.
