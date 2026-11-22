# R65 — Module-only safety procurement and N32R16V pin-budget feasibility

**Date:** 2026-11-22. **Status:** verified manufacturer specifications + **UNCOMMISSIONED pin proposals**; no physical actuator or controller setup.
**Supersedes:** old Heltec Tracker R7/R8 and R15 Boat pin candidates; retains module-only R15 policy, owner-selected ESP32-S3-DevKitC-1-N32R16V, EKM014 41024394, MDD20A, paired E22-900T22S. Sonar remains deferred.

## Critical selection conclusion

A complete ready-made, independently verified safety chain **has NOT yet been identified or approved**. We cannot treat a software one-hot limit, a powered EKM014, an ESP32 internal watchdog, or a simulator as physical safety. No supply/output harness is accepted for power-on.

| Function | Part / identified manufacturer evidence | Outcome / hard blocker |
|---|---|---|
| Emergency STOP monitor | **Duelco NST-3.2 42042237**, 8–36 V DC; [manufacturer](https://www.duelco-safety.com/products/emergency-stop-relays/emergency-stop-relay-nst-32); [OEM Automatic exact part](https://www.oem.ee/products/safety/safety-relays/emergency-stop-relays-_-c32656/duelco---nst-3-2-safety-relay-_-p1795306) | **CANDIDATE**, not rated to carry motor/solenoid current; manual **monitored** reset and external contact feedback essential. Manufacturer output limits from R17: 3A @ 24VDC resistive; safety category is a device attribute, not an automatic whole-vessel claim. BG stock unverified |
| Heartbeat + rail monitor | **Brentek P8D-ISM5**, 4.5–5.5 V, [manufacturer](https://www.brentek.com/watchdog-timer-modules/p8d-ism-industrial-supervisory-modules) and [datasheet](https://www.brentek.com/sites/default/files/uploads/attachments/products/P8D-ISM%20Industrial%20Supervisory%20Modules_0.pdf) | **RESEARCH CANDIDATE**, requires separate verified stable 5V, correct octal socket/configuration, fault lockout, and compatibility between its 3–30 V optoisolated trigger and ESP32 3.3V. NOT safety-certified as a complete interlock; BG shop not confirmed |
| Heartbeat + rail monitor 12V option | **Brentek P8D-ISM12** manufacturer datasheet | **REJECT DIRECT 3S**: min operating 10.9V, so it would switch off before lower portion of pack use and may not behave as intended under sag. Can be reconsidered only with an independent regulated 12V supply and rail+load proof |
| Contact switching | **Albright SW60C/SW60A** 12V 100%-duty with mechanically appropriate auxiliary [manufacturer](https://www.albrightinternational.com/products/sw60/) | **CANDIDATE**; manufacturer publishes 80A thermal and DC inductive duty, but complete suffix/coil pick-up at sag, actual motor+coil DC-breaking curves, coil suppression and mirror-contact relation to welded load must be confirmed. Thermal current rating alone is insufficient |
| Independently controlled four-way pulse | **NONE SELECTED** | **BLOCKING**: mutually exclusive selection, held-high lockout, pulse duration under tolerance, no repeat after brownout, hard max time and current-zero feedback all remain unmet |
| Alternative configurable safety-controller approach | **Pilz PNOZ m B0 772100** four safe semiconductor outputs [manufacturer](https://www.pilz.com/en-INT/eshop/product/772100) | **RESEARCH ONLY**: requires 24V supply, ordered terminals/chipcard, independently validated 4-way no-repeat/timer program, 24V-to-EKM014-compatible prebuilt interface, monitored cutoff and verified current-zero. Do not connect 24V outputs directly to EKM014 or 3.3V ESP32. High size/cost and programming verification may make unsuitable |
| 24V rail for alternative industrial logic | **MEAN WELL DDR-15G-24**, input 9–36V, 24V/0.63A 15W [Bulgarian specialist listing](https://meanwell.bg/product/ddr-15g-24) | **EXACT BULGARIAN LISTING**, seller reports stock (not checked out); DOES NOT imply rail sufficiency for Pilz plus other equipment or 3S performance below 9V, and does not act as emergency cutoff |
| Four low-side switches already selected | **Electrokit EKM014 41024394**, [manufacturer documentation](https://docs.electrokit.com/en/modules/EKM014/) | **FROZEN MODEL**, not purchased. VL 4.5–12V, VH 5–50V, logic active high, **shared 10A slow-blow VH fuse**. 6A-class individual coil current and inductive flyback tests remain necessary. Pins must be driven through independent onehot/nonrepeat safety gate; no direct powered GPIO |

**Procurement:** verified product/manufacturer listings are not stock+shipping confirmations. Prefer Farnell BG, Comet, Olimex, Ardboard, Mouser/DigiKey Bulgaria or manufacturer/known EU electronics specialists. No eMAG/OLX/anonymous shops. Do not issue an unconditional shopping cart until the mandatory one-hot independent cutoff is proven and each part has an exact complete SKU.

## N32R16V pin budget and tentative interface plan

Espressif [official DevKitC-1 v1.1 header tables](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html) identify J1/J3 pins; [WROOM-2 datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-2_datasheet_en.pdf) gives **1.8V GPIO47/48** for V variants and reserves memory interface on GPIO35/36/37. GPIO38 is v1.1 RGB LED; v1.0 RGB LED uses GPIO48. Preserve native USB19/20, boot straps GPIO0/3/45/46, and UART0 43/44; JTAG 39–42 require explicit repurposing tests. **No candidate assignment constitutes approval.**

- Full 44-pin header crosswalk: [R65 pin-resource CSV](BOAT_R65_ESP32S3_PIN_RESOURCE_CROSSWALK.csv). Each proposed signal is **candidate only**, and is NOT copied into active firmware pin macros.
- Primary nRF24: GPIO9 CE, 10 CSN, 11 MOSI, 12 SCK, 13 MISO (5).
- Backup E22 UART: GPIO14 M0, 15 M1, 16 AUX, 17 TX, 18 RX (5); assumes matching **premade E22-900T22S carrier still unverified**.
- GNSS UART: GPIO4 RX from GNSS TX, GPIO5 TX to GNSS RX (2).
- Low-speed I2C: GPIO8 SDA, GPIO21 SCL (2); compass/IMU/feedback expanders must be verified at 3.3V and on nonconflicting addresses.
- MDD20A *logic only*: GPIO1 PWM1, 2 DIR1, 39 PWM2, 40 DIR2 (4); JTAG pin repurposing and independent physical power removal required.
- Candidate ESP32 watchdog heartbeat GPIO6 (1); advisory physical STOP/contactor/coil-zero status GPIO7/42/41 (3).
- **22 of 22 screened candidate GPIO used**. No direct GPIO remains for EKM014 four-channel request. Use **preassembled I2C GPIO expander** (e.g., MCP23017 0x20) only as low-speed intent signals into a qualified **separate hardware safety gate**, not directly onto powered FETs. Reset-state inputs and external pull-downs must be proven. If GPIO39–42/any other candidate are unavailable, **STOP** and remap, no overbooking.
- The expansion proposal saves MCU pins but does **NOT** solve hardware one-hot, non-retrigger pulse or watchdog. No second microcontroller, no custom PCBs, no Boat SD.
- **Do not** enable `Boat/source/boat_radio_hw_config.h` GPIO macros or set `*_PIN_AUDIT_SIGNED`; physical board pin map and compatible ready-made modules are not confirmed.

## Next objective evidence required
1. Request complete Albright contactor auxiliary/12V coil variant, dropout/pickup and DC induction-breaking data under measured BMS/motor stall current; check weather sealing and 3S undervoltage.
2. Request a real manufacturer-documented, factory assembled, **4-channel independent exclusive one-shot** or commission the complete qualified industrial configurable safety controller with verified state machine, feedback and level interfaces. No substitute from an ordinary timer without proof.
3. Obtain exact preassembled E22 SMD adapter or authorize a **matched pair** of fully verified compatible DIP radios; actual RF PHY+UART pairing remains untested.
4. Confirm 5V/3V3 load budgets, fuse placement, 10A EKM014 shared fuse vs measured 6A-class coil and 20A BMS vs two 550 stall currents.
5. Once modules arrive, independently test **power-on**, reset, unplugged GPIO, stuck-high, simultaneous requests, one-pulse only, contact weld, brownout, MCU freeze and current zero on dummy loads. No firmware output adapter or on-water operations beforehand.

**Version hold:** Remote VER028 / Boat VER014 unchanged; no hardware purchased/actuated, no sonar implementation.


### R67 superseding sensor/expander model choice
- Owner fixed **GY-91 MPU-9250 + BMP280** (1×) and exact **Waveshare MCP23017 IO Expansion Board SKU15391** (1×) on Boat; see [R67 selected hardware notes](BOAT_R67_GY91_MCP23017_SELECTION.md).
- R65 generic MCP23017 **0x20** was a research address, **NOT this board's factory default**: Waveshare 15391 has **0x27** by default; 0x20 requires physical jumper changes. Use [R67 I²C plan](BOAT_R67_I2C_AND_EXPANDER_MAP.csv) as authoritative until hardware commissioning.
- Four expander GPIOs are only *candidate release requests* to a separate independent qualified safety controller; never direct live EKM014 drive. Model is selected; no actual board or driver is commissioned.
