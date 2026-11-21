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
