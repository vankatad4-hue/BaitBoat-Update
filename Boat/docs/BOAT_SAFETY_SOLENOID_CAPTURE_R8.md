# R8 native KiCad capture contract — actuator cutoff and four coil drivers
2026-11-21 | **Draft engineering connectivity, not an ERC-pass schematic or a fabrication release.**
Active design: Heltec Wireless Tracker V1.2 + separate Cytron MDD20A, integrated SX1262, primary nRF24, 4 coils. No second MCU, no Boat E22. Sonar unselected/on hold.

## PWR_01 — verified physical power paths required
- PACK_POS/PACK_NEG -> main pack-protection/fuse coordinated with 3S4P BMS -> manual master switch -> PACK_SWITCHED. Actual connector/current/voltage/polarity must be verified. Do NOT wire PACK_3S to Heltec single-cell charger connector.
- PACK_SWITCHED -> independent fused logic regulator -> +5V_LOGIC -> approved Heltec 5V input; local filtered 3V3 sensors/nRF rails sized after RF burst measurements.
- PACK_SWITCHED -> **hardware normally-OFF, fault-latched actuator high-side disconnect** -> ACTUATOR_BUS -> separately fused MDD20A power and coil supply branches. Gate driver and high-side device are not specified until verified motor startup/stall, wiring, current-limit and thermal calculations.
- Coil branch voltage: direct pack 9–12.6V only if four actual 12V coils tolerate that variation; otherwise select isolated/regenerated 12V coil rail from measured envelope.
- Real actuator cutoff must discharge motor-driver and coil terminals under worst-case load. TPS3431 WDO toggling alone is NOT proof of disconnect time.

## SAFE_02 — independent watchdog / local fault latch
- TPS3431 candidate U_WD powered from verified 3V3_LOGIC, with manufacturer-recommended bypass and timing components after tolerance analysis. 150pF C0G CWD gives nominal ~66.6ms using the R7 equation but is NOT a frozen hardware timer.
- The MCU WDI pin is independent of PWM and is toggled **only by a completed and healthy main application cycle**. No ISR/timer autonomously feeds the watchdog. Watchdog timeout asserts WDO; asynchronous hardware logic captures fault into a latch.
- Inputs combined in a physically independent **active-FAULT** path: mechanical STOP, watchdog WDO, real undervoltage comparator, rail-health monitor, actual actuator overcurrent and thermal comparator. Any fault trips a HARDWARE latch without MCU assistance.
- Latch powers up faulted/OFF. It may only rearm on a deliberate physical/local arm edge, while every fault source is healthy and all actuator outputs are physically verified OFF. Software SET/RESET and radio commands are NOT allowed to bypass this.
- Keep permit GPIO as one additional required safety condition, but permit alone cannot energize a latched gate or clear fault. An I2C expander may NOT own WDI, STOP, permit or latch set.
- Confirm TPS3431 outputs/open-drain polarity, reset behavior, CWD, VCC range, EN/PS pins, resistor pullups and electrical net names against vendor datasheet at native capture. R7 existing 15 ERC issues must not be waived just to export a netlist.

## COIL_03 — FOUR separate circuits, one physically permitted pulse
For each channel LH, RH, LK, RK:
1. ACTUATOR_SOL_POS via an individually coordinated branch fuse to a keyed two-wire coil connector pin1. Connector pin2 to the drain of a dedicated logic-level N-channel MOSFET Q_LH/Q_RH/Q_LK/Q_RK; source to ACTUATOR_RETURN via appropriate measured current sense, not sensor ground.
2. Gate from one authorized **one-hot selection** signal through provisional 100 ohm resistor; provisional 100 kohm resistor gate-source ensures OFF during power-up and loss of GPIO. Final gate drive R and FET must be selected from measured coil current, pulse duration, SOA and 3.3V VGS rating.
3. Flyback suppression individually from drain/coil negative toward coil positive; choose diode vs TVS clamp based on measured coil release time and energy; a simple diode can delay release. Final voltage/current/thermal and layout footprints remain TBD.
4. Hardware safety chain in series with supply/enable independent from firmware: gate latch permission, one-hot decoder/supervisor, fixed max-pulse hardware limiter, motor/coil mutual exclusion. A stuck I2C bus or GPIO output MUST NOT hold a coil indefinitely.
5. Per-channel actual pulse current/idle feedback; post-pulse verified current-zero feedback is required before EXECUTED ACK. No solenoid request allowed while motor PWM active.
6. An I2C non-programmable expander may select one-of-four only if loss of bus/MCU cannot defeat the physical pulse limiter and high-side cutoff. Default all expander outputs inactive using the actual chip's power-on/reset specification.

## MOTOR_04 — external MDD20A
- Native ESP32 PWM_L/DIR_L/PWM_R/DIR_R to the manufacturer's identified logical channels via keyed carrier harness. Keep default PWM=0 and explicit DIR levels until verified board load/reset behaviour.
- Separate motor supply fused after ACTUATOR_BUS hardware cutoff. Use rated wire and keyed connectors; no motor current through Tracker/sensor return. The 88.9 x 78.74 mm MDD20A does not fit inside the existing 77 x 79 mm low-voltage carrier footprint and requires separate mount validation.
- Actual reverse and coast/brake safety, inrush/stall and full cutoff at terminals require load tests.

## SPI / sensor / ERC mapping
- Integrated SX1262 internal NSS/BUSY/RESET/DIO1 stay wired on Tracker, not through carrier. External nRF SCK/MISO/MOSI sharing requires verified header access and **both chip selects inactive before SPI startup**; separate RF antennas.
- Keep all unverified pin and connector selections explicitly `TBD`; do not fabricate physical chip pin numbers.
- Resolve current R7 **15 open ERC issues** (13 incomplete release/feedback nets + 2 TPS3431 power problems), including correct power sources/PWR_FLAG only when actually powered. Then add the above sheets, assign real component symbols/footprints and perform ERC again. Any new power, gate and coil circuits introduce fresh issues; **zero ERC has not been achieved**.
- Native verification needed: zero relevant ERC errors, netlist cross-check against signed V1.2 connector map, 3S current/thermal/fuse budget, PCB DRC, spacing/ground return, bench tests with motors/coil supply disconnected initially, then dummy loads. No PCB manufacturing authorization.

## RF and sonar constraints
Remote E22/SC16IS750 remains **unmatched** to raw Heltec SX1262. Backup link OFF until real over-air PHY test or approved Remote radio revision. Sonar has no selected model and no associated circuitry or firmware; do not design an unqualified sonar power/data connector.

## Gates to complete after this capture plan
Exact V1.2 pin audit, real motor/coil current and battery output data, high-side cutoff and fault latch component ratings, native four-coil symbols/footprints/net wiring, verification of load power disappearance after watchdog/STOP, zero ERC and PCB DRC, secure RF pairing and full physical commissioning. Official versions Remote 028 / Boat 014 retained.

## R9 continuation — native four-channel low-side stage capture
A separate, editable KiCad prototype is now committed under `Boat/hardware/Coil_Drivers_R9/` with 26 physical-symbol instances, four fused MOSFET coil channels, four clamps, gate resistors and pulldowns, and distinct actuator and gated-pulse input interfaces. `Boat/tests/verify_coil_netlist_r9.py` plus workflow `boat-kicad-coils-r9.yml` perform actual exported KiCad XML net checks and standalone ERC. **Do not conflate this with a completed power-cutoff circuit or full-boat ERC acceptance.** Hardware one-hot/max-pulse limiter, default-OFF fault latch with STOP/WDO/UVLO/overcurrent/thermal, physical isolation of both motor and coil rails, measured current-zero feedback, MOSFET/clamp and fuse selections, footprints and production PCB are not implemented. See the R9 folder README before ordering anything.

### R9 native verification results (not system acceptance)
The new standalone `Boat/hardware/Coil_Drivers_R9` KiCad four-coil low-side sheet passed KiCad 9 XML netlist pin checks (18 distinct nets) and standalone ERC (0 violations): https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44874476930 . **All earlier R7 control/watchdog 15 ERC errors remain.** The R9 coil schematic takes power from an EXTERNAL as-yet-unspecified high-side cutoff and pulses from an EXTERNAL as-yet-unbuilt hardware one-shot/one-hot limiter; it is not safe to connect to MCU GPIO. See `Boat/tools/safety_gate_latch_r9.py` for an **abstract test model** of the desired release-edge, latch and one-hot behavior, not actual fault-tolerant hardware. Host CI passed: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44874405863 .


## R10 engineering checkpoint — independent STOP/witness, high-side candidate and cross-sheet safety (2026-11-21)

- Added native editable `Boat/hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch` with **23 generic component instances** and **35 exported KiCad electrical nets**. It comprises SN74HC08 safety-input AND tree, SN74HC74 positive-edge ARM/asynchronous-clear latch, SN74HC14 redundant STOP witness, a candidate PMOS high-side branch to BOTH MDD20A and four-coil supplies, a low-side MOS gate driver, main/motor/coil fuses, physical NC STOP source-return contact and second independently monitored NC witness. Actual monitored J3 STOP_OK requires a third mechanically coupled contact and independent conditioning. **Hardware is draft/not powered/not a signed safety system.**
- STOP single-monitor stuck-good hazard was caught in review: S1 opening Q2 source alone could allow automatic reenergization after physical STOP release if J3 STOP_OK stuck HIGH. Added S2 separate NC witness, candidate R5 pullup and SN74HC14 to clear the latch even with the other contact input stuck HIGH. Hardware triple-pole STOP mechanical selection/synchronism/required NC behavior and actual ARM debounce/release-qualified pulse remain unverified.
- Native R10 KiCad target run **#(44890983595) PASSED structural tests** on `e77facdc4edb789906429cbd215404858374e266` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44890983595): **35 nets**, no geometry warnings, **exactly TWO unresolved `power_pin_not_driven` ERC errors** (3V3 safety rail and return; no regulator approved). CI now **checks for exactly those two blocking power-source errors** and rejects new violations; this is not a zero-ERC or fabrication release.
- All **1,024 combinations** of the ten external conditioned inputs and the redundant STOP-contact scenarios are covered in host-only truth-table tests, including stuck ARM and permit. Boat host CI **passed** on `7d515e89` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44890701527), as did the cross-sheet interface host CI `7d894bf` (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44891024408). A Python model is not hardware fault containment.
- `Boat/docs/BOAT_R7_R9_R10_INTERFACE_CONTRACT.csv` enumerates critical unverified signals. R7 has its own 10k WDO_N pullup; R10 R1 is now **DNP if R7 pullup fitted** to avoid an undocumented parallel pull-up. R10 coil rail connects to R9 `ACT_SOL_POS_AFTER_LATCH_TBD`; return mapping is PACK_RETURN to R9 ACT_RETURN only through a separately rated harness. R7 solenoid requests MUST NOT directly drive R9 gates: the independent one-hot hardware limiter and pulse time-out do not exist yet.
- Remaining ERC by **standalone sheet**: R7 control/watchdog **15 unresolved**, R9 low-side coil **0** (but not a complete actuator system), R10 STOP/power candidate **2 deliberate external-power-source errors**. These are separate schematics; no combined hierarchy, system-wide ERC, PCB DRC, measured thermal/stall/SOA, physical cutoff latency, final 3S fuse/regulators, ARM hardware, overcurrent/thermal/UVLO comparators, or production footprints.
- Keep exactly one Boat Heltec Wireless Tracker + separate MDD20A dual PWM/DIR differential thrust; **no rudder**, no RP2040, onboard Boat SX1262 currently blocked by unverified Remote E22 PHY interoperability. Sonar remains unselected/held with no implementation. No physical actuator activation. Official markers Remote VER028 and Boat VER014 unchanged; 029/015 withheld.
