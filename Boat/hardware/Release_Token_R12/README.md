# R12 physical consumed-release token — KiCad candidate, NOT fabrication ready

The native KiCad file `Boat_Release_Token_R12.kicad_sch` is a **wiring candidate** to prevent an automatically repeated solenoid release when an R7 command remains HIGH after the R11 main monostable times out. No circuit has been powered or tested on the boat.

## Explicit physical one-use state machine

1. U5/U6 SN74HC32 detect `ANY_REQ` and OR six invalid pairwise overlaps; U3/U4 SN74HC08 generate each of six overlap terms and `ONEHOT_VALID = ANY_REQ AND NOT OVERLAP`. U7 SN74HC14 provides Schmitt conditioning. R1/C1 remove short request transients; all R/C values **TBD**, not deemed effective before tolerance analysis.
2. R3/C3 filter `RELEASE_QUALIFY_RAW = NO_REQUEST_CLEAN AND MOTOR_CURRENT_ZERO AND COIL_CURRENT_ZERO AND (POR AND HW_LATCH AND RAIL_VALID)`. U8 SN74HC14 converts the filtered level into a rising `RELEASE_QUAL_EDGE` after stable release.
3. U1 first SN74HC74 DFF (1CLR_N pin1, D1 pin2, 1CLK pin3, 1Q pin5): a qualified release rising edge sets `RELEASE_ARMED=1`. The **second** DFF 2Q pin9 rises on a **separately filtered/delayed** request rising edge when `RELEASE_ARMED AND ONEHOT_VALID AND MOTOR/COIL_CURRENT_ZERO AND HARDWARE_SAFETY` was true at setup/hold. U1 2CLR_N pin13 is held low when no clean request or independent safety is lost.
4. The second DFF `FIRED_Q` feeds U7's inverter and **asynchronously clears U1's first DFF**: `ARM_CLR_N = ASYNC_SAFETY_N AND NOT_FIRED`. Thus a successful press *consumes* the released-state token. Even if the first DFF is cleared, U1 second DFF **remains latched** throughout the held request, so a timeout cannot create another `FIRED_Q` rising edge.
5. U2 CD74HC221 first-half nonretriggerable monostable triggers from `FIRED_Q` at pin2, output pin13 `RELEASE_EDGE_TOKEN_TBD` to J3 pin1. The R12 token must have a **qualified strictly shorter maximum pulse** than the R11 main monostable's **minimum** pulse (including tolerances and reset/edge propagation). R4/C4 are **unspecified**. Both half-2 reset and unused outputs are handled explicitly.
6. When the user releases, U7's debounced `REQ_CLEAN` goes LOW, U1 2CLR_N goes LOW and `FIRED_Q` is cleared. **This does not rearm** U1 first DFF; only a *fresh* `RELEASE_QUAL_EDGE` after stable zero requests and measured current zero can do so.

## Connections in THIS candidate

- J1 pins 1..4: R7 SOL_REQ_LH / SOL_REQ_RH / SOL_REQ_LK / SOL_REQ_RK, 3.3V conditioned active-high requests (physical interface still TBD).
- J2 pins 1..7: `VLOGIC_3V3`, `GND`, `HW_LATCH_OK_R10`, `POR_OK_TBD`, `MOTOR_CURRENT_ZERO_TBD`, `COIL_CURRENT_ZERO_TBD`, `RAIL_VALID_OK_TBD`. All external actual sensor circuits, approved regulator, physical R10 latch connector and POR supervisor **NOT CAPTURED**.
- J3 pin1: `RELEASE_EDGE_TOKEN_TBD` output to the existing R11 J2 pin6 net of the same name; pin2 signal GND. This is a future harness link, not an integrated hierarchy.

## Explicitly unqualified, failure modes

- R1/C1, R2/C2 (reset release before clock), R3/C3 (stable release), R4/C4 (short token) have **NO selected values**, thresholds, resistor drift, capacitor DC-bias or temperature measurements. Reset-recovery, clock setup/hold, simultaneous request changes, Schmitt hysteresis and asynchronous reset races are not proven. The RC delay must meet a *minimum* reset-recovery and setup margin, not merely a typical value.
- With a request already HIGH at boot, POR must remain asserted until all RC nodes are settled and all safety comparators have valid states, or a false `RELEASE_QUAL_EDGE` may arm the token. POR logic is external/TBD. This is a serious safety gate.
- If R12 token maximum width overlaps R11 main window expiration, the R11 `WINDOW_OFF` path could retrigger; require `T_TOKEN_MAX + T_prop_margin < T_R11_MIN`, plus independent physical fault detection. No made-up timing values.
- If the analog current-zero feedback is stuck HIGH, neither this logic nor its simulator proves the coil has stopped; use actual measured current feedback, independent FET short detection, and hardware STOP cutting high-current supply.
- Ordinary SN74HC logic and a single high-side MOSFET do **not** create a qualified functional-safety circuit. Catastrophic shorts, welded STOP contacts, miswired power, floating gates, ESD, radio/session faults and heat remain untested.
- This sheet is standalone and does not clear R7's **15 ERC errors**, R10's **2 external-power errors**, or R11's **2 external-power errors**. R9 has 0 isolated ERC errors. All component footprints remain empty; 3S/12V motor and solenoid loads MUST NOT connect.

**No sonar, RP2040, Boat E22 or rudder.** Boat uses Heltec Wireless Tracker and two independent MDD20A propulsion motor channels. Official Remote VER028 / Boat VER014 withheld from physical release.

Manufacturer reference: TI SN74HC74 https://www.ti.com/lit/ds/symlink/sn74hc74.pdf ; TI CD74HC221 https://www.ti.com/product/CD74HC221 ; TI SN74HC08 https://www.ti.com/product/SN74HC08 ; TI SN74HC32 https://www.ti.com/product/SN74HC32 ; TI SN74HC14 https://www.ti.com/product/SN74HC14 .
