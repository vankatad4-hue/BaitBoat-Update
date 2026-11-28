# Continuation checkpoint — R30A regression and build verification

Updated: 2026-11-28
Branch: `platformio-unified-r30h20`
Starting source tree: `7f36eb9b5c4c0a962cf3819c9d40c657c11f14e6`

## Completed locally

- Fixed the R30A assembly validator regression. `SONAR_UART_RX` remains mapped
  from Open Echo coprocessor TX to Boat ESP32-S3 GPIO21 and is allowed its
  documented `R30H124W` authority label; the endpoints must still match the
  R23 schedule. A regression test rejects unauthorized label changes.
- R30A validator passed; all 8 R30A host tests passed, including 7 negative
  mutation cases and the sonar UART reclassification case.
- Offline map planner host tests passed (6 tests). The full offline-map tool
  suite could not run locally because Pillow is not installed.
- Restored the offline-map workflow's pull-request self-trigger and made its
  test step explicitly identify the planner and MBTiles import/export coverage.
- Added Sonar ADC acquisition and DBT UART contract tests, plus CI jobs for the
  host test and all four DevKitC V4 PlatformIO compile profiles.

## Verified GitHub Actions results

- [Boat portable host tests #46133216284](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46133216284): **SUCCESS**, including the corrected R30A validator and all 8 R30A tests.
- [Primary PlatformIO compile #46133216294](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46133216294): **SUCCESS** for both Boat and Remote target jobs.
- [Sonar host and target tests #46133216583](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46133216583): **SUCCESS** for the acquisition/DBT UART host test and all four DevKitC V4 compile profiles (base, ADC, SPI, UART).
- [Offline Map Tool Tests #46133216280](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46133216280): **SUCCESS** for the complete discovered Remote tool suite, including map planning and MBTiles export.
- Local offline map planner tests also passed (6 tests). The local full suite lacked Pillow, so its complete result is taken from GitHub Actions.

## Hardware boundary

All builds and tests are software-only. Sonar ADC/SPI/UART hardware remains
disarmed, PlatformIO upload remains denied, and no sensor timing, voltage,
transducer, or water-depth behavior has been physically verified. The local
PlatformIO attempt could not install the ESP32 platform because the sandbox
blocked downloads and PlatformIO could not write its user cache; CI supplied
the target-build verification instead.
