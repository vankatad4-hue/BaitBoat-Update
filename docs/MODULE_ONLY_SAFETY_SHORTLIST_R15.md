# R15 historical premade safety shortlist (UPDATED BY R17)

> **R17 supersedes this initial candidate study.** See [manufacturer-checked R17 safety stack](../Boat/docs/BOAT_PREMADE_SAFETY_STACK_R17.md), including SW60C/SW60A optional monitoring feedback, **SW60P no available auxiliary**, and the exact Duelco reset warnings. Still no independent actuator power approval.
**2026-11-21. No custom electronic PCB is allowed.** This is a procurement/compatibility investigation, not a working independent safety circuit. All parts here require measurement and matched-system wiring verification before selection.

## S1 — Premade STOP/supervisory logic: Duelco NST-3.2 8–36V DC
- Manufacturer: [Duelco Emergency Stop Relay NST-3.2](https://duelco.com/products/emergency-stop-relays/emergency-stop-relay-nst-32)
- Exact **8–36VDC** variant is **Product ID 42042237**; 12VDC version is separate **42042228**. Prefer investigating the 8–36VDC variant because the boat 3S battery moves from ~9V to 12.6V and may sag; **actual minimum at load remains unmeasured**.
- Designed to monitor emergency-stop and other safety input arrangements. A safety relay's safety outputs are NOT automatically suitable to directly interrupt two motor startup/stall currents and the 6A-class solenoid supply. Select and pair with a **separately DC-rated, inductive-load-qualified contactor/disconnect** with monitored auxiliary NC feedback; exact part **not selected**.
- Need product manual pinout, reset type (manual/monitored), voltage thresholds, NC fault behavior, tolerance of battery transients, physical dimensions and IP/environment ratings. Without this evidence it is only a candidate. **Do not assert that this module by itself provides 4-way onehot or a pulse timer.**

## S2 — Timed one-shot module: Cebek I-33 (CI033), research only
- [Cebek I-33 ready-made 12V dual-delay timer](https://cebek.co.uk/Item/cebek-i-33-12vdc-double-delay-timer-relay-module-1-240-second).
- Advertised non-retriggerable start closure, a fresh trigger needed after completion. Two adjustable delays, *minimum nominal* **0.1 s**, 12Vdc supply, **53.75 x 72 x 25mm**, 7A resistive/**2A inductive** relay contacts. **Therefore its relay contacts must NEVER power a proposed 6A coil directly.** It can only be considered a signal/interlock command, and only after level and output contact validation.
- **NOT SELECTED** because individual coil safe pulse time is unknown, a 100ms minimum may be too long, it does not inherently arbitrate 4 overlapping channels, and its power-on-start option must be disabled. Its physical size consumes almost all of the old 77 x 79 carrier area. Potentially unsuitable for V080 hull.
- Replacement search gate: if measured safe maximum pulse <100ms or minimum qualified pulse <100ms, reject CI033 and find a manufacturer-documented faster premade nonretriggerable timing/interlock product; do not attempt to construct a custom timer PCB.

## S3 — Current feedback: Electrokit EKM025 INA260, telemetry candidate only
- [Electrokit EKM025 preassembled current-voltage Qwiic breakout](https://docs.electrokit.com/en/modules/EKM025/) uses INA260, 0–36V bus and ±15A current path, 25x25mm, I2C default 0x40, optional addresses 0x41/0x44/0x45, alert open-drain.
- **One coil up to 6A can in principle fit within a 15A channel rating**, subject to transient, connector and shunt thermal tests. Four coil branches require four separate sensors if monitoring each coil; EKM025 address options make 4 distinct I2C addresses possible. This is a *monitoring proposal*, not a proven independent current-zero shutdown. I2C readings processed by a potentially wedged MCU do not independently cut actuator power.
- Two brushed motor stall currents and combined BMS fault conditions can exceed 15A, so **do not** place this on the full 20A motor power path or count it as motor hardware protection.
- A vendor-certified ready-made threshold comparator/relay module physically controlling S1 is still needed for independent current-off/protection; no approved one has been found.

## S4 — High current rated DC disconnect: unresolved
- Candidate industrial contactor examples are often marked **40A AC** but DC-motor inductive cutoff can be very different. Need official DC-1/DC-13/actual battery 9–12.6V inductive duty tables at measured motor stall and both fuse coordination and suppression.
- A Pololu #2813 electronic pushbutton switch is rated **~16A continuous** and the manufacturer warns it is **not an emergency/safety disconnect**. It is **NOT ACCEPTED** as the 20A Boat actuator cutoff. [Pololu vendor](https://www.pololu.com/product/2813)
- Manual service-disconnect master switch and 20A BMS do NOT replace the independently fault-latched emergency stop.

## S5 — Brownout / manual reset: unresolved
- Premade 3.3V or 12V watchdog/POR relay module needed with independent cutoff and manual, no-automatic-restart latch. The RECOM R-78E3.3-0.5 preassembled converter has **no PG output** and may auto-restart on overcurrent.
- A simple GPIO loop/software watchdog is NOT an independent safety device. All physical paths must default to no actuator energy on reset, low voltage, unplugged safety lead, high stuck-on input, cutoff MOSFET/contact welding, and loss of main MCU clock.

## Selection status / next engineering gate
Duelco NST-3.2 8–36V is a **specific research candidate for S1**. Cebek I-33 is a **conditional research candidate for S2** (may be too slow/large). Electrokit EKM025 is **telemetry only**. S4 high-current DC contactor, complete onehot, independent current-zero, and matched reset chain all remain **UNSELECTED**. Do not order a partly qualified interlock chain as if it is complete.

All components must be bought preassembled; any factory-made DIN-rail/base/terminal adapter is allowed; no new PCB artwork/fabrication, soldered logic matrix or homebuilt interlock circuit.
