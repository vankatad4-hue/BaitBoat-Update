# R76 — Original JR-V030RX-2507 controller comparison and safety review

**Date:** 2026-11-22  
**Basis:** owner-uploaded front/rear PCB photographs; current active project status R63/R65/R66/R75; public V030/V080 receiver-board documentation.  
**Decision:** reference-only reverse engineering. No pinout, circuit values, safety claim, BOM substitution or actuator wiring is approved from photographs alone.

## 1. Identification and confidence

The uploaded PCB silkscreen reads **JR-V030RX-2507**. The board is visually consistent with the Flytec V030/V080 receiver-board family shown by the specialist Yukon Grizzly replacement-board listing and described by the V030/V080 board-set manual. The public sources do not identify the exact JR-V030RX-2507 revision or provide its schematic; do not assume YXD-V030-RX or any replacement revision is electrically identical.

The V030-08 receiver-board family is listed at **77 × 79 × 14 mm**, matching the familiar original-board envelope. The V030/V080 manual describes 2×550 brushed motors, 2.4GHz control, electronic fence and lost-signal return. These are product-family features, not proof of the exact firmware behavior or a hardware safety system in this particular PCB.

## 2. What the uploaded photographs actually show

| Visible feature | Evidence from photographs | Confidence / limit |
|---|---|---|
| PCB identity | Silkscreen **JR-V030RX-2507** | High; directly legible |
| Battery interface | Heavy red/black wires at pads marked **B+ / B−** | High; exact input range and protection topology not traced |
| GPS/peripheral interface | Six-wire connector labels **SDA, SCL, GND, +5V, RX, TX** | High; exact pin order/orientation must be recorded from the physical connector before use |
| RF connection | Coax/antenna connection marked **ANT** | High; RF IC, band and matching network not identified from photo |
| Main processor | Large central QFP IC | Visible; exact part marking unreadable |
| RF/control ICs | Smaller ICs near ANT and main processor | Visible; exact identities/functions unverified |
| Peripheral connectors | Four connectors marked **M1–M4** on right edge | High for labels; load/channel mapping unknown |
| Power conversion | Several inductors, electrolytic capacitors and regulator components | Visible; output rails and ratings not traced |
| High-current switching | Multiple MOSFET banks on reverse side; specialist replacement listing describes the underside as an H-bridge area | Strong evidence of discrete power stages; exact topology, transistor part numbers, channel grouping and ratings remain unknown |

Do not assign M1–M4 to the motors or individual hopper/hook mechanisms until each cable is traced. The board-set documentation separately describes motor, light, silo and hook connections, but it does not map those labels to this PCB revision.

## 3. Comparison with the current active project

The current R63/R65/R66/R75 project baseline is **ESP32-S3-DevKitC-1-N32R16V + HGLRC M100 PRO (with QMC5883L) + GY-91 + nRF24 primary + matched E22-900T22S backup + Cytron MDD20A + Electrokit EKM014**. The older Heltec Tracker board-integration notes are historical/superseded in the current master status; do not use their pin maps as current wiring.

| Function | Original JR-V030RX board | Current retrofit | Comparison |
|---|---|---|---|
| Main logic | One integrated main controller IC | ESP32-S3 DevKitC module | Same control role; retrofit is replaceable/modular |
| GNSS/compass | Six-wire GPS connector: I²C + UART + 5V/GND | M100 PRO with integrated QMC5883L; separate GY-91 for IMU/barometer | Strong interface similarity; verify voltage, connector and sensor bus |
| Primary RF | One ANT connection and onboard RF circuitry | External nRF24L01+ PA/LNA | Same 2.4GHz primary-control role; new antenna/module are separate |
| Backup RF | No second RF path visible | Matched E22-900T22S backup | Retrofit adds a second independent RF path |
| Two reversible motors | Integrated MOSFET H-bridge stage(s) | External Cytron MDD20A dual-channel H-bridge | Functionally close; new module is replaceable and separately mounted |
| Four switched loads | MOSFET banks and four M1–M4 connectors are visible; exact mapping unverified | EKM014 four independent low-side channels | Functionally close in channel count, but do not assume pin/logic equivalence |
| Power | Battery B+/B− enters integrated PCB with onboard conversion | Separate fused 5V/3.3V rails and separate motor/coil power branches | Retrofit is more modular and easier to isolate, but has more harnesses and interfaces |
| Safety | No separate safety relay, contactor or feedback path is visible in the photos | Independent safety chain specified in R65/R66/R17, but still not selected/assembled | Neither is presently proven to have independent hardware safety |

## 4. Hardware configurations worth carrying forward

### A. GPS connector architecture — adopt as an interface pattern

The original six signals are a useful reference:
- I²C SDA/SCL for the integrated compass/peripheral bus.
- UART RX/TX for GNSS data.
- +5V and GND for module power/reference.

The M100 PRO has a similar combination of GNSS UART and QMC5883L I²C. We can use a **six-conductor removable harness** with keyed connector and documented pinout. Do not reuse the original connector pin order or connect it directly until the actual M100 PRO connector, supply current, UART voltage and I²C pull-up voltage are verified. Both data interfaces must be 3.3V-safe at the ESP32-S3.

### B. Dual motor H-bridge architecture — retain MDD20A

The original board demonstrates the integrated discrete-MOSFET H-bridge approach. The retrofit's Cytron MDD20A provides the same essential function as a ready-made dual-channel module: two reversible brushed-motor outputs with PWM/DIR control. Cytron lists 6–30V operation, 20A continuous per channel, 60A peak, overcurrent/current limiting, thermal protection and undervoltage shutdown. It does **not** provide reverse-polarity protection. Those ratings do not establish that the 3S4P pack/BMS, wiring, connectors or motors can support the loads.

**Recommendation:** keep the selected MDD20A. Do not recreate the original discrete H-bridge on a custom PCB and do not try to reuse the original MOSFET stage until its FETs, gate drive, current path and control signals have been reverse-engineered. MDD20A's factory test buttons must be guarded against accidental full-speed activation during commissioning.

### C. Four actuator channels — retain EKM014, with an important distinction

The original board has multiple MOSFET banks and four M1–M4 connectors. The retrofit EKM014 provides four independent active-high **low-side** MOSFET outputs, 3.3V/5V-compatible control inputs and Schottky flyback protection. It is not an H-bridge and cannot reverse a motor. Its VH rail has a shared 10A slow-blow fuse; the advertised 10A per channel is not 40A simultaneous capacity.

**Recommendation:** retain EKM014 for the four hopper/hook coils. Map LH/RH/LK/RK only on the new harness; do not infer that original M1–M4 correspond in the same order. Add per-branch fuse protection only after coil current, pulse, wire and connector measurements. The EKM014 is a power switch, not a one-hot timer or safety controller.

### D. Battery distribution and wiring — adopt the service principles

Keep clearly marked B+/B− input, separate high-current and logic harnesses, short power loops, and replaceable connectors. Unlike the original integrated board, the retrofit should distribute battery power to independently fused branches:
- MDD20A motor supply;
- EKM014 VH coil supply;
- TSR 2-2450 5V EKM014 VL;
- DCDC6 5V controller/GNSS;
- separate D24V10F3 3.3V radio and sensor rails.

Use a documented power-star/return strategy and keep motor/coil return currents out of the GNSS, compass and radio signal harness. Do not copy the original PCB trace widths, component values or regulator circuits from photographs.

### E. RF and mechanical layout — adapt, do not copy

The original board uses one antenna connection. The retrofit intentionally uses two RF paths and two antenna connections (nRF24 primary and E22 backup). Use the original as a reference for strain relief and antenna exit placement only. Keep antennas and their coax/feedlines separated from motor and solenoid wiring; verify the exact antennas and EU RF settings for the selected modules.

## 5. Independent safety architecture — verdict

**No independent hardware safety architecture can be verified on the original PCB from these photographs.** The visible assembly is one integrated controller with a main processor and MOSFET power stages. There is no visible separate safety relay, power contactor, mechanically monitored contactor auxiliary, external watchdog module, independent current-zero sensors or fault-latched actuator power-disconnect path. A hidden circuit or external device cannot be ruled out without tracing the full boat wiring, but it is not evidenced here.

The V030/V080 manual describes electronic-fence and lost-signal return behavior. That is a navigation/radio failsafe feature; it is not an independent electrical emergency stop. It relies on the control electronics, position solution and motor-control path continuing to function. It cannot protect against a frozen main MCU, shorted MOSFET, welded output or corrupted actuator command.

**The retrofit is not yet independently safe either.** R65/R66/R17 specify the required external STOP, manual reset, watchdog, independently rated motor and EKM014-VH isolation, monitored contactor feedback, hardware one-hot bounded non-retriggerable coil pulse logic and physical current-zero checks. However, the exact qualified controller/one-hot device, contactors, interfaces and current-zero modules remain unselected/uncommissioned. The design requirement is more explicit than the original board, but it is not yet a built safety system.

Until those devices are selected and physically tested, do not connect live motors or coils to the retrofit or claim RTH as an emergency stop.

## 6. Reverse-engineering tasks before copying any original circuit

1. Take sharp macro photographs of both PCB sides, especially the MOSFET package markings, central MCU marking, RF IC marking, regulators and every connector silkscreen.
2. Photograph the original board with all boat harnesses still connected and identify which physical loads each connector drives.
3. With battery disconnected, map continuity from B+/B− to the motor, actuator and regulator supply nodes. Record connector pin order and polarity; do not infer it from cable colors.
4. Identify the MOSFET and gate-driver part numbers. Trace whether the reverse-side banks are two H-bridges, four low-side channels or another arrangement.
5. Measure original motor and coil current/voltage independently using a suitable current probe or meter and a controlled setup. Record 3S pack voltage, motor startup/stall behavior and coil energization/pulse/temperature. Do not deliberately stall a motor for a long interval.
6. Compare those measured loads against MDD20A, EKM014, the reported 20A BMS and proposed branch fuses.
7. Only then freeze the new harness pinout and the safety chain. Keep all actuator pins disabled until independent safety acceptance.

## 7. Decision

- **Keep** the current modular MDD20A + EKM014 architecture.
- **Adopt** the original GPS UART+I²C six-wire interface as a wiring pattern, after voltage and pinout verification.
- **Adopt** clear battery input and separate serviceable power branches, not the original integrated power PCB.
- **Do not copy** original MOSFET circuits, component values, pinouts or alleged failsafe behavior without reverse-engineering.
- **Do not mark** the original board or retrofit as having an independent hardware safety architecture.
- No component selection, firmware pin assignment, release version or actuator wiring is changed by this R76 visual review.

## Sources

- Yukon Grizzly specialist V030/V080 replacement receiver listing and underside H-bridge image captions: https://yukongrizzly.com/en/products/riadiaci-panel-pre-zavazaciu-lodku-v030
- Flytec V030/V080 board-set manual (receiver/remote board family, 2×550 motors, 2.4GHz, electronic fence, lost-signal return and board size): https://manuals.plus/ae/1005007735103605
- Cytron MDD20A manufacturer product page and technical claims: https://www.cytron.io/c-industry/p-20amp-6v-30v-dc-motor-driver-2-channels
- Electrokit EKM014 manufacturer documentation: https://docs.electrokit.com/en/modules/EKM014/
- Current project safety review: [R66](BOAT_R66_SAFETY_ARCHITECTURE_REVIEW.md), [R65](BOAT_R65_SAFETY_AND_PIN_BUDGET.md), [R17](BOAT_PREMADE_SAFETY_STACK_R17.md).
