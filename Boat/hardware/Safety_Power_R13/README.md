> **R15 ARCHIVED, NOT FOR FABRICATION:** This bespoke KiCad board is retired from the active build. Owner chose factory-made modules/harnesses only. [R15 active Boat implementation](../../docs/BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md), [project policy](../../../docs/HARDWARE_MODULE_ONLY_POLICY_R15.md). The EKM014 premade board replaces R9's custom FET stage provisionally; separate premade power/STOP/timer/current-sense still unqualified.

# R13 Boat safety 3.3 V — ready-made RECOM switching regulator

**Chosen component:** RECOM **R-78E3.3-0.5** SIP3 preassembled switching DC/DC module, not a custom regulator-IC feedback/inductor design.

Official manufacturer source: https://recom-power.com/pdf/Innoline/R-78E-0.5.pdf (REV 9/2025), https://recom-power.com/en/rec-p-R-78E3.3-0.5.html

## Manufacturer requirements checked
- **Pin1 = +VIN; pin2 = GND; pin3 = +VOUT**, confirmed from manufacturer mechanical drawing, **DO NOT invert**.
- Input **6–28 V DC**, output fixed **3.3 V**, maximum output **0.5 A**, **non-isolated**. 3S pack nominal 11.1 V / fully charged 12.6 V is inside the input range *assuming measured pack transients stay inside 6–28 V*. Real dips/transient limits not verified.
- Module dimensions **11.6×8.5×10.4 mm**. In the 77×79 mm carrier/30 mm lid clearance there is provisional envelope space; hardware stacking, socket/vibration retention and pin hole orientation not validated.
- For >2% output load the ripple specification applies as characterized; manufacturer also lists typical 50mV peak-to-peak output ripple at test conditions and recommends 100nF across output for test. 2%×500mA = **10mA**. R1 optional 300Ω 1% 0.125W contributes ~11mA (~36mW) *if installed*, at the cost of extra standby loss. Measure actual load and ripple before deciding to install. Manufacturer maximum capacitive output load 220uF; proposed C2 10uF, C3 0.1uF <220uF, but include downstream module bypass in final sum.
- C1 10uF/35V input and C2 10uF/10V output are **optional manufacturer example components**, subject to current PCB/EMI placement and temperature/bias. Input fuse F1 is a **provision with no selected rating**; BOM does not yet authorize a fuse value. C3 100nF/10V is output test/decoupling. All footprint assignments left empty.

## Actual net plan
```
3S Li-ion BMS -> MASTER SWITCH -> PACK_SWITCHED_3S_TBD
       |                                     |
       |                                 R13 J1 pin1
       |                                 R13 F1 (value TBD)
       |                                 R13 U1 pin1 +VIN
       |                                 R13 U1 pin2 = LOGIC_RETURN_STAR <-- J1 pin2
       |                                 R13 U1 pin3 = VLOGIC_3V3
       |                                     |
       |                                 R13 J2 3V3/GND -> safety R10/R11/R12
       |                                 R13 J3 test points
       +---- separate 3S-to-5V supply -> Heltec approved 5V input (TBD)
       +---- independent RF 3V3 supply -> nRF24 PA/LNA (TBD)
       +---- R10 load-rated STOP-latched high-side cutoff -> MDD20A + 4 coils (TBD)
```
The module's grounds are COMMON internally — `LOGIC_RETURN_STAR` is a controlled shared reference, **not** galvanic isolation. Keep motor and solenoid return currents out of the 3.3 V safety signal harness.

## Important failures still not solved
- Unlike some Pololu boards this 3-pin RECOM module provides **no PG/POWER GOOD output**. We **must implement a separate, physical, independent voltage supervisor/POR** with fault-latch reset. Do not treat a rising 3.3 V rail as valid POR. No software pull-up or MCU pin can substitute.
- Converter **short-circuit protection automatically recovers**. A supply fault/restart **must NOT rearm actuator power**. Physical fault latch power-up default OFF and deliberate local ARM edge after healthy signals and zero currents are still mandatory.
- Real active+idle current budget, peak startup load, temperature derating, 3S BMS collapse, EMI near GNSS/RF, cap load and fuse/current transient need measurement. The module is not certified here as a system-level independent functional safety device.
- Do **not** connect PACK 3S directly to the Heltec's 1S battery/charger pad. Do **not** power the nRF24 PA/LNA from this safety rail. Do **not** power MDD20A or solenoids from this module.
- No R13 schematic ties actual source power to the **separate** R10/R11/R12 standalone sheets yet. Nothing previously had a system-wide zero ERC. R13 also is not production-ready: all footprints empty; no PCB, current-sense, UVLO, hardware POR, STOP acceptance or actuator activation.

## Proposed next work
Add a separate supervisory IC or ready-made board that asserts an independent `POR_OK_TBD` only after 3V3 within tolerance for defined period and goes LOW on brownout. Integrate its reset path to the R10 hardware fault latch and R12 state machine. Then verify combined KiCad hierarchy/netlist with actual current sensing, not by adding PWR_FLAG to hide errors.

**Architecture unchanged:** single Boat Heltec Tracker, two MDD20A motor channels, four solenoids, no rudder/RP2040/Boat E22; sonar is model-undecided and unimplemented. Official Remote VER028 / Boat VER014 remain unchanged.

### R13 native KiCad completed structural verification (2026-11-21)

- Manufacturer ready-made **RECOM R-78E3.3-0.5** safety-only 3V3 regulator; native sheet `Boat/hardware/Safety_Power_R13/Boat_Safety_Power_R13.kicad_sch`, module pins **1 VIN, 2 GND, 3 VOUT**, input fuse F1, optional RC supply components, separated 3.3V safety connector/test connector, no production footprints.
- Actual CI KiCad 9 XML+ERC **PASS**: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44931874988 on commit `0b5eb1f149a62c5c951429a5ab37a57f1b485847`. Validated **9 component instances / 4 exact electrical nets**, including series fused input and vendor supply pin numbers. **Exactly two remaining blocking external power-source ERC errors** (upstream switched 3S supply and logic common return); **zero extra ERC warnings**, no off-grid endpoints. The first R13 runs failed missing connector endpoints and off-grid placement; native 1.27 mm grid correction `0b5eb1f` supersedes them.
- R13 is physically unassembled and its source BMS/fuse, transient protection, 500mA load budget, ripple, PG/POR supervisor, EMC, connector/socket, RF separation and physical R10/R11/R12 connections have not been commissioned. RECOM module has **no PG pin** and can restart after a short-circuit; independent reset/latch must stay OFF and require deliberate manual ARM. The R13 sheet does not imply zero whole-Boat ERC and **is not fabrication-ready**. No servo/rudder/RP2040/Boat E22; CHIRP sonar choice remains open, no sonar firmware. Official release markers Remote VER028 / Boat VER014 unchanged.

