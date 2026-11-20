# Repository Path Audit — Hardware Bench Tests

Date: 2026-11-20
Branch: `hardware-ready-dual-radio-r1`

## Finding
The R4/R2 hardware test-build documents and the master status refer to radio bench sketches as present, but the previously attempted paths were not retrievable and GitHub code search did not locate matching files. This is a documentation-to-repository discrepancy; it is not evidence of a hardware failure.

## Previously attempted paths (not found)
- `Remote/hardware_test/E22_SC16IS750_Bench/E22_SC16IS750_Bench.ino`
- `Boat/hardware_test/E22_UART_Bench/E22_UART_Bench.ino`
- `Remote/hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `Boat/hardware_test/nRF24_Bench/nRF24_Bench.ino`
- `Remote/source/radio_hw_config.h`
- `Boat/source/radio_hw_config.h`

## Corrective action
- Create explicit, minimal nRF24 bench probes in the documented hardware-test directories.
- Keep boat GPIO assignment disabled until the exact controller revision/pin crosswalk is checked.
- Use direct UART E22 bench testing on a UART-capable MCU first; SC16IS750 bridge testing is a separate remote test because its I2C bridge and wiring must be validated.
- Update master README/status to stop describing missing sketches as already implemented.

## Safety limits
Bench probes send only a fixed identification payload, do not encode actuator commands, and do not control motors, solenoids, or autopilot. Use antennas, low transmit power, short-range tests, and verify wiring before power-up. No field operation or range claims.
