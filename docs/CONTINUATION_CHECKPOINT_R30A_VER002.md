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

## Awaiting CI verification

- Boat and Remote PlatformIO builds and the complete offline-map suite are not
  yet verified on the latest change. Local PlatformIO could not install the
  ESP32 platform: the sandbox blocked network access and PlatformIO rejected
  writes to its user cache.
- Sonar acquisition/UART host tests and the four target compile profiles are
  added but not yet executed. Their results must be read from GitHub Actions
  before calling them passed.

## Hardware boundary

All builds and tests are software-only. Sonar ADC/SPI/UART hardware remains
disarmed, PlatformIO upload remains denied, and no sensor timing, voltage,
transducer, or water-depth behavior has been physically verified.
