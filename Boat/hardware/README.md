# ARCHIVED custom Boat PCB engineering — R15 notice

**No custom electronics PCBs may be designed, ordered or fabricated for this project.** This entire folder is kept as historical engineering reference ONLY.

- `Direct_MDD20A_R7` old Heltec to MDD20A carrier-PCB draft: replaced by premade Heltec/MDD20A and commercial harnesses.
- `Coil_Drivers_R9`: old custom four-FET circuit, replaced in active plan by premade **Electrokit EKM014** candidate.
- `Safety_Cutoff_R10`: custom high-side STOP/fault cutoff, no longer an active board design. **Premade DC-rated safety cutoff remains unselected**.
- `Pulse_Limiter_R11` and `Release_Token_R12`: custom timing/one-hot/rearm circuits retired. **Premade hardware interlock/timing remains unselected**.
- `Safety_Power_R13`: custom wiring for preassembled RECOM converter is archived; RECOM itself remains a valid COTS module with an off-the-shelf terminal adapter (no custom carrier).

All six former KiCad CI workflows are **manual-only archived checks**, not ongoing fabrication gates. The active hardware engineering deliverables are module purchasing/qualification datasheets, cable harnesses, 3D printed **non-electronic** mounts, and safety/dummy-load tests.

[Active project-wide R15 policy](../../docs/HARDWARE_MODULE_ONLY_POLICY_R15.md) • [Boat premade modules](../docs/BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md) • [Remote premade modules](../../Remote/docs/REMOTE_MODULE_ONLY_IMPLEMENTATION_R15.md)

NO high-current power or actual solenoid/motor actuation is authorized. R15 is development-only. Sonar remains undecided; no RP2040 and no rudder.
