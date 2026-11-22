# R74 — 5 V regulator selection: Olimex DCDC6-16-TO5 + TRACO TSR 2-2450

**Date:** 2026-11-22. **Owner direction:** DCDC6-16-TO5 confirmed for the main 5 V rail, one unit initially; a second unit is conditional only if load/EMI/segregation testing proves necessary. Continue with TRACO TSR 2-2450 as the dedicated EKM014 VL logic regulator. Both are selected for design, **not purchased or physically commissioned**.

## Selected modules and roles

| Rail | Qty now | Exact part | Output | Input | Loads | Status |
|---|---:|---|---:|---:|---|---|
| BOAT_5V_MCU | 1 | Olimex DCDC6-16-TO5 | 5V / 2A | 6–16V | ESP32-S3 DevKitC-1 + HGLRC M100 PRO (integrated QMC5883L); filtered Ardboard 1127 nRF adapter only if aggregate load/noise test passes | Owner-confirmed model |
| BOAT_5V_GATE | 1 | TRACO TSR 2-2450 | 5V / 2A | 6.5–36V | Electrokit EKM014 **VL only** | Owner-directed selection |
| BOAT_3V3_E22 | 1 | Pololu D24V10F3 #2830 | 3.3V / 1A | 3.4–36V | E22-900T22S only | R72 owner-confirmed |
| BOAT_3V3_SENSORS | 1 | Pololu D24V10F3 #2830 | 3.3V / 1A | 3.4–36V | GY-91 + MCP23017 only | R72 owner-confirmed |

**DCDC6 quantity policy:** start with one DCDC6-16-TO5 feeding the main 5V domain. A second is *not* automatically added to the BOM. Add a second independent DCDC6 only if bench measurements show the main rail cannot maintain the specified voltage/current/thermal margin or if the nRF adapter needs a separately conditioned 5V source. If added, assign distinct loads and outputs; **never parallel 5V outputs**. Do not connect DCDC6 USB output and DevKit USB/5V pin concurrently unless a verified isolation arrangement is present. Its barrel input/USB-A output require a secure, keyed harness and protected distribution to both ESP32 and M100 PRO.

## TRACO TSR 2-2450 official specifications and 3S fit

Manufacturer: [TRACO POWER official TSR 2 datasheet](https://www.tracopower.com/sites/default/files/products/datasheets/tsr2_datasheet.pdf), 2A SIP3 family, 12V-nominal-input variant:
- **VIN 6.5–36V**, fixed **5V / 2A**, nominal efficiency **94%**, output set accuracy **±2%**.
- 3S Li-ion range 9.0–12.6V is inside the stated operating range, with 2.5V margin above minimum at 9.0V.
- Ripple/noise: **50mV peak-to-peak typical** (20MHz bandwidth, 12V model); input regulation 0.5% max; load regulation 1% max.
- 5ms typical start-up, continuous automatic-recovery short-circuit protection, foldback overload protection, automatic thermal shutdown (150°C typical internal IC), no minimum load.
- Switching frequency for this **12V-input variant is 410kHz typical**. Ignore distributor pages that show 1.2MHz for this exact 12V model; official datasheet differentiates 1.2MHz 5V-input models from 410kHz 12V-input models.
- SIP3 through-hole package, **14.0 × 7.5 × 10.1mm**, pins (bottom view): **1 = +VIN; 2 = GND; 3 = +VOUT**. Non-isolated converter; common ground required.
- Datasheet lists a **2.5A slow-blow recommended input fuse for 12V-in / 5V-out model**; final branch fuse must also be checked against wire ampacity, measured VL draw, startup, battery harness and applicable protection policy. No explicit reverse-polarity protection specification is claimed; add upstream reverse-polarity protection.
- Full-load ambient operation is advertised to +67°C without heatsink for family, but actual enclosure derating and case temperature must be bench verified. Do not use nameplate 2A as guaranteed hot-enclosure continuous capacity.

## EKM014 compatibility

[Electrokit EKM014 manufacturer documentation](https://docs.electrokit.com/en/modules/EKM014/) specifies:
- VL gate-driver logic supply **4.5–12V**; 5V from TSR is within range.
- VH FET supply **5–50V**, and **VH must be >= VL**. The boat 3S pack supplies VH independently (nominal 9–12.6V), while TSR provides 5V only to VL. Keep VH and VL separate; do not bridge them.
- IN1–IN4 accept 3–12V logic, so ESP32-S3 3.3V outputs are compatible at the published logic threshold, subject to wiring and grounding verification.
- EKM014's VH is the actual solenoid energy source. TSR must never supply the 6A-class solenoids, their coils, MDD20A motors, or the EKM014 VH input.
- The UCC27524 gate driver's advertised 5A is peak gate-drive output capability, not 5A continuous consumption from VL. EKM014 does not publish the assembled board's measured VL current; verify at bench under the intended pulse/control activity. 2A is a strong candidate margin, not a substitute for measurement.
- Use independent physical safety cutoff on VH, and independent hardware one-at-a-time pulse interlock. Powering VL does not establish that FET outputs are safe or disabled.

## Mechanical / modular integration constraint

The TSR 2-2450 is a premanufactured encapsulated SIP3 converter **not a screw-terminal board**. No custom PCB is authorized. Before purchase, source/approve a premade 3-pin SIP socket / locking harness or a suitable premade carrier arrangement with mechanical retention and protected terminals. Do not leave unsupported pins or loose Dupont wires in the wet/vibration-prone boat. Preserve replaceable-module architecture and keyed removable cable harnesses.

## Main 5V DCDC6 aggregate load

DCDC6-16-TO5 is a 6–16V input, 5V/2A output module with barrel input and USB-A output. It is selected as the main controller supply for ESP32-S3 + M100 PRO (which includes QMC5883L and requires 3.6–5.5V module input). Ardboard 1127 nRF socket adapter may share only if verified 5V input compatibility and combined load/ripple/thermal headroom pass. The 2A output budget must be tested at battery 9.0/11.1/12.6V, ESP32 boot and RF bursts, GNSS, nRF TX, cable drops and hot enclosure. Do not add a second DCDC6 until evidence justifies it; if required, separate branch, no output paralleling. USB backfeed prevention remains mandatory.

## Procurement evidence (snapshot, not order)

- [Farnell Bulgaria exact TSR 2-2450, order code 2848984](https://bg.farnell.com/traco-power/tsr-2-2450/dc-dc-converter-5v-2a/dp/2848984): listing showed **5,022 units** available at research, **€12.05 excluding VAT** per unit; checkout delivery and final stock must be rechecked. Farnell is an accepted verified distributor.
- [TME Bulgaria product family listing](https://www.tme.eu/bg/katalog/p%2Ctraco-power_548/): search result showed TSR 2-2450 and stock/pricing snapshots but direct product page stock differed by region/session; use Farnell BG as primary sourcing evidence, not an assumed TME reservation.
- [TRACO official datasheet](https://www.tracopower.com/sites/default/files/products/datasheets/tsr2_datasheet.pdf).

## Acceptance gates — all NOT RUN

1. Verify physical module MPN, pin orientation and SIP3 socket/harness; continuity check 1=VIN, 2=GND, 3=VOUT.
2. Current-limited bench supply at 9.0, 11.1, 12.6V; output setpoint and no-load/full-load output; check 4.9–5.1V tolerance.
3. Scope startup, shutdown, 0–measured EKM014 VL load steps, ripple and overshoot; no unexpected IN1–4 pulses. EKM014 VH physically disconnected for logic-only test.
4. Measure actual EKM014 VL draw at idle and during intended gate activity. Check regulator case temperature in final enclosure and fuse/cable heating.
5. Confirm EKM014 VH remains independent and >= VL during normal operation; physical STOP disconnects VH; no VH/coil energy is sourced from TSR.
6. Verify 3S undervoltage shutdown and no automatic actuator re-arming after reset or brownout.
7. Test common reference ground and 3.3V ESP32 control levels; no 5V injection into GPIO.
8. For DCDC6, test ESP32 + M100 PRO + conditional nRF adapter aggregate startup/radio load and USB backfeed; add second unit only if evidence requires it.

**No hardware purchased or powered. No GPIO/motor/solenoid commissioning. No release ZIP or version bump. Boat VER014 / Remote VER028 remain unchanged.**


## R75 — socket, connector and fuse implementation plan (2026-11-22)

- Mechanical SIP3 candidate: **econ connect MP70S3**, 1×3 precision socket, 2.54mm pitch, 3A nominal. The manufacturer listing does not state the exact mating tolerance for the TSR's 0.46mm square pins; sample fit and vibration/contact-resistance test are mandatory before purchase lock.
- Removable 3-wire pigtail candidate: **Phoenix Contact MSTB 2,5/3-ST-5,08 (1757022)** screw-terminal cable plug + **MSTBA 2,5/3-G-5,08-RN (1926028)** 3-position male THT header. 12A rated, 5.08mm pitch; solder header tails to supported pigtails, no custom PCB. Check exact mating/retention and enclosure fit.
- Fuse candidate matching TRACO recommendation: **SCHURTER 0001.2508**, 2.5A time-lag 5×20mm ceramic, 250VAC/300VDC. Fuse holder not yet selected; Littelfuse 80000000005 is only a candidate pending exact DC rating, current revision, conductor and terminal review; IP40 means enclosure-only.
- Main DCDC6 input pigtail candidate: **Olimex PWR-CABLE**, 30cm AWG22/0.35mm², 3A, 5.5×2.1mm barrel plug. DCDC6 output remains USB-A 5V/2A; an assembled cable-only split is needed for ESP32 + M100. Ardboard nRF adapter is excluded until the combined 2A load/thermal/noise tests pass. No simultaneous PC USB and external 5V unless power-blocked/data-only cable is verified.
- Full pinout, distribution diagram, protection notes and bench gates: [R75 TSR socket/harness/fuse plan](BOAT_R75_TSR_SOCKET_HARNESS_AND_FUSE_PLAN.md). Full boat hardware confirmation status: [R75 audit](BOAT_R75_HARDWARE_CONFIRMATION_AUDIT.md). Physical acceptance matrix: [R75 NOT_RUN tests](BOAT_R75_HARDWARE_ACCEPTANCE.csv).
- Purchase register and static CI updated. All R75 mechanical/fuse parts are candidates, not owner-confirmed purchases. No hardware powered or actuator enabled; Boat VER014 / Remote VER028 unchanged.
