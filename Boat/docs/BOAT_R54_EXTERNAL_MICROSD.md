# R54 — external microSD module (no integrated ESP32-S3 SD slot)

**Architecture:** ready-made 3.3V SPI microSD breakout with socket; no custom PCB and no second MCU. Use a quality module with true 3.3V logic, suitable 3.3V supply and accessible CS/SCK/MOSI/MISO/GND pins. Avoid unverified 5V-only SD adapters or resistive-divider modules until signal integrity is measured.

## Provisional SPI wiring (not a signed wiring release)

| External SD | ESP32-S3-DevKitC-1-N32R16V | Notes |
|---|---|---|
| SCK | GPIO12 | Shared nRF24 SPI SCK |
| MOSI / DI | GPIO11 | Shared nRF24 SPI MOSI |
| MISO / DO | GPIO13 | Shared nRF24 SPI MISO; verify module MISO tri-states when deselected |
| CS | GPIO33 **provisional** | Dedicated SD CS; verify physical header and octal-memory pin reservations on exact board |
| VCC | regulated 3.3V | Size for SD peak current; local decoupling |
| GND | logic ground | Keep away from motor/coil high-current return |

nRF24 CSN remains GPIO9; CE GPIO10. Both CS lines require verified inactive-high behavior at boot, and both devices must share an SPI bus lock/transaction discipline. A card insertion/removal while running must not disrupt radio timing. Prefer read/write only during maintenance/idle; never block safety watchdog or motor command lease with SD I/O. An independent physical STOP is mandatory.

## Implemented
- `Boat/source/boat_sd_module_profile.h`: audited GPIO, CS separation, 3.3V verification and conservative SPI frequency.
- `Boat/source/boat_sd_module_arduino.h`: Arduino SD.begin/read/write diagnostics, disabled until explicit profile provided.
- `Boat/source/boat_sd_calibration_backend.h/.cpp`: fixed-file slot A/B bridge to `Figure8CalibrationStore`.
- `Boat/tests/boat_sd_module_profile_test.cpp`: host validation in CI.

## Hold points
- Confirm exact purchasable module and datasheet; check CS pullups, tri-state MISO, regulator/logic voltage and current.
- Confirm GPIO33 physical accessibility and octal-memory board restrictions; verify sharing with nRF SPI library transaction and nonblocking watchdog.
- Current FAT file writes are **not guaranteed atomic**. Two independent CRC-validated files preserve a previous record if one write tears, but card/filesystem corruption may still affect both. Test card removal, full card, interrupted writes, radio timing, FAT recovery and safe startup before activating.
- Code is a hardware adapter, **not attached to the running Boat sketch**. Physical card, bench measurements, target compilation of enabled SD path and integration tests remain pending.
- Boat VER014 unchanged, sonar deferred.
