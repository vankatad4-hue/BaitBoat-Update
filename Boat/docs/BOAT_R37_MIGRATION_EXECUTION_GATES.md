# R37 — R36 migration execution and remaining acceptance gates
2026-11-21. Source inspected: boat_dual_radio_port.cpp/.h, e22_uart_link.cpp/.h, firmware/Flytec_Boat.ino.

## Completed in repository
- R36 ESP32-S3-DevKitC-1 + HGLRC M100 PRO + E22 UART selected in config, BOM and checkpoint.
- R37 fail-disabled E22 UART board adapter in source/e22_uart_board_port.h; rejects unverified/duplicate/invalid pins. This is an adapter scaffold, NOT wired into BoatDualRadioPort.
- Existing E22UartLink already implements UART framing, configuration and duty budget, but has no verified hardware tests on selected E22.
- GitHub Actions prior R34 diagnostic ESP32-S3 target and portable tests were successful; R36/R37 target results are not independently confirmed here.

## Blocking source integration
1. Replace BoatDualRadioPort's HeltecSx1262Link with E22UartBoardPort and audited HardwareSerial. Preserve authenticated handshake, frame signing, replay prevention and cross-radio sequence semantics. Current code uses SX1262 and must NOT be represented as E22-compatible.
2. Check E22 transparent-mode framing, packet fragmentation, RX parser resynchronization and actual RF pairing with Remote E22; verify real frequency and regulatory duty limits. E22 test profile's fixed 25ms airtime estimate is NOT validated for all payload/air-rate settings.
3. Verify HGLRC M100 PRO exact GPS output protocol, UART and optional compass. Adapt tracker sensor port and add bench NMEA tests.
4. Select exact DevKitC-1 variant, freeze non-conflicting pin assignments, power/boot/PSRAM and watchdog pins. Validate clean nRF 3V3 supply.
5. Select commercially built safety STOP, contactor, watchdog and solenoid onehot/time-limit devices, measure current, complete wiring and fault tests.
6. Compile selected ESP32-S3 firmware, run host and RF loopback tests, then physical integration and acceptance. No sonar implementation pending selection.

## Safety and release
No custom PCB, no second MCU. Existing diagnostic sketch keeps actuator GPIO disabled. No claims of full working hardware, physical safety verification or release readiness.
