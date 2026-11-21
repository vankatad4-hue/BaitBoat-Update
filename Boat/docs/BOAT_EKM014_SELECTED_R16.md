# R16 — Electrokit EKM014 selected solenoid module
**Decision status:** Model **CONFIRMED by owner** 2026-11-21; unpurchased, unassembled, UNCOMMISSIONED. Only commercial modules/harnesses, no custom PCB.

## Exact identification and external evidence
- Manufacturer **Electrokit**; model **EKM014**, Electrokit part number **41024394** (also formatted 4102 4394).
- Four prebuilt N-channel low-side MOSFET outputs; UCC27524 gate-driver IC; four independent active-high control inputs and included Schottky flyback protection.
- Manufacturer [technical wiring/manual](https://docs.electrokit.com/en/modules/EKM014/), [product listing](https://www.electrokit.com/en/mosfet-drivkort-4-kan-50v-10a), [manufacturer PDF](https://www.electrokit.com/upload/quick/ea/ed/1094_manual.pdf).
- Supplier lists **10 A per channel** on product page, but common **VH input is protected by a single 10 A slow-blow non-resettable fuse**. The fuse may not trip immediately at modest overload; **one active coil at a time** is mandatory and physical 6 A-class coil peak/temperature/fuse limits remain to be measured.
- **VH:** +5 to +50 VDC FET supply; **VL:** +4.5 to +12 VDC gate-driver supply. **VH >= VL**. **Do not bridge VH and VL for the 3S boat**: the fully charged pack can be 12.6 V, exceeding VL maximum 12 V.
- Use a **separate ready-made regulated 5 V VL supply**, e.g. a Pololu S13V20F5 candidate, no connector to the RECOM 3.3 V safety-logic rail. Input **IN1..IN4** accept active-high **3.3 V** controls, have approx **120k pull-downs** in manufacturer documentation; pull-downs alone are NOT independent failsafe timing.
- Each output terminal pair is labeled **CHx (+, -)**. Positive coil wire to CHx **+** (internally tied to VH), negative coil wire to CHx **-** (switched to common GND when input HIGH). The load supply must share the board's designated return with the control reference while high-current coil current is kept off the sensor/control harness. No independent relay/H-bridge or reversal.
- Supplier published **32 x 30 mm** board envelope, **2.5 mm diameter mounting holes**, with mounting-center conflict: product listing says **41 x 25 mm** (larger than 32 mm board, obviously inconsistent), official module manual says **27 x 25 mm**. **Do not drill, make brackets or claim fit until physical caliper measurement or manufacturer clarification.** Expected 3.5 mm screw terminal pitch, 2.54 mm 10-pin logic-header pitch. Terminals included but **unsoldered**: attach manufacturer's terminals to factory-made board; custom replacement PCB is forbidden.
- A VH–GND bulk capacitor footprint exists but capacitor is not included; selection needs measured supply transients and manufacturer's recommended voltage margin.

## Frozen logical channel assignment
| EKM014 channel | Control | Solenoid | Boat mechanism | Allowed while motors active? |
| --- | --- | --- | --- | --- |
| CH1 | IN1 | LH | Left bait hopper FW0630-009 (12 V nominal) | **NO** |
| CH2 | IN2 | RH | Right bait hopper FW0630-009 (12 V nominal) | **NO** |
| CH3 | IN3 | LK | Left hook release FW0630-001 (12 V nominal) | **NO** |
| CH4 | IN4 | RK | Right hook release FW0630-001 (12 V nominal) | **NO** |

Map firmware channel integers **0→IN1 (LH)**, **1→IN2 (RH)**, **2→IN3 (LK)**, **3→IN4 (RK)**. No combined masks (e.g. 0x3) are valid. All control input pins default LOW until physically commissioned.

## Electrical harness concept — off the shelf only
```text
3S4P protected pack / BMS / master switch / rated branch fuses
    -> independent physical fail-OFF DC-rated disconnect [UNSELECTED]
         -> MDD20A propulsion bus (2 independent brushed 550 motors)
         -> EKM014 supply terminal VH; GND returns to rated power star
              CH1 +/- -> Left hopper    [FW0630-009]
              CH2 +/- -> Right hopper   [FW0630-009]
              CH3 +/- -> Left hook      [FW0630-001]
              CH4 +/- -> Right hook     [FW0630-001]
Separate premade 3S->regulated 5V (VL) -> EKM014 VL,GND
Ready-made independent hardware cutoff / pulse-limiter / one-hot / rearm modules
           [NOT SELECTED / NOT CONNECTED] -> IN1..IN4 (3.3V HIGH)
Heltec ESP32-S3 physical requests -> those independent safety modules
```

The R9/R10/R11/R12 **custom** board schematics are archived, **not** part of this wiring. Keep hardware current-zero and power-fault feedback independent of GPIO firmware. The earlier host-only `premade_module_output_r15.h` is NOT a physical safety interlock.

## Blocks before actuator power
1. Measure physical coil R/I at 3S operating extremes, energization duration, thermal buildup, inrush and BMS behavior. A nominal 12 V coil should NOT automatically receive 12.6 V without tolerance evidence.
2. Obtain EKM014 board, confirm actual connector positions, VL/VH/GND polarities, output polarity, under/overvoltage and chassis/ground behavior with a current-limited **unloaded** bench circuit. Add physical keyed harness and independent branch fusing as required.
3. Select commercially manufactured **independent** power cutoff with NC emergency STOP, manual rearm and verified DC inductive duty at measured motor stall, plus fail-OFF POR/watchdog and non-retriggerable four-way one-hot pulse-time limiter.
4. Use dummy loads only after independent hardware safety system is approved; test all four output channels including held-high, double activation, contact weld, BMS reset, empty input, timer timeout and current-zero feedback.
5. Confirm 3S-to-5V VL module meets startup/voltage current; VH must not drop below VL during fault without hardware disabling logic. Enforce ground integrity and off-state leakage.
6. Only after above, consider actual solenoids and target firmware activation. Maintain Remote VER028 / Boat VER014 marker unchanged and sonar unimplemented.

**Do not claim the 10 A per-channel switching rating means 40 A of simultaneous rail/fuse capacity. The shared board fuse is only 10 A and independent one-hot operation is a design requirement.**
