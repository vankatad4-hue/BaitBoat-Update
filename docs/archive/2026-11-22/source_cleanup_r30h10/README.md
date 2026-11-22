# Source cleanup archive — R30H-10 (2026-11-22)

Historical source code moved out of the active Boat/Remote source trees. No archived file is a current pin map, production source, approved firmware, or commissioned hardware configuration. Use the active `Boat/source/boat_radio_hw_config.h`, `Remote/source/radio_hw_config.h` and R20/R27/R29 authority documents.

- `Boat/source/config/boat_config.h` — early R36 controller selection macros, never imported by current targets.
- `Boat/source/config/radio_config.h` — early R02 nRF TODO values superseded by authenticated dual-radio code.
- `Remote/source/boat_config.h` — obsolete layout sizes; actual LVGL UI uses local screen geometry.
- `Boat/source/heltec_sx1262_link.cpp` and `.h` — retired integrated-Heltec SX1262 implementation, superseded by external E22 UART; no longer compiled or linked.

All changes remain uncommissioned. Physical RF STOP/PPS tests, actual GPIO signatures, R30A independent cutoff/current-zero, and the common-3S dummy-load gates remain NOT_RUN. Boat VER014 / Remote VER028 unchanged.
