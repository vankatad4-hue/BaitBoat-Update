# R49 — EKM014 coil interlock increment

Added `Boat/source/ekm014_pulse_guard.h` and regression `Boat/tests/ekm014_pulse_guard_test.cpp`.

- Four distinct channels (left/right bait hopper, left/right hook), with configured individual pulse widths in 1–500ms range **only after measured coil characterization**.
- Requires commissioned independent hardware pulse limiter, fresh (<50ms) coil power feedback, motors-current-zero and post-deenergization coil-current-zero.
- Enforces single active channel; no retrigger while pulse active or awaiting zero-current; timeout or stale feedback disables the software guard.
- A software gate alone cannot prevent stuck MCU/expander outputs or welded MOSFET. **External independent pulse limiter, one-hot interlock and DC coil cutoff remain mandatory**.
- This module is NOT connected to a physical EKM014 GPIO expander; actual control polarity, voltage, interface and manufacturer revision require verification. No actuator power enabled.
- Existing `BoatApplicationPort` already coordinates release tokens and motor neutral. Integration with a real `BoatPhysicalDriver` and the EKM014 COTS I/O interface remains pending.
- Boat/Remote RF, physical STOP, navigation, and on-water acceptance not tested. Official Boat VER014 unchanged.
