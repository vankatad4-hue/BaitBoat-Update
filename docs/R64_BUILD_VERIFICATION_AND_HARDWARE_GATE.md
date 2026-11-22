# R64 — R63 target-build verification and next hardware integration gate

**Date:** 2026-11-22. **Branch:** `hardware-ready-dual-radio-r1`.
**Scope:** verification of previously pending R63 target CI and focused read-only application/radio execution review. This is **not** physical validation or a new operational release.

## Verified CI completion
- Remote full selected Waveshare target compile, GitHub Actions run [44991021855](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44991021855): compile job **completed / success**.
- Boat selected ESP32-S3-DevKitC-1-N32R16V diagnostic-only target compile, run [44991310465](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44991310465): compile job **completed / success**.
- Earlier R63 host runs recorded and passed: Remote 44991125196, Boat 44991241882 and 44991310498.
- This verification checks GitHub job conclusion, not detailed compiler warnings, hardware run behavior, or on-water reliability.

## Source inspection
Inspected `Boat/source/boat_application_port.cpp`, `Boat/source/boat_application_port.h`, `Boat/source/boat_dual_radio_port.cpp`, `Boat/source/boat_dual_radio_port.h`, `Boat/source/boat_radio_safety.cpp`, `Remote/source/dual_radio_manager.cpp` and `Boat/tests/boat_application_integration_test.cpp`.

Observed:
1. Boat diagnostic application refuses to attach a physical driver without read/apply/disable methods; the un-attached branch only drains commands, does not apply output.
2. The application handles an active-plan `MODE_RETURN_HOME` and checks current link freshness and software STOP; upstream application policy/codec prohibit backup manual drive and release.
3. Physical-driver feedback and output-enable verification still have no commissioned pins or independent safety-chain evidence. Simulator success is not evidence of safety-rated STOP or DC load isolation.
4. Existing manual release completed ACK defaults to primary NRF, consistent with backup release being prohibited; this is **not** a new change.
5. No firmware source changes were made during this verification; no fresh compile is required for this documentation-only revision.

## Next implementation path (blocked at purchase/bench interfaces)
1. Select and verify exact **ready-made**, 3S-compatible, independently acting emergency STOP/hold-off, watchdog and brownout supervision, DC-rated motor+solenoid isolation with real contact feedback, one-hot non-retriggerable maximum pulse limiter, current-zero sensing, fusing and low-voltage rails. A supervisory relay rated only for 3A resistive DC must not carry two motors or four 6A-design coils. Keep every candidate **UNAPPROVED** until a full voltage/current/inductive rating and compatible harness is proven.
2. Complete a **physical pin-to-module wiring crosswalk** using actual purchased board connector revisions and official manufacturer wiring information; do not assign pins solely from theoretical free GPIO counts.
3. Implement the real MDD20A/EKM014 HAL behind an explicit commissioning switch **only after** the independent hardware cutoffs and feedback sensors are installed, bench-tested and fail-closed. No simulated feedback is an acceptable live enable condition.
4. Validate authenticated NRF24 plus matching E22 hardware, EU868 duty-accounting, live GNSS/heading calibration and return-home truth table under link loss and independent STOP.
5. Carry out fault-injected dummy-load testing, then controlled water acceptance; publish new versioned ZIPs only once these gates actually pass.

## Frozen selections / holds
- Boat controller: ESP32-S3-DevKitC-1-N32R16V; Remote: Waveshare ESP32-S3-Touch-LCD-4.3 Standard.
- Ready-made modules only, no electronic custom PCBs; MDD20A differential thrust (no rudder), EKM014 selected model for four low-side solenoid channels.
- Primary NRF24; matching E22-900T22S UART LoRa pair needs verified ready-made carrier. LoRa backup STOP/Home only for previously active mission.
- Boat has no SD; Remote integrated SD stores eight lake profiles and up to sixteen waypoints.
- Sonar is deferred. Hardware still not purchased/assembled or safety-commissioned.
- **No physical actuator enable approval.** Official Boat VER014 / Remote VER028 remain unchanged.
