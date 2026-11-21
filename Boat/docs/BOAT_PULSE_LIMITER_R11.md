# R11 pulse-limiter engineering gate — no PCB release

This work builds on existing R7 (MCU command/control), R9 (four low-side coil channels), and R10 (draft independent high-side STOP latch). **R10 is already in the branch**; do not recreate it or claim it is validated. R11 records the still-absent **physical one-hot / non-retriggerable pulse** implementation and an explicit clock-discontinuity failure test in the host reference model.

## Candidate physical topology — requirements, not yet captured

1. **Raw channel selection:** four conditioned, normally-low requests `SOL_REQ_LH/RH/LK/RK`. No raw MCU pin can energize Q1–Q4 on the R9 sheet. Validate each source's boot/reset default with Heltec V1.2 hardware. An expander reset must make all requests LOW.
2. **Invalid requests:** independently implemented combinational `ONE_HOT_VALID` must reject all multi-bit combinations. 0000 is idle; exactly 0001,0010,0100,1000 is valid. A 2-input gate network alone is not sufficient without verified propagation and glitch behavior. Invalid input must also latch a fault until physical reset.
3. **One-shot:** a **hardware** non-retriggerable monostable with a guaranteed **maximum** ON time across resistor/capacitor tolerances, temperature and supply variation. Trigger only on rising edge after an observed all-zero release and proven coil current zero. A stuck HIGH request may not re-trigger. The host model's 80ms is a **synthetic test fixture, not the coil pulse target**.
4. **Output gate:** `LATCH_Q AND ACTUATOR_BUS_OK AND MOTOR_CURRENT_ZERO AND ONE_HOT_VALID AND PULSE_WINDOW` plus an independently qualified coil-current-zero condition at pulse start. The actual 4-way demultiplexer and power gate must default LOW at reset and under partial power loss. Avoid asynchronous glitch-generated pulses; a valid request must be stable for a qualified minimum time.
5. **Feedback:** measure actual current in each branch, and prove return to zero after a pulse before EXECUTED ACK or a second release. The current sensor, amplifier/comparator, threshold and fault propagation remain unselected. MOSFET stuck ON cannot be detected reliably from a software gate signal alone.
6. **STOP/watchdog:** any R10 STOP contact opening, WDO, UVLO, overcurrent, thermal, rail fault, or invalid gate feedback must remove **both** motor and coil actuator power. Physical STOP must not depend on the ESP32. For a welded high-side Q1, define independent protection and test with an external current-limited dummy load before real coils.

## Safety timing proof to obtain

Let `T_coil_max` be the manufacturer's/measured safe pulse limit. Prove

`T_mono_max(tolerance, temperature, supply) + T_output_off_max < T_coil_max`

and

`T_STOP_to_no_coil_current < T_STOP_required`,

with actual component and wire tolerances, including flyback suppression and stored energy. No values for these times are known today; do not fabricate component values or advertise a safety response time.

## Acceptance tests

- 16 request masks: 0 idle, four single-bit valid, 11 multi-bit invalid; all combinations tested.
- Stuck high at boot, loss of MCU, brownout, watchdog timeout, stuck high while STOP asserted, repeated press without release, overlapping requests, timer wrap/discontinuity, and fault-latched reset.
- Worst-case actual motor PWM activity, motor coasting and solenoid residual current before allowing any pulse.
- Gate stuck HIGH, MOSFET D-S short, high-side cutoff Q1 short, single STOP contact welded and missing 3V3 safety rail must be analyzed separately. **No software test proves physical fault tolerance.**

## Current status

The R9 reference model now fails closed on negative/backwards simulated timestamps while active, with additional host regression. **No physical pulse-limiter schematic or qualified parts have been created in R11.** R7 still has 15 ERC errors, R10 has 2 deliberate undriven-power ERC errors, and R9 standalone has 0; these are separate sheets, not a system-wide ERC pass. Physical Heltec V1.2 GPIO, exact motor/coil currents, real fuse and high-side MOSFET selection, PCB, physical tests and official releases remain blocked. Sonar remains planned only.


## R11 physical-timing candidate and verified KiCad continuation — 2026-11-21

- Created an actual editable KiCad schematic at `Boat/hardware/Pulse_Limiter_R11/Boat_Pulse_Limiter_R11.kicad_sch` with 17 components: CD74HC221 monostable, SN74HC175 channel latch, six SN74HC08 quad AND ICs, two SN74HC32 quad OR ICs, one SN74HC14 Schmitt inverter IC, three R/C passives, and three harness connectors. All footprints and timer R/C ratings remain unassigned.
- Native KiCad **PASS** run https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201259 on `cb8ed8546bff1c5555ddecc667c052365cbe61d1`: 54 named exported XML nets checked against exact pin numbers, 9 explicit no-connect pins, 1,024 static one-hot/window combinations and cross-sheet logical-name checks passed. The isolated R11 sheet has **exactly two outstanding power_pin_not_driven errors**: the external unqualified 3V3 safety rail and return. No additional ERC errors; neither 2 ERC errors nor isolated ERC proves system safety.
- Existing Boat host CI PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201349; ZIP packaging workflow PASS https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44918201366. Previous R11 KiCad run failed a test assertion pin number (pin 13 vs real pin 12), corrected before passing.
- The new interface contract `Boat/docs/BOAT_R7_R9_R10_R11_INTERFACE_CONTRACT.csv` maps R7 raw release, R9 MOSFET gates and R10 safety, but **all physical inter-sheet connections remain UNBUILT**. R10 internal LATCH_Q is not wired out to an approved connector.
- **Blocking unsafe failure mode:** The external `RELEASE_EDGE_TOKEN_TBD` is **not implemented**. If this input remains HIGH, a held release request can trigger a second pulse after the 74HC221 window ends. Analog motor-/coil-current-zero comparators, independent 3V3/POR and rail-good logic, R/C pulse bounds, input filtering/setup/hold and actual actuator STOP/current-off testing are also missing. **Never connect/energize motors or solenoids from this schematic.**
- Remaining standalone ERC blockers: R7 **15**, R9 **0** (isolated low-side stages), R10 **2** (external power), R11 **2** (external power). No combined ERC, PCB DRC, validated high-current path, physical acceptance or release.
- Next: capture the *consumed* hardware release/zero-current rearm token (no autorepeat), independently qualify current-zero and logic supply, analyze race/glitch/fault timing, establish manufacturer pulse width/fuse/MOSFET ratings from measured hardware, integrate four sheets into native KiCad hierarchy and run system ERC. Sonar remains **planned only**; no sonar module selected. Single Heltec Boat controller with MDD20A differential motors/no rudder, no RP2040 and no Boat E22. Official Remote VER028 / Boat VER014 unchanged; 029/015 withheld.

## R12 physical consumed-release token — verified isolated KiCad candidate (2026-11-21)

- New native **editable** schematic `Boat/hardware/Release_Token_R12/Boat_Release_Token_R12.kicad_sch` (23 generic components) and project-local symbol library. Two flip-flops in SN74HC74 implement **RELEASE_ARMED** (set only after a separately RC/Schmitt-qualified all-requests-released and current-zero condition) and **FIRED_Q** (held during a pressed request, consuming RELEASE_ARMED asynchronously). CD74HC221 supplies a *separate short token* to existing R11 `RELEASE_EDGE_TOKEN_TBD`; six pairwise overlap terms reject multi-request patterns. R1-C4 timing/filters unspecified, all footprints blank. R12 output connector J3 is **NOT physically wired to R11 J2** yet.
- Actual KiCad CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745446 on `ee9ff4fbb122ce59ce2628007c3ad2f05d24ace8`: **53 precisely matched electrical nets, 11 explicit no-connect output pins, 256 static mask/ready tests and 7 synthetic temporal-model tests**. Only **2 unresolved isolated-sheet `power_pin_not_driven` ERC violations** remain for the unapproved 3V3/ground source; this is NOT zero ERC on full Boat. Boat portable host CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745373; ZIP packaging CI **PASS** https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44930745305.
- Detailed interface mapping: `Boat/docs/BOAT_R7_TO_R12_INTERFACE_CONTRACT.csv`, with every unbuilt harness/status explicit. R7 SOL_REQ_x to R12 J1; R12 J3 pin1 to R11 J2 pin6; R11 J3 4 gates to R9 J2; R10 internal LATCH_Q is NOT exposed on an approved connector. R10 feeds R9 coil rail only by a proposed independent fused power connector.
- Critical remaining hazards listed in `Boat/docs/BOAT_R12_HAZARD_REVIEW.csv`: external POR may assert before 3 separate RC filter nodes settle, request bounce/race, non-zero coil current or failed current sensor, R12 token width longer than R11 main time, stuck-high token that can retrigger R11, U2 or high-side MOSFET failure, motor and coil real flyback, unverified 3S high current. **No actuator power may be connected.**
- Component gate: qualify *actual* 3S-to-logic regulator/POR, R/C release/debounce/reset-to-CLK and token-vs-main timing over tolerance+temperature, independent motor/coil current-zero comparators, physical R10 STOP latch and output connector, and a power-rated high-side actuator disconnect for both motor and solenoid supply. Integrate actual combined KiCad hierarchy/ERC/PCB/DRC and perform dummy-load fault acceptance.
- Standalone ERC status: **R7 15, R9 0 (partial low-side only), R10 2, R11 2, R12 2**. No system-wide zero ERC or fab authorization. Existing Boat and Remote pin maps remain uncommissioned; no RP2040, no rudder, no Boat E22. Sonar has no selected model and remains **planning only**. Official versions Remote VER028 / Boat VER014 unchanged; VER029/VER015 withheld.

