# R75 — TSR 2-2450 socket, removable harness and input fuse plan

**Date:** 2026-11-22  
**Scope:** finish the mechanical/electrical connection plan for the selected TRACO TSR 2-2450 EKM014 VL regulator, and define the safe 5V distribution harness. This is a wiring/module plan, not a fabricated PCB or an electrical acceptance claim.

## 1. TRACO interface

Selected regulator: **1× TRACO TSR 2-2450**, 5V/2A, 6.5–36V input, dedicated to **EKM014 VL only**. It is a non-isolated SIP3 module. Pin mapping from the official TRACO datasheet (bottom view):

| TSR pin | Signal | Destination |
|---|---|---|
| 1 | VIN | separately fused 3S battery branch |
| 2 | GND | low-voltage star/reference ground |
| 3 | +VOUT | EKM014 VL 5V only |

The 3S pack feeds EKM014 VH separately through an independent hardware load-disconnect/safety chain. VH must be >= VL while enabled. **Do not connect TSR VOUT to EKM014 VH, solenoids, MDD20A or motor power.** The 5V regulator does not make the FET outputs fail-safe and does not replace the physical VH cutoff.

## 2. Removable SIP3 mounting candidate

**econ connect MP70S3** is a mechanical candidate:
- 1×3 female precision socket, 2.54mm pitch, 7mm stacking height.
- Manufacturer listing: 3A nominal current, -40…+105°C, PPS housing, UL94 V-0, solder tails.
- It is a PCB-mount socket; there is no verified off-the-shelf screw-terminal carrier for the TSR in this design.
- The available manufacturer listing does **not** state the exact mating-pin acceptance tolerance for the TSR's 0.46×0.46mm square pins. Do not assume fit from pitch alone.

**Required gate before purchase/lock:** obtain one MP70S3 sample and verify pin fit, insertion/withdrawal force, no contact damage, no intermittent contact after vibration, and contact resistance under the intended current. Do not force the TSR pins into a socket that is too tight or too loose. If it fails, find a precision machined SIP socket whose manufacturer explicitly documents the 0.46mm square pin compatibility.

## 3. Pigtail and locking/removable cable connection

Proposed no-custom-PCB assembly:
1. TSR plugs into the MP70S3.
2. Short, flexible pigtails are soldered to the MP70S3 solder tails; each joint receives heat-shrink and independent strain relief.
3. The pigtails terminate at a **Phoenix Contact 3-way pluggable terminal connector**:
   - cable plug **MSTB 2,5/3-ST-5,08**, order no. **1757022** (screw terminals, 12A, 3 positions, 5.08mm pitch, 0.2–2.5mm² wire range);
   - matching male header **MSTBA 2,5/3-G-5,08-RN**, order no. **1926028** (12A, 3 positions, through-hole solder tails, friction lock).
4. Header tails are wired to the protected boat harness and mechanically supported in a non-conductive holder. Plug and header are a premanufactured connector pair; no custom electrical PCB is used.
5. The MP70S3/socket and TSR receive a non-conductive 3D-printed retaining cradle/clip. The harness is clamped to the enclosure so vibration is not carried by solder joints or regulator pins.

Provisional connector pin order must be permanently marked at both ends: **1=VIN, 2=GND, 3=+5V_OUT**. Verify continuity and polarity before connecting the regulator. Do not use the same plug type for a different-voltage rail without distinct keying/labels. The pluggable terminal's friction lock is not a waterproof seal; install it inside a protected enclosure and vibration-test the assembly.

## 4. TSR input fuse

TRACO's 12V-input TSR 2-2450 datasheet recommends a **2.5A slow-blow input fuse**. A suitable exact fuse candidate is:
- **SCHURTER 0001.2508**, 5×20mm ceramic time-lag fuse, 2.5A, 250VAC / 300VDC, 1.5kA breaking capacity (manufacturer/distributor listing). This is a fuse candidate matching the maker's 2.5A slow-blow recommendation.

A holder is still a separate item. **Littelfuse 80000000005** is a panel-mount enclosed 5×20mm holder candidate, but its listings differ on current rating and its published conductor/terminal details require review. It is IP40, so it must be inside the sealed boat electronics enclosure; it is not a waterproof external holder. Confirm exact holder revision, DC rating, terminal size, conductor compatibility and mounting before purchase. Do not treat the holder as selected yet.

Fuse placement: as close as practical to the 3S distribution take-off, in the positive lead to TSR VIN. The 2.5A value is the regulator manufacturer's recommendation, **not permission to use undersized wire**. Final wire cross-section, branch current, startup/inrush and fuse time-current coordination must be checked together. Add external reverse-polarity protection; the TSR is not documented as reverse-polarity protected.

## 5. Main controller 5V rail — DCDC6 wiring constraint

The selected **Olimex DCDC6-16-TO5** is 6–16V input and 5V/2A output. Its connectors are:
- input: standard 5.5mm OD / 2.1mm ID barrel jack; center positive;
- output: USB-A host connector, 5V up to 2A.

The manufacturer's **PWR-CABLE** is a 30cm 5.5×2.1mm barrel plug to wire leads, AWG22 / 0.35mm², listed 3A. This is suitable as the input pigtail candidate for the 3S branch (9–12.6V), subject to fuse and harness checks.

**Important:** DCDC6 has no 5V screw-terminal output. A cable-only 5V split harness is required to supply:
- ESP32-S3 DevKitC-1 5V input; and
- HGLRC M100 PRO full module VCC (3.6–5.5V).

The M100 PRO UART TX and integrated QMC5883L I²C pull-up levels still must be verified 3.3V-safe before connecting to ESP32. The nRF24 Ardboard 1127 socket adapter is **not** included in the guaranteed 2A load budget: its VCC input range and thermal/current rating remain unverified. Add it to the DCDC6 split only after measured combined startup, radio burst, ripple, cable-drop and hot-enclosure tests. A second DCDC6 is not automatic; only consider a second independent 5V branch if those tests prove it necessary. **Never parallel DCDC6 outputs.**

For field use, distribute the DCDC6 USB-A 5V/GND through a protected cable harness to separately keyed/connectorized loads. For PC programming, do not simultaneously power the DevKit from the DCDC6 5V rail and a PC USB 5V source. Disconnect one source or use a verified data-only/power-blocked USB arrangement; prove no backfeed on the exact DevKit revision.

## 6. Complete low-voltage branch plan

```text
3S4P pack / BMS
  |
  +-- protected branch -> TRACO TSR 2-2450 -> 5V -> EKM014 VL ONLY
  |
  +-- protected branch -> Olimex DCDC6-16-TO5 -> 5V/2A USB-A
  |                                      +-> cable split -> ESP32-S3 5V
  |                                      +-> cable split -> M100 PRO VCC
  |                                      +-> Ardboard nRF only after load/thermal qualification
  |
  +-- protected branch -> Pololu D24V10F3 #1 -> 3.3V -> E22-900T22S ONLY
  |
  +-- protected branch -> Pololu D24V10F3 #2 -> 3.3V -> GY-91 + MCP23017 ONLY
  |
  +-- independent safety-rated/qualified cutoff -> MDD20A motor branch
  |
  +-- independent safety-rated/qualified cutoff -> EKM014 VH / coil branch
```

- Keep the two D24V10F3 outputs electrically separate; never parallel outputs. Separate input fuses/protection and output wiring. Share signal ground only as required; keep high-current motor/coil return paths away from sensor and MCU return.
- TSR, DCDC6 and both D24V10F3 branches are separate converters; no 5V or 3.3V output cross-feed.
- Final fuse ratings for DCDC6 and the two D24V10F3 branches are **not assigned** until measured loads, startup current, conductor sizes and connector ratings are known.
- Main motor and solenoid power never pass through logic regulator wiring.

## 7. Acceptance checks — NOT RUN

1. Confirm TSR pin 1/2/3 orientation and MP70S3 physical mating sample.
2. Four-wire/contact-resistance or equivalent low-resistance check on socket contacts; continuity/polarity check through the complete pigtail and plug.
3. Mechanical retention and vibration test with the regulator installed; no pin movement, intermittent output or solder-joint loading.
4. Confirm Phoenix 1757022 / 1926028 mating, terminal torque, wire range and retention in final cradle.
5. Verify fuse and holder manufacturer's DC rating, current derating, terminal conductor range and enclosure mounting. Confirm branch wire protection.
6. Bench-test TSR at 9.0, 11.1 and 12.6V with EKM014 VL only; measure startup, 0-to-measured-VL load step, ripple, overshoot, current, fuse/connector temperature and enclosure temperature. VH remains disconnected.
7. Test DCDC6 5V output under ESP32 boot, GNSS startup and radio bursts; measure total current, output drop, ripple, temperature and USB backfeed.
8. Test M100 UART/I²C levels and backfeed before MCU connection.
9. Test both D24V10F3 rails independently under E22 TX and GY-91/MCP traffic, including 3S sag, I²C pull-up and MCU-off backfeed.
10. Confirm physical STOP and independent motor/coil power isolation before connecting any live actuator.

**No hardware purchase, physical fit test, power-on, actuator test or GPIO commissioning is claimed. Boat VER014 / Remote VER028 remain frozen.**
