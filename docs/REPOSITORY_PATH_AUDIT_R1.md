# Repository Path Audit — Hardware Bench Tests

Date: 2026-11-20
Branch: `hardware-ready-dual-radio-r1`

## Corrected finding
The earlier fetch attempt returned 404 for anticipated test paths. A fresh path-by-path fetch succeeded: **all four bench sketches and both hardware-test READMEs exist at the paths below**. The initial error was a retrieval/path-check issue, not missing project files. No sketches were recreated or overwritten.

## Verified files
- `Remote/hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `Boat/hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `Remote/hardware_test/E22_SC16IS750_Bench/E22_SC16IS750_Bench.ino`
- `Boat/hardware_test/E22_UART_Bench/E22_UART_Bench.ino`
- `Remote/hardware_test/README.md`
- `Boat/hardware_test/README.md`

## Status after inspection
- Remote nRF probe uses provisional GPIO6/15 and SPI GPIO11/12/13; shared SPI/SD and RS485/Sensor AD conflicts still need bench isolation and exact revision verification.
- Boat nRF sketch is intentionally disabled until CE/CSN pins are assigned.
- Remote E22 test uses SC16IS750 I2C bridge at 0x48 and GPIO8/9; config/readback is attempted.
- Boat E22 sketch is intentionally disabled until Tracker UART and M0/M1/AUX pins are assigned.
- All probes are bench-only and do not contain actuator execution; compile and bench operation remain unverified.

## Safety
No motors, solenoids, or autonomous control during radio tests. Antennas must be attached; start at low power and short range. No field use until regulatory/duty-cycle and link-safety checks are complete.
