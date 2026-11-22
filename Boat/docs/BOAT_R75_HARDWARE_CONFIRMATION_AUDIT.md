# R75 — Boat hardware confirmation audit

**Date:** 2026-11-22  
**Branch:** hardware-ready-dual-radio-r1  
**Audit basis:** current purchase register R20, Boat master status, R65 pin-resource crosswalk, R66 independent-safety review, R67 sensor map, R68 power BOM/acceptance plan, R73 M100 integration and R74/R75 power updates.

## Executive result

The component architecture is substantially selected, but **the boat is not yet a work-ready powered prototype**. The main remaining blockers are independent physical safety, measured motor/solenoid currents, exact module/carrier interfaces, protected power harnesses, and physical commissioning. A passing host test or source audit does not prove an electrical module is wired or safe.

Status meanings:
- **Owner-confirmed model:** exact model or SKU agreed; not purchased.
- **Selected for design:** current engineering selection, not a purchase order.
- **Candidate / unresolved:** model, interface, quantity, fit or ratings still need owner/manufacturer/bench confirmation.
- **Retained:** existing asset; condition/current ratings not verified.

## 1. Main electronics and sensors

| Item | Current decision | What remains to confirm |
|---|---|---|
| Main MCU | ESP32-S3-DevKitC-1-N32R16V exact board family selected | Actual purchased board revision, J1/J3 header mapping, physical pin continuity, boot/strapping behavior and final harness. GPIO39–42 repurpose/JTAG effects need hardware validation. |
| GNSS + integrated compass | HGLRC M100 PRO with onboard QMC5883L selected; no separate QMC board | Exact module variant, supply/current at 5V, connector pinout, UART TX voltage and I²C pull-up voltage. Must be 3.3V-safe or use rated prebuilt level conversion. |
| IMU + barometer | GY-91 (MPU-9250 + BMP280) selected | Genuine MPU-9250/BMP280 identities, exact breakout supply and logic levels, I²C address, mounting orientation, magnetic calibration and vibration/temperature behavior. |
| I/O expander | Waveshare MCP23017 IO Expansion Board SKU15391 selected | Actual board revision/address jumper (expected factory 0x27), supply and pull-up voltage, terminal/harness pinout, reset behavior and safe startup states. It is only a request-intent interface, not a safety controller. |
| Primary radio | nRF24L01+ PA/LNA architecture retained | Exact PA/LNA module supplier/SKU and antenna connector; Ardboard 1127 adapter remains user-proposed, not qualified. Adapter VCC input, regulator thermal/current, decoupling and peak TX behavior must be measured. |
| Backup LoRa | Matched EBYTE E22-900T22S pair owner-confirmed in R69 | Factory-made SMD-to-header carrier at both ends, 3.3V UART levels, carrier VCC, AUX/M0/M1 wiring, RF settings, antenna and EU868 lawful power/duty. Confirm the 3.3V rail can support the selected RF output level; no RF test yet. |

## 2. Motor and actuator hardware

| Item | Current decision | What remains to confirm |
|---|---|---|
| Motors | Existing 2× brushed 550 motors retained; differential steering, no rudder | Measure each motor free-running, loaded, startup and stall current at 9.0/11.1/12.6V. Confirm direction, prop load, cable size, suppression, temperature and simultaneous peak current. |
| Motor driver | Cytron MDD20A dual-channel selected | Exact board revision and terminal pinout, reverse-polarity protection, physical mounting outside the earlier 77×79mm bay if needed, heatsinking, PWM/DIR boot-safe states and test-button guarding. Its 20A/channel rating does not prove the 20A BMS can supply both motors. |
| Hopper coils | 2× FW0630-009 retained/selected | Actual resistance and current at 9.0/11.1/12.6V; peak/hold current, release travel/time, maximum safe pulse, flyback, coil temperature and mechanical load. |
| Hook coils | 2× FW0630-001 retained/selected | Same electrical and thermal measurements as hopper coils; verify independent mechanical release. |
| Solenoid switching | Electrokit EKM014 41024394 owner-confirmed | Verify actual board revision and terminal pinout, shared 10A fuse, individual output current/thermal behavior, inductive flyback and physical mounting. Four outputs do not provide one-at-a-time arbitration or a hardware pulse deadline. |
| EKM014 supplies | TSR 2-2450 selected for VL; separate raw 3S safety-cutoff path for VH | Measure VL current and 5V stability. Prove VH >= VL during operation and prove physical STOP disconnects VH even with MCU output stuck HIGH. Never treat VL shutdown as coil-energy isolation. |

## 3. Power system and protection

| Rail / source | Current decision | What remains to confirm |
|---|---|---|
| Battery | Existing 3S4P 18650 pack, reported 20A BMS | Confirm cell model/health/matching, pack capacity, BMS continuous/peak current and trip behavior, cell/busbar temperature, voltage sag, actual low-voltage cutoff and connector. The “20A” rating is not yet a measured pack capability. |
| Main 5V | 1× Olimex DCDC6-16-TO5 selected | Its 5V/2A USB-A output must supply ESP32 + full M100 PRO through a protected cable split. Measure startup and RF bursts. PWR-CABLE is a female-barrel pigtail; the Olimex listing conflicts on 2.1mm versus 2.5mm inner diameter and lists 12V/3A while the 3S pack reaches 12.6V. Verify exact fit, voltage rating at 12.6V and center-positive polarity against the board schematic. Input branch fuse and wiring still need installation. |
| EKM014 VL 5V | 1× TRACO TSR 2-2450 selected | MP70S3 sample fit, Phoenix plug/header pigtail, fuse holder and final conductor coordination. Measure ripple, transient, current and hot-enclosure temperature. |
| 3V3 LoRa | 1× Pololu D24V10F3 #2830 owner-confirmed | E22 carrier VCC and 3.3V load/thermal tests; dedicated fused input. |
| 3V3 sensors | 1× Pololu D24V10F3 #2830 owner-confirmed | GY-91 + MCP23017 total current, sensor noise, I²C pullups and power-sequence/backfeed. Keep separate from LoRa output. |
| nRF24 supply | Ardboard 1127 socket adapter proposed for investigation | Seller does not specify input range or guaranteed continuous/thermal output. Do not assume it is a 3S regulator. Use only regulated 5V after testing, or a separate approved supply. |
| Branch protection | No complete approved fuse/terminal/harness set | Select DC-rated fuse and holder for each branch from measured load, wire ampacity, inrush and connector ratings. Reverse-polarity protection, main fuse, master disconnect and keyed harnesses remain open. |

## 4. Independent safety — critical blocker

There is **no complete confirmed independent hardware safety chain** yet.

| Required safety function | Current state | Confirmation still required |
|---|---|---|
| Physical emergency STOP | Candidate Duelco NST-3.2 42042237, not wired | Exact revision, approved dual-channel wiring, manual monitored reset, low-voltage/sag operation and manufacturer fault behavior. |
| Motor power isolation | Albright SW60 family research candidate | Exact full SW60C SKU with continuous-duty 12V coil and mechanically trustworthy NC auxiliary feedback; DC inductive interruption rating at measured motor stall current; weld-fault behavior. |
| Coil/VH isolation | Separate contactor path required | Exact contactor rating for coil branch, independent of EKM014 VL; prove VH is disconnected on STOP/fault. |
| Watchdog | Brentek P8D-ISM5 candidate | Exact socket/programming, frozen-high heartbeat detection, 5V input and non-auto-rearm/manual reset behavior. |
| 4-channel one-hot pulse controller | **No exact ready-made qualified unit selected** | Must reject simultaneous requests, power-up all OFF, issue one bounded non-retriggerable pulse, require all requests LOW and current-zero before next event, and fail OFF on MCU freeze/brownout. |
| Current-zero proof | No independent verified sensor/interlock selected | Four coil channels and both motor channels need physical current-zero threshold feedback independent of ESP32 telemetry. |
| Brownout / power-on latch | Unresolved | A sag below converter minimum must open load branches and prevent auto-rearm after supply recovery until local manual reset. |
| 24V auxiliary rail | MEAN WELL DDR-30G-24 candidate only | Minimum 9V input is close to 3S low-voltage edge; verify pack sag, total safety-controller/contactor load, power-on and no auto-restart. |

The R66/R17 requirements remain binding: no direct ESP32 or MCP23017 output to EKM014 IN1–IN4; no solenoid actuation until the independent one-hot pulse limiter, physical STOP, current-zero and VH isolation are verified.

## 5. Mechanical, wiring and environmental

Still to confirm:
1. Real hull mounting locations for MDD20A, EKM014, power converters, safety relay/contactor and antennas. The previous 77×79mm controller-bay limit is no longer the design constraint, but actual mounting/clearance still must be surveyed.
2. Actual wire lengths, cross-sections, temperature ratings, voltage drop and separation of motor/coil returns from MCU/sensor grounds.
3. Keyed locking connectors and exact pinouts for all removable modules; no loose Dupont jumpers in the boat.
4. Water ingress, condensation, vibration, strain relief, corrosion and thermal behavior of the enclosure and cable glands.
5. Main fuse, per-motor fuse, coil fuse, converter input fuses and reverse-polarity protection coordinated with the real conductors and load current.

## 6. Deferred / out of scope

- CHIRP sonar model remains undecided and intentionally deferred.
- No boat SD-card module is required; offline maps and waypoint storage are on the remote.
- No custom electrical PCB design or fabrication is permitted under the current modular-module-only decision.
- No official Boat release bump: Boat VER014 and Remote VER028 remain frozen pending target and physical acceptance.

## 7. Priority order to reach a testable boat

**P0 — safety and power envelope**
1. Measure the real 3S pack/BMS sag and motor/coil currents.
2. Select and verify the independent STOP + contactor + watchdog + manual reset architecture.
3. Select a factory-made four-channel one-hot, bounded-pulse, no-repeat controller and independent current-zero feedback.
4. Complete DC-rated fuses, wire sizes, reverse-polarity protection and separate motor/VH cutoffs.

**P1 — finish module interfaces**
5. Qualify TSR SIP3 socket/pigtail and the DCDC6 5V split harness.
6. Verify M100 PRO UART/I²C levels and the exact ESP32 header mapping.
7. Verify GY-91/MCP23017 I²C addresses/levels and both D24V10F3 rails.
8. Select compatible E22 carrier and exact nRF24 module/adapter.

**P2 — physical commissioning**
9. Mount modules, separate high-current and logic harnesses, add strain relief and waterproofing.
10. Bench-test power rails and safety using current-limited dummy loads.
11. Test motors and each coil individually only after independent safety acceptance.
12. Run radio range/EMI tests, GPS/heading calibration, then full boat acceptance.

**Current readiness:** component model selection is partly complete; purchase, physical qualification and commissioning are not. Host tests are software/static evidence only. Do not energize motor or solenoid branches.


## R75A — socket-fit review update

The **BINDER MPE 006-1-003-D-B1STF-XS0** is now the primary dimensioned SIP3 socket candidate (2.54mm pitch, 3A, 4-finger BeCu clip). Its published square mating-pin size is 0.25×0.45mm; TRACO specifies 0.46×0.46mm nominal with ±0.1mm tolerance. The nominal difference is only 0.01mm, but compatibility is **not guaranteed**. Obtain one sample and request manufacturer confirmation before locking. econ connect MP70S3 is demoted to secondary because its listing gives a round pin form and does not publish a square mating-pin tolerance. If neither sample passes, use a direct-solder pigtail service method rather than forcing an unsuitable socket.
