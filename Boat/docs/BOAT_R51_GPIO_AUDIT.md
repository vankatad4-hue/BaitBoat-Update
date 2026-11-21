# R51 — ESP32-S3 N32R16V GPIO output audit

Added `Boat/source/esp32s3_gpio_audit.h` and `Boat/tests/esp32s3_gpio_audit_test.cpp`. Rejects boot straps (0,3,45,46), native USB (19,20), default UART0 console (43,44), octal memory (35,36,37), and WROOM-2 GPIO47/48 1.8V pads as direct 3.3V motor control outputs. Requires signed pin audit and non-overlapping allowed GPIO mask.

Proposed direct MDD20A pins from R45: PWM 39/40, DIR 41/42, watchdog request 2, permit request 21. These are **provisional** and must be physically cross-checked with exact module headers, boot behavior, actual board schematic and external wiring before commissioning.

R50 diagnostic target run 44948745058 PASS. R50 host run 44948760293 FAIL at R49 EKM014 regression; test timestamp corrected in later commit 9c216df049eee2e30edbc85636c955e8b3f0845c. R51 CI rerun required. No production GPIO outputs are enabled. No EKM014 physical interface is connected.

Next: actual COTS EKM014 I/O level/polarity datasheet, independent STOP/watchdog/contactor component selection, safe boot input pull design, real HAL and BoatApplicationPort attachment, authenticated Remote calibration command, GPS figure-eight and calibration persistence. No physical acceptance or new release.
