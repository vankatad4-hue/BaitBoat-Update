# R19 status reconciliation and next gates — 2026-11-21

## Authority and scope
Read canonical continuation checkpoint R18, R15 module-only policy, R16 EKM014 freeze, R17 safety stack, R18 sourcing register, Boat/Remote master status and R15 pin/bus map before any further implementation. This document resolves conflicts introduced by newer but less complete R1/R2 draft files. No code or physical wiring approval is implied.

## Conflicts and disposition
1. Boat/docs/MODULAR_WIRING_NETLIST_R2.md shows fused battery branches feeding MDD20A and EKM014 directly. **NOT AN APPROVED ACTUATOR POWER SCHEMATIC**. Active R18 design routes BOTH through a commercially rated normally-open power cutoff/STOP contactor with monitored feedback. Correct interpretation: battery -> master/fuse -> logic branch + STOP/contactor -> motor/coil branch fuses -> drivers.
2. R2 suggests direct MCU solenoid IN1–IN4 drive with software-only one-at-a-time. **REJECTED**. R16/R17 demand independent COTS one-hot, non-retriggerable pulse limit, brownout/POR and current-zero verification; modules remain unselected, so actuator enables must stay disabled.
3. R2 proposes an unspecified 5V converter. R18 already identifies Olimex DCDC6-16-TO5 5V/2A as a Bulgarian-listed candidate; R15 lists Pololu S13V20F5 for separate 5V islands and D24V22F3 for radio 3V3. These are **candidate alternatives**, not one globally approved converter.
4. R1 BOM suggests substituting two EKM013 modules for EKM014. **NO AUTOMATIC SUBSTITUTION**: R16 owner-froze exactly one EKM014 41024394. Any change requires explicit owner approval.
5. Legacy Boat E22 pin tables and older source remain historical for boat. Boat integrated SX1262 is the intended LoRa, but its packet/protocol compatibility with Remote E22 is UNPROVEN. Do not claim backup link works.
6. Prior carrier dimensions 77x79x14mm / 30mm are retired; R18 distributed placement is active.
7. R7–R13 custom KiCad PCBs remain archived only, no custom board manufacturing or PCB/ERC milestone is an active release gate.
8. R1 shopping links are often general catalogue links. R18 VERIFIED_ELECTRONICS_SOURCING_R18.csv is the authoritative exact-SKU supplier evidence with stock and BG-shipping qualifications.
9. Earlier note that the Boat firmware is uncompiled is superseded by newer checkpoint evidence of Boat diagnostic target #22 and host tests. This is not proof of functional actuator or radio integration.

## Next executable development sequence
A. Perform exact Heltec v1.2 schematic/header/boot/SX1262/GNSS/TFT pin crosswalk and create pin-resource budget with conflict reasons; leave GPIO macros disabled until supported by board evidence.
B. Reconcile Boat SX1262 and Remote E22 at packet/modulation/frequency/SF/BW/CR/sync-word/CRC/IQ and region level. Until a validated interoperability test, backup LoRa must remain DISABLED.
C. Update the unified master BOM by referencing R18 exact SKU sourcing, adding all Remote components, supplier URL, shipping-to-Bulgaria, date, stock evidence and approved alternative status. Do not invent availability.
D. Select independently rated factory-built STOP monitor, DC contactor with mechanically linked feedback, independent no-repeat/one-hot pulse limiter, POR and current-zero modules; if no viable COTS module meets all functions, keep outputs inhibited and present engineering tradeoff to owner rather than relaxing the module-only requirement.
E. Complete connector-numbered harness drawing and separate fuse/current/thermal worksheets AFTER load measurements. Document test points, wire colors, pin1 orientation and assembly checks.
F. Keep sonar entirely unselected/unimplemented; no purchase or release claims. Retain Remote VER028 / Boat VER014 markers; release 029/015 only after required build/hardware acceptance.

## Status and acceptance
Documents only. No purchase placed, no verified bench test, no full functional target build, no validated wiring, no certified safety chain, no new release. All motor and coil enable paths remain disabled.
