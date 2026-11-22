# R18 — Distributed, prefabricated Boat module placement

> **R19 ACTIVE BOM:** Use [BOAT_HARDWARE_SELECTION_R19.md](BOAT_HARDWARE_SELECTION_R19.md) for the current ESP32-S3-DevKitC-1-N32R16V, M100 PRO, GY-91, nRF24, E22-900T22S, MDD20A, EKM014 and selected regulators. The older Heltec/SX1262 power table is historical, not procurement authority.

**Owner revision 2026-11-21:** The former 77 × 79 × 14mm carrier and ~30mm lid clearance are **RETIRED AS HARD LIMITS**. Modules may be independently mounted **along the Flytec V080 hull**, not forced onto a single compact carrier. The exact hull interior still needs physical measuring (access, water exposure and service clearances). **No custom PCB, no backplane and no soldered homemade circuit board.**

## Modular zones — placement concept, not approved dimensions
| Zone | Ready-built functions | Mounting/harness priority |
|---|---|---|
| A — protected pack switching | Existing 3S4P pack/BMS; commercial master switch, fuse holder, DC-rated main contactor and manual STOP/supervisor candidates | Closest practicable to battery positive, low-loss short protected cables; independently serviceable and inaccessible to splash; high-current wiring only |
| B — drive power | **Cytron MDD20A**, 2 × brushed 550 motors | Separately mounted next to motor harness path; allow wire bend radius, cooling and ventilation; measure stall/ripple and waterproofing |
| C — four coil outputs | OWNER-SELECTED **Electrokit EKM014**; CH1 LH bait, CH2 RH bait, CH3 LK hook, CH4 RK hook | Near the four solenoid load harnesses, avoid radio/compass, verify board hole-centres, reserve coil flyback and fuse terminals; 1 active coil at a time |
| D — low-voltage supplies | Premade RECOM R-78E3.3-0.5 safety 3V3; premade separate 5V rails (Olimex DCDC6-16-TO5 candidate); separate nRF 3.3V | High-current returns excluded from logic star ground; 5V Olimex needs barrel input and USB-A output factory harness; do not parallel regulated outputs |
| E — navigation/comms | Heltec Wireless Tracker v1.2, MPU9250/BMP280 module, nRF24L01+ PA/LNA module and antenna | GNSS sky-view and antenna separation, calibrated magnetic compass away from motor conductors, removable low-current cables |
| F — independent STOP/timing | Commercial stop supervisor, DC contactor feedback, commercial 4-way pulse one-hot/current-zero/POR (all UNSELECTED) | Small DIN/safety contactor units can occupy a separate insulated/waterproof bay; ample free space for qualified mounting and fault servicing |

### Mechanical design rule changed
**Do not reject parts because they exceed the previous 77×79mm board or 30mm cover space.** The Duelco NST-3.2 and Albright SW60C/A are now allowed to be considered irrespective of those historical dimensions. Their total dimensions, weight, shock/vibration, cable termination, waterproofing/condensation, service access, heat rejection and effect on balance are **still practical acceptance requirements**. Plastic 3D printed enclosure/mounting supports are allowed; custom electronic PCB is not.

### Independent supply and current routing
```text
3S4P BAT + BMS -> master switch / fused distribution
    +-> small protected unswitched logic branch: commercial 3.3V safety regulator,
    |    premade 5V supplies, Heltec and safety supervision
    +-> rated COTS fail-OFF contactor / hardware STOP (not selected)
          +-> dedicated fused motor supply -> Cytron MDD20A -> left/right 550
          +-> dedicated fused solenoid VH -> Electrokit EKM014
                                      CH1/IN1 Left Hopper
                                      CH2/IN2 Right Hopper
                                      CH3/IN3 Left Hook
                                      CH4/IN4 Right Hook
Separate 5V VL -> EKM014 gate driver (DO NOT connect raw 12.6V charged 3S)
Independent premade 4-way one-hot/no-repeat limiter -> EKM014 IN1..IN4 (not selected)
```

The topology is conceptual and **not yet a wiring instruction**: high-current STOP/contactors, fuse values, physical current-zero and off-power behavior are not qualified.

### Distributed-mount acceptance checklist
1. Measure actual usable hull bay zones and safe cable passages, including waterproof compartment options and battery access; no hard single-board mechanical footprint.
2. Keep battery fuse as close as practical to pack output. Use a commercial appropriately rated enclosed DC fuse/terminal/fork ring assembly and strain relief; no unprotected long conductor.
3. Separate EMI-loud motor power from GNSS magnetometer, SD and nRF radio; route looped pairs and suppress noise only using manufacturer-approved add-on modules.
4. Maintain IP-rated housings for splash/condensation and isolate terminals; if modules need cooling, validate heat rise under worst load rather than sealing hot power electronics.
5. Provide keyed removable factory-made cables with documented colors, pin numbers and service loops. Ground-reference control inputs correctly; do not share MOSFET/load current through a GPIO ground wire.
6. Weigh actual components and check hull balance/waterline after placement; do not infer buoyancy or service clearance from schematic dimensions.
7. All low-voltage power modules must be independently tested with current-limited supplies before any driver is energized. Every EKM014 actual-coil test remains gated by independent hardware STOP/timeout/onehot proof.

## Procurement
Follow [R18 electronics shop policy](../../docs/APPROVED_ELECTRONICS_SUPPLIERS_R18.md) and [sourcing evidence CSV](../../docs/VERIFIED_ELECTRONICS_SOURCING_R18.csv). Bulgaria-first: Farnell BG, Comet BG, Olimex Plovdiv, Ardboard. Manufacturer/specialist EU source allowed for **exact** EKM014 and MDD20A when Bulgarian sources are absent (actual BG shipping unverified). AliExpress can be considered only after manufacturer/seller authenticity review; no AliExpress supplier was approved in this research.
