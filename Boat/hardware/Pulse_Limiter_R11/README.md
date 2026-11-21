# R11 KiCad hardware pulse window + hardwired one-hot logic — NOT FABRICATION READY

This is a native, manufacturer-pin-number-based **partial hardware implementation** that connects the logical R7 solenoid request names to R9's limited gate names through an independent non-retriggerable pulse window and registered 4-bit selection. **All electrical power and coil loading remain prohibited.**

## Captured physically in this standalone schematic
- **U1 CD74HC221:** non-retriggerable monostable 1: B1 rising trigger (pin 2), A1 tied LOW (pin 1), active-low reset 1CLR_N pin3; pulse 1Q pin13; timer pins REXT/CEXT pin15, CEXT pin14 grounded with external R1/C1 **both values TBD**. Second half reset LOW; no outputs used. This is a timed hardware pulse, independent of firmware delay loops.
- **U2 SN74HC175:** captured channel requests at common positive-edge CLK pin9; asynchronous active-low clear pin1. Relevant Q pins 2,7,10,15 gate downstream drivers. Capturing the requested bit prevents a live channel swap during the same timer window.
- **U4/U5 SN74HC08:** all **six pairwise overlap products** from four raw requests; **U6/U7 SN74HC32:** OR-combine overlap and any request; **U8 SN74HC14:** invert overlap and condition trigger. Therefore `ONEHOT_VALID = ANY_REQ AND NOT(OVERLAP)`. Any 2+ request combination blocks output.
- **U11 SN74HC08:** qualify ready signal from independent high-side latch, motor-current zero, coil-current zero, external release-edge token, and rail valid status. **These signals' physical sensor circuits and release-edge memory are not included**.
- **U9 SN74HC08:** asynchronous monostable/latch reset on hardware latch loss or invalid/zero channel; window-valid masking, and start gate from external ready. **U5/U8:** gate new trigger by inverted active window and use Schmitt conditioning.
- **U10/U3 SN74HC08:** each gated output is `CAPTURED_x AND RAW_REQ_x AND WINDOW_SAFE`, with `WINDOW_SAFE = WINDOW AND HW_LATCH_OK AND ONEHOT_VALID`. A changed channel cannot immediately energize a different channel even if the register glitches through unsupported edge timing.
- **J1** raw four R7 SOL_REQ_x inputs; **J2** raw 3V3/return and externally *hardware-proven* safety/current/release qualification; **J3** four R9 PULSE_x_LIMITED_TBD outputs. **J3 is NOT connected to live R9 MOSFET gates or physical coils in an assembled circuit.**

## Critical missing safety circuit and race risks

1. **A token that is CONSUMED after each trigger and cannot reset until all raw requests are idle, coil current is measured zero, and a fresh physical release occurs is NOT designed.** J2 RELEASE_EDGE_TOKEN_TBD is a dependency on that independent hardware, not a functioning feature. Stuck request/high at boot can otherwise trigger. The monostable is nonretriggerable *during an active pulse*, but an unchanged HIGH can be retriggered after the pulse if trigger logic develops a false rising edge from window transitions. DO NOT energize.
2. Trigger CLK is shared between U1/U2. Combinational logic transitions, propagation delay, 74HC221 reset-recovery, register setup/hold, asynchronous clear and 3V3 rail ramp may produce spurious pulses; validate timing and revise edge-formation circuit before approval. The current logic may produce **transient pulses under simultaneous changes** despite static truth table.
3. R1/C1 values, nominal width, max width at temperature/voltage/tolerance, and the actual physical safe pulse time **are unmeasured**. U1 output should independently terminate coil drive before a measured safe limit; neither a 100% duty signal nor firmware may prolong it.
4. R10 high-side Q1, NC STOP, voltage, fusing and physical ARM/STOP witness must be accepted against actual motor stall and coil peak; their standalone R10 draft has 2 unresolved power-input ERC errors. The older R7 sheet has 15 ERC errors. R9 low-side standalone has 0, but none of the three sheets is an electrically integrated approved board.
5. R11 uses local generic KiCad symbols with **pin NUMBERS taken from TI CD74HC221, SN74HC175, SN74HC08, SN74HC32 and SN74HC14**, but **all footprints are UNASSIGNED**. Check actual stocked vendor package and symbol mapping against its revision-specific datasheet before production. R11's ERC status describes only the candidate isolated logic sheet. Never add a false PWR_FLAG to hide that J2's safety rail is an external unapproved supply.

## Cross-sheet mapping
- `R7 SOL_REQ_LH/RH/LK/RK -> R11 J1 pins1..4`: **future physical harness**; conditioned signal voltage, default LOW, boot and rail-fault unknown.
- `R10 LATCH_Q -> R11 J2 HW_LATCH_OK_R10`: require actual independent latch with STOP/witness, WDO, UVLO/thermal/current.
- `R10 clean return and supervised 3V3 -> R11 J2 pins2/1`: actual regulator not approved, no 3S direct wiring.
- `R11 J3 pins1..4 -> R9 J2 pins1..4`: logical one-hot timed gates; actual keyed harness/PCB not approved.
- `R10 high-side coil power -> R9 J1`: only after physical load-rating validation.

Vendor references: TI [CD74HC221](https://www.ti.com/product/CD74HC221), [SN74HC175](https://www.ti.com/product/SN74HC175), [SN74HC08](https://www.ti.com/product/SN74HC08), [SN74HC32](https://www.ti.com/product/SN74HC32), [SN74HC14](https://www.ti.com/product/SN74HC14).

**No sonar driver or connector, no RP2040, no rudder, no Boat E22; all physical actuator GPIO remain disabled. Official Remote VER028 and Boat VER014 unchanged.**
