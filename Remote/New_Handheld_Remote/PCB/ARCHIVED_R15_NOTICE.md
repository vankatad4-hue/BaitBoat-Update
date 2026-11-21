# R15 — Entire Remote PCB directory archived

All earlier TPS63070/TPS61023 feedback schematics, radio carrier PCB, keypad/joystick PCB, 5V/3V3 power routing and hardware pin drafts located here are **reference only**. Do not manufacture any board from these files. Owner's rule covers both Boat and Remote.

The **new active physical design** is prefabricated:
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard (onboard CS8501, TF),
- factory-made MCP23017 breakout (16 button inputs),
- two ADS1115 breakouts for four joystick axes + diagnostics (proposed 0x49 and 0x4A to avoid SC16 bridge at 0x48),
- separate off-the-shelf Pololu S9V11F3S5 3.3V for primary RF and S13V20F5 5V for remote E22,
- manufactured SC16IS750 breakout, nRF24 PA/LNA and E22 radio modules,
- genuine protected 1S4P battery pack, factory-made fuse/charger/terminal/cable assemblies,
- only the **plastic outer shell, grips, switch brackets** may be 3D printed.

Use [R15 active Remote plan](../../../docs/REMOTE_MODULE_ONLY_IMPLEMENTATION_R15.md), [Remote harness](../../../docs/REMOTE_MODULE_ONLY_WIRING_R15.csv), and [R15 master policy](../../../../docs/HARDWARE_MODULE_ONLY_POLICY_R15.md).

All pin mappings, module boot defaults, charging and RF compatibility require actual bench verification. Official releases remain withheld.
