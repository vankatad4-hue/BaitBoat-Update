# R6 — Direct Heltec → Cytron MDD20A implementation and electrical freeze gates

**Active architecture:** Heltec Wireless Tracker (ESP32-S3/UC6580) is the only programmable Boat controller. It controls the separate Cytron MDD20A through two PWM and two DIR signals. **No RP2040/second MCU.** Four separate DC12V solenoid drivers remain outside the MDD20A, as do the physical STOP and independent watchdog/power cutoff. This document is a development specification, **not** a signed GPIO netlist or PCB fabrication release.

## 1. What is implemented in source

- `mdd20a_direct_profile.h`: requires explicitly signed, distinct, allowed output-capable GPIO for PWM_L, PWM_R, DIR_L, DIR_R, watchdog WDI and actuator-permit signal. The allowed mask must be built from a *verified V1.2 board schematic and pin-by-pin functional audit*, not an inferred internet pinout.
- `carrier_output_plan.h`: normalized signed motor demand to 10-bit duty; independent polarity configuration, deadtime before reversal, brakes at PWM=0, and motor/coil mutual-exclusion state.
- `mdd20a_direct_safety.h`: software boundary requiring a fresh (less than 50 ms) **locally measured** STOP, external-watchdog, battery, gate and interlock report. It latches failure and requires explicit reconfiguration after a fault. A coil request cannot follow a moving motor or an unconfirmed motor-idle state.
- `mdd20a_arduino_stage.cpp`: Arduino-ESP32 v3 pin-based 20 kHz LEDC 10-bit outputs using two distinct LEDC channels. Initializes actuator permit LOW, watchdog output LOW, PWM LOW and DIR LOW; checks LEDC write failure. **Does not self-arm the hardware gate, pet the external watchdog, or drive the four solenoids.**
- No stage instance is attached in `Flytec_Boat.ino`; the default Boat build remains a *diagnostic-only, non-actuating firmware*. The new code compiles but cannot issue physical motor commands until a reviewed profile, local input driver, independent cutoff and full application attachment have been commissioned.

Cytron documents the MDD20A PWM/DIR input, 3.3 V compatibility and standard PWM operation to 20 kHz. The driver rating **does not** supersede the measured 3S4P battery, 20 A BMS, fuse and motor stall-current envelope.

## 2. GPIO budget — no fabricated pin choices

| Function | Heltec requirement | Direct GPIO or candidate peripheral | Gate |
|---|---:|---|---|
| MDD20A motor PWM1/PWM2 | 2 native PWM | Native ESP32 LEDC only | MUST verify accessible nonconflicting output GPIO |
| MDD20A DIR1/DIR2 | 2 outputs | Native ESP32 GPIO | MUST verify boot/power-off low |
| Independent watchdog WDI | 1 output | Native ESP32 GPIO | External TPS3431 and measured cutoff mandatory |
| Actuator permit | 1 output | Native GPIO to independent *fault-latched* power gate | Defaults OFF; never arm on boot |
| nRF24 CE/CSN, optional IRQ | 2–3 signals | Verified SPI and direct control | Existing dual radio retained |
| E22 backup UART, M0/M1, AUX | 5 signals, optional reset | Verified UART and GPIO | Existing backup radio retained |
| MPU9250 + ADS1115 | 2 shared I²C lines | Direct I²C | Validate bus addresses and pullups |
| Four solenoid requests | 4 independent outputs | Native GPIO **or** non-programmable expander, unselected | Hardware one-hot and independent maximum pulse |
| Physical STOP, actual motor/coil current/idle, gate/WDO and battery health | Multiple distinct inputs | Direct feedback, optional input expander | Must be physical, fresh, authenticated *locally* |
| Future sonar | **0 allocated** | Physical connector reservation only | No model and no code |

Exact accessible/usable V1.2 pins remain unverified: manufacturer public information for other revisions cannot certify V1.2 GPIO conflicts. SPI sharing with onboard SX1262, GNSS UART/power, USB, flash, straps, LCD, external radios and any oscillator/LED usage must be reconciled **before** an allowed pin mask can be signed. If native GPIO capacity is insufficient, evaluate a non-programmable expander **only for non-timing-critical coil selection and feedback**. Do not shift the independent watchdog, physical STOP or motor PWM to an I²C expander. If direct architecture cannot meet these gates, block hardware freeze and seek approval rather than silently adding a second microcontroller.

## 3. Electrical schematic capture checklist (not yet native KiCad/ERC)

**Sheet PWR_01 — battery and rails**
- PACK+ / PACK− from reported 3S4P BMS → main coordinated fuse → retained master switch → BUS_3S. No 3S connection to Tracker's onboard single-cell charger input.
- BUS_3S → separately fused MDD20A motor branch, fused electronics 5 V buck and protected coil supply branch. If actual coil range does not tolerate pack 9–12.6 V, select dedicated 12 V regulator after measuring the coil ratings; no final converter value is asserted.
- Define keying/current-rated J_PACK, J_MOTOR_PWR and star return. Motor current and coil flyback paths do not traverse sensor, GNSS or RF ground.

**Sheet MOTOR_02 — Cytron MDD20A (external board)**
- J_MDD20A_CTRL: pin 1 PWM_L; 2 DIR_L; 3 PWM_R; 4 DIR_R; 5 LOGIC_GND. This is a **proposed carrier harness pin order**, not a claim about the manufacturer's connector numbering. Verify and document actual manufacturer terminal labels before making a cable.
- MDD20A supply 6–30 V rated by manufacturer; 3S pack range requires full load validation. M1A/B → left 550; M2A/B → right 550. PWM defaults LOW on reset; DIR deterministic. Hardware cutoff must prevent power reapplication after MCU/watchdog failure. Separate MDD20A board does not fit 77×79 mm carrier.

**Sheet REL_03 — four individually switched channels**
- J_SOL_LH/RH/LK/RK: 1 protected SOL+, 2 MOSFET drain / coil−. Each has keyed removable wiring, separate low-side MOSFET with verified 3.3 V gate drive, off pulldown, individually sized flyback/clamp and branch protection.
- Independent gate logic limits pulse duration and enforces one-hot output and motor/coil mutual exclusion even if ESP32 software or GPIO expander freezes. Solenoid coil current, suppression energy, heat, release time and actual de-energized feedback **have not been measured**.

**Sheet SAFE_04 — independent electrical cutoff**
- Candidate TPS3431, WDI from completed validated application cycle, NOT timer/DMA; external watchdog output to **hardware fault latch**. Physical STOP, brownout/undervoltage, overcurrent/thermal trip and WDO assertion must revoke the independent actuator gate asynchronously.
- Fault latch powers up OFF and is not cleared by watchdog WDO deassertion or MCU reboot. A deliberate local rearm after all rails, STOP and actual off feedback are good is required.
- Candidate capacitor 150 pF and 3.3 V rail are *unfrozen*. Validate watchdog window and actual overall energy cutoff at motor and coil terminals; software receipt lease is not physical cutoff.
- TPS3431/WDI, load switch/contactor and each clamp/fuse part number, tolerance, footprint and schematic remain pending measurements and native KiCad capture.

**Sheet SENSOR_05 / CONNECTORS_06**
- UC6580 UART/read-only; calibrated MPU9250/AK8963 I²C; external ADC only after hardware/input-protection review; independent physical current/idle feedback; 3.3 V radio supply and E22 branch.
- **Reserve J_SONAR on the block diagram only**, with no power voltage, communication pin or active circuit chosen. See `BOAT_SONAR_INTEGRATION_PLAN_UNDECIDED_R1.md`.

## 4. Safe commissioning before GPIO assignment
1. Obtain exact V1.2 schematic or inspect/trace the actual board, then sign GPIO and SPI/UART function matrix. Freeze connector pins, not assumed firmware constants.
2. Verify true independent STOP and actuator power interruption; choose rated power cutoff and fuses from measured motor startup/stall and 4 coil currents.
3. Bench-test the power and MCU/LEDC stages **with actuator supply disconnected**, pin probe/logic analyzer first.
4. Only after electrical watchdog and motor/coil feedback are proven should an application-specific hardware driver be attached.
5. Run target compilation, host regressions, and real shutdown on STOP, hang, radio loss, rekey, undervoltage and coil/power feedback failure. No operation on water until physical tests pass.

**Status:** R6 software skeleton and interface budget only. No native schematic ERC, physical measurements, motor or solenoid actuation, or sonar implementation.
