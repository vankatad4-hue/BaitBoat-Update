# R65 — Safety supplier evidence request (NOT SENT)

**Prepared:** 2026-11-22. **Status:** technical questions for manufacturer/authorized industrial electronic distributors. **No messages sent or orders placed.** All modules must be factory-built. No custom PCBs or soldered logic matrices.

## Common system envelope (send with every applicable enquiry)

Flytec V080 bait boat retrofit, 3S4P Li-ion battery typically 9.0–12.6V at external rails (**verify pack BMS actual cutoff/sag**), 20A BMS reported but not measured. Two independent brushed 550 12V motors, driven by selected Cytron MDD20A (two channels; actual starting/stall/inductive interruption current unknown), four 12V coil actuators (two bait hoppers and two hooks), provisional **up to 6A per coil**; only one coil may be energized at a time. Selected FET switching board **Electrokit EKM014 41024394** low-side, 5–50V VH, 4.5–12V VL, shared 10A slow-blow supply fuse; this is NOT a safety interlock.

Independent hardware requirements (not the ESP32 firmware): normally OFF on every power-up and battery drop, safety-rated or independently validated manual local reset, physical emergency STOP, removal of all motor and coil load supply, mechanically trustworthy cutoff feedback, watchdog for MCU freeze, brownout inhibition, mutually exclusive four-coil command arbitration, each event triggers at most one bounded pulse (no repeat on held-high, noise or reboot), zero-current confirmation and fail-safe feedback wiring. STOP/contactor cannot assume 20A BMS suffices. Test with current-limited dummy loads before live motors/solenoids. No assumed safe coil pulse width until measured.

### Duelco NST-3.2 / 8–36V (42042237)
[Manufacturer](https://www.duelco-safety.com/products/emergency-stop-relays/emergency-stop-relay-nst-32)

Please provide the exact revision's official installation/terminal diagram (including X1/X2), mechanically-linked external contactor feedback wiring, mandatory manual/monitored reset implementation (not an X1–X2 permanent bridge), external coil suppression policy and rated DC inductive contactor **coil** current. State expected loss-of-supply, welded-contactor, feedback-open, stuck-reset and restoration behavior. Is 8–36 V input continuous over the entire 3S high-ripple sag profile? Quote exact 42042237 with stock, authorized distributor and shipping to Bulgaria. Confirm ingress protection and moisture limitations.

### Albright SW60 contactor family
[Manufacturer](https://www.albrightinternational.com/products/sw60/)

Please quote complete purchase SKU for **normally-open** SW60 with **12V coil rated for continuous energization**, confirmed low-voltage pickup/dropout across 9.0–12.6V under current draw and hot-cold coil, insulated coil suppression, and mechanically suitable NC auxiliary feedback for main contact weld monitoring. Show exact SW60A vs SW60C variant and pin/terminal schematic. Confirm the auxiliary can meet Duelco safety relay's external device monitoring assumptions (not merely a separate switch not mechanically linked). Supply DC inductive switching/breaking curves at 9–13V and expected two-motor stall current; 80A continuous thermal is NOT proof of prospective interrupt capacity. Provide thermal/coil power, dimensions, waterproofing, preferred fuse/TVS harness instructions, order code and Bulgaria shipping. If one contactor is insufficient under welded-contact analysis, identify a manufacturer-approved monitored redundant pair architecture.

### Brentek P8D-ISM5 regulated 5V
[Manufacturer](https://www.brentek.com/watchdog-timer-modules/p8d-ism-industrial-supervisory-modules)

Please quote **P8D-ISM5** (not 12V raw 3S model), exact DIN8 preassembled socket/base, user/factory programming tool and lockout/manual-reset options. Confirm 4.5–5.5V supply and thresholds, 3.3V GPIO compatible triggering on 3–30V optical input, heartbeat maximum/restart/power-on timing and measurable contact release after a frozen MCU. Does a frozen-high heartbeat count as a valid tick? Does the relay automatically reenergize after brownout/recovery, and how can external interlock require independent deliberate local reset? What is permissible DC inductive *contactor coil* current and suppression? Quote stock and Bulgaria shipping; describe a 5V regulator reserve budget and EMC requirements.

### Independent four-channel safety pulse arbitration
**NO SUITABLE EXACT PREASSEMBLED PRODUCT IDENTIFIED. THIS IS THE MAIN BLOCKER.**

Please identify a factory-made programmable or hardwired safety controller capable of all of:
- 4 independent digital requests (3.3V compatible using manufacturer-approved off-the-shelf interface).
- hardware/safety-controller-enforced **one-hot** mutual exclusion, simultaneous-input **all OFF**, power-up **all OFF**.
- single edge-consumed pulse with external commissioning-set maximum (target setting to be established by actual solenoid measurements); no repeated pulse for a held-high request or even after MCU/brownout reboot.
- each new actuation requires all four requests to be low and all four measured coil currents zero for a qualified interval, plus independent local reset after a fault.
- safe output or redundant disconnect behavior if MCU freezes, supply falls, a signal sticks high, output stage fails or an auxiliary feedback device reports weld.
- actual timing tolerances over 9–13V input, -10–50 C environment, wire-break behavior, hardware fault test procedure, and certified/specified reliability limits.

Pilz [PNOZ m B0 772100](https://www.pilz.com/en-INT/eshop/product/772100) is a **research alternative only**. Please verify whether its certified programmable function blocks can implement this EXACT non-retriggerable four-way pulse contract and safe output fault response, and whether complete ready-made 24V-to-EKM014 3.3V control interfaces are available. Supply software/tool license, terminal and chip-card SKUs, configuration verification method, 24V rail input requirements, currents and dimensions. The [MEAN WELL DDR-15G-24](https://meanwell.bg/product/ddr-15g-24) 9–36V to 24V/0.63A supply is a listed Bulgarian candidate; its 15W is **not** assumed sufficient for a complete controller plus contactors.

### Pack/motor/coil acceptance data missing from owner hardware
Do not sign off any fuse/cutoff/pulse until measuring actual 3S cell and BMS cutoffs, battery under-load voltage, startup/stall of **each** 550 motor and combined BMS overload, four coil resistance/current/turn-off flyback/peak temperature vs pulse width, contactor pickup/release at sag, branch conductor rating and physical STOP response. Record test instrument/range, temperature, load and pass/fail. The current test data is **none**.

## Expected result
A supplier-backed bill of materials with exact manufacturer full SKUs, pinouts, DC inductive/load-current ratings, safe pulse timing, module manuals, physical dimensions, actual purchase availability/delivery to Bulgaria and manufacturer-supported interconnect diagram. Until then, all motor/coil load feeds and four EKM014 active requests remain disconnected; firmware commission flags OFF. Official Boat VER014 / Remote VER028 unchanged.
