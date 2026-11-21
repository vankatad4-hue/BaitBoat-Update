# R17 — Ready-made actuator safety chain: vendor-checked candidate study
**2026-11-21 | Module-only policy R15 is binding | EKM014 R16 is OWNER SELECTED.**
**Status: DESIGN CANDIDATES ONLY. No independent safety chain installed, physically tested, commissioned or released. NO ACTUATOR POWER.**

## Verified architecture: different functions, different commercial modules

The EKM014 is a 4-output **low-side switching module**, not an actuator power disconnect or standalone automatic timeout/interlock. To honor no custom electronics PCBs, procure a complete **factory-built and documented** chain of (a) manual STOP supervisor, (b) load-current-rated DC cutout with contact feedback, (c) separate no-repeat one-hot short timer/release qualification module, (d) independently validated brownout/wdog/current feedback, and (e) ready-made fuses/conductors. A software limit or fake host simulation is NOT the independent timed hardware.

### C1 — Duelco NST-3.2 **42042237**: emergency-stop *supervisor* (research candidate)
- [Duelco manufacturer product](https://duelco.com/products/emergency-stop-relays/emergency-stop-relay-nst-32) and [Duelco instruction sheet](https://duelco.com/media/fb4ha1fj/nst-3-2_manual.pdf).
- Exact 8–36 V DC **H variant** product 42042237; NOT the 12 V fixed-range type 42042228. Manufacturer diagram: A1+/A2− supply, normally-open safety output contacts 13–14, 23–24, 33–34, NC auxiliary 41–42, start/reset X1–X2. **Those names are reference only**: installation must follow the selected variant's actual manufacturer diagram, including E-STOP topology, dual-channel/monitored fault behavior and load fuses.
- Manufacturer manual specifies **3 A at 24 V DC resistive outputs** (also 6 A AC resistive, not interchangeable); output contacts **must NOT carry MDD20A or EKM014 battery current**. Contactor coil enable only (pending coil load and suppression).
- CRITICAL: **permanent X1–X2 bridge allows automatic reset**. Requirement is a local deliberate, monitored physical reset that CANNOT auto-rearm on power return. Follow the manufacturer's **external contactor feedback** example (K3/K4 and positive-guide monitoring). An NC contact that is not mechanically linked to actual main contacts cannot prove a welded power contact.
- Manufacturer DIN-rail casing approx **22.5 × 82 × 98.8 mm** and ~200 g; larger than the legacy 30 mm-height electronics slot; mounting in a new waterproof compartment has not been proven. Operational voltage below 8 V, 3S BMS cut/restart, transient tolerance, 2.5 VA control load, 1.4 A short starting surge and reset rise/fall must be tested.
- **CANDIDATE ONLY.** It is not a four-channel timing board or high-current emergency switch.

### C2 — Albright **SW60C / SW60A** 12 V *continuous-duty* DC contactor WITH auxiliary (research candidate)
- Manufacturer [SW60 family](https://www.albrightinternational.com/products/sw60/) and [SW60 official data sheet](https://www.albrightinternational.com/wpcms/wp-content/uploads/2020/08/SW60-Data-Sheet.pdf).
- Family designed for inductive DC (small traction motors) with **80 A thermal** rating, M6 main studs, normally-open double-break load contacts. **80 A thermal is NOT blanket approval for actual two-motor start/stall, 3S BMS or fuse/connector coordination.**
- A **12 V coil, 100% continuous-duty** (manufacturer 5–7 W nominal coil power) with manufacturer-fitted **SW60C V4 auxiliary or SW60A auxiliary** is preferred **for feedback into C1**. Exact full order code, coil/diode suppression polarity, actual auxiliary NC monitoring and availability are **UNRESOLVED**. Do not assume any generic SW60-360P has the needed auxiliary.
- Manufacturer guideline continuously rated pull-in voltage 66% of nominal at 20°C, typically ~15ms to pull-in; specified at other temperatures/configurations and 3S droop **must be verified**. Manufacturer suggests contact drop-out can be ~6ms with no suppression versus ~35ms with diode-only suppression; no guaranteed E-STOP delay from that typical number. Suppression selection changes contactor release time and is safety relevant.
- Body dimensions in manufacturer mechanical drawing roughly **81 × 38/41 × 57 mm** depending orientation and bracket, nominal mass ~190g; does **not fit** original 14–30 mm vertical clearance. Use a physically separate waterproof mounted compartment or choose a different approved rated contactor.
- **Do NOT substitute an SW60P purely for IP66 without feedback.** Manufacturer [SW60P datasheet](https://www.albrightinternational.com/wpcms/wp-content/uploads/2020/08/SW60P-Data-Sheet.pdf) explicitly marks **both auxiliary contact options NOT AVAILABLE (X)**. Thus bare SW60P cannot satisfy the chosen monitored external contactor return. SW60A/C + waterproof enclosure is a more plausible fit functionally, but mechanical fit is not accepted.
- Coil draw (~0.42–0.58 A at 12 V for 5–7 W) adds to 3S power/current budget, and relay coil must be supplied via a suitable ready-made source that remains safe through battery sag. Exact power module and qualified coil undervoltage characteristics not chosen.
- **CANDIDATE ONLY:** supplier confirmation of exact part and all real current/feedback/mounting characteristics required.

### C3 — Factory-built 4-channel programmable pulse board: **ShillehTek** (FUNCTIONAL research candidate only)
- Manufacturer [four-channel programmable board manual](https://shillehtek.com/blogs/shillehtek-product-manuals/4-channel-relay-module-programmable-delay-6-24v-manual) describes 6.5–25 V DC input or 5 V DC supply, four optoisolated inputs compatible with 3.3 V, four relays, and **0.01 s – hours timer modes depending on mode**, size ~**85×55 mm**.
- Actual pulse minimum in the selected mode, **single press vs held-high behavior**, non-retriggerable at pulse expiration, default-off on POR/brownout, rejection of **simultaneous requests**, and wetting/logic compatibility of dry relay outputs with the EKM014's **3.3 V active-high inputs** are **NOT VERIFIED**. A label "interlock mode" or "pulse" does NOT establish four-way zero-on-collision one-hot, fault containment or an independent safety-rated limiter.
- The timer board relay outputs must **never be substituted for the EKM014 coil-power switching output** and its 10A@30V DC claim is not an inductive coil safety rating.
- Its unshielded ~85×55mm board will not fit original 77×79mm carrier slot in all orientations; enclosure separation/fixation is an open mechanical gate.
- **NOT SELECTED FOR HARDWARE SAFETY.** At most use for isolated logic/dummy-load research until manufacturer verifies explicit held-trigger/overlap behavior, min/max timing tolerances, test failure modes and environmental constraints. A certified off-the-shelf multi-channel safety logic controller is a fallback class, **not selected**, if this board fails.

### C4 — Cebek I-33 (older timed relay research)
- [Cebek I-33](https://cebek.co.uk/Item/cebek-i-33-12vdc-double-delay-timer-relay-module-1-240-second) minimum ~0.1 s and only 2 A inductive relay contacts. Cannot qualify arbitrary 6 A coil safe pulse and does not provide 4-way onehot. **NOT SELECTED**.

### C5 — Hardware brownout, motor/coil current-zero, watchdog and coil pulse timing: UNSELECTED
- RECOM preassembled R-78E3.3-0.5 has **no POWER_GOOD pin**; independent qualified POR supervisor is mandatory and must keep C1/C2 OFF until a deliberate physical reset.
- Generic I2C INA260/EKM025 is **telemetry only**, not a standalone analogue fault trip independent of ESP32 clock/software.
- No verified complete solution currently guarantees four-channel one-at-a-time + fixed energized maximum + consumed release lockout + current-zero + fail-OFF on loss of any module. DO NOT fake this using a C3 programmed timer and MCU code.
- The physical actuator wiring shall remain DISCONNECTED until complete vendor-verified module stack is demonstrated with current-limited dummy loads.

## R17 functional topology (NOT a connection diagram to construct unqualified circuitry)

```text
3S4P PACK -> BMS -> MASTER SWITCH -> rated commercial fuses
                                          |
        +---------------------------------+----------------------+
        |                                |                      |
  Clean independent regulated       C1 8–36V STOP         Current-limited
  safety/control power                monitor (candidate)   3V3/5V RF/MCU
        |                                  |
        |             momentary manual rearm + independent motor/coil fault
        |                                  |
        |                        C2 12V coil command (candidate)
        |                                  |
        +------------------------- C2 NO HIGH-CURRENT CONTACT ----+
                                                       |          |
                                              MDD20A MOTOR      EKM014 VH
                                              (two 550s)        (four coils)
                                                                  |
                EKM014 VL = independent regulated 5V ready module |
               IN1..IN4 <---- C3 FUNCTIONAL PULSE candidate ------+
                           only if independent no-repeat/onehot
                           safety qualification is later proven
```

C1-to-C2 coil-control and auxiliary feedback details are **not** physically pin-mapped here. Manufacturer has verified example schematics; those are **not** permission to wire a complete high-current emergency system without a qualified design review.

## Why direct C2 contactor vs EKM014 VL inhibit is important
Removing EKM014 VL *alone* is NOT an independently demonstrated coil cutoff if a low-side FET fails short; removing the separate high-current **VH battery branch** is necessary. Cut both motor and EKM014 VH behind the contactor, while preserving low-current supervisor supplies upstream for fault reporting. If a main contact welds, feedback must fault the next reset; during weld, independent backup power disconnect/fuse measures may be required. Confirm hardware STOP behavior at motor inductive load before any water use.

## Mechanical reality check
- EKM014 nominal **32×30mm** (hole pitch unresolved 27×25 vs product-listing contradiction), likely only this small module is plausible in legacy low-voltage 77×79 slot.
- C1 Duelco DIN unit depth **~99mm**, C2 Albright SW60C length **~81mm** and thickness/height **~57mm**: neither is compatible with a 30mm lid clearance slot. **Existing boat hull additional-volume measurement is now a hard blocker.**
- Rearrange only factory-made modules and removable harnesses; 3D-printed *non-electrical* mounts and waterproof enclosures are permitted; no custom PCB even for an adaptor.

## Hard stop / continuation
- Owner-selected **EKM014 41024394** remains frozen. None of C1, C2, C3 selected, bought or validated.
- Must first measure the physical motor/coil current and available hull volume, then obtain C1+C2 exact vendor full SKU/polarity/feedback and test under 3S min/nominal/max + coil voltage. In parallel obtain written manufacturer confirmation for standalone C3 four-way no-repeat/zero-on-overlap and actual pulse-time tolerance; if unavailable **reject C3 for safety** and source a different ready-made safety sequencer.
- Do not enable Heltec actuator pins, motor/coil power, or Remote release action because of module research. 3S actual power and waterproofing remain unverified.
- No custom PCB designs, no RP2040, no rudder, no Boat E22; integrated SX1262 disabled until LoRa matching; sonar model undecided and **not implemented**.
- Official Remote VER028 / Boat VER014 markers unchanged until physical acceptance.
