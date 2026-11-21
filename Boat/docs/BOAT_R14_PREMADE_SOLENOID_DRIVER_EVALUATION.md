# R14 module-first alternative — replace R9 custom four-coil PCB (CANDIDATE)

**Date:** 2026-11-21. **Decision status:** user asked whether ready-made NCD or equivalent modules can replace custom PCB development. Engineering recommendation is **Electrokit EKM014** as the preferred *commercial off-the-shelf candidate* for four coil power-switching channels. This is **not an authorized module purchase or final design freeze**; preserve R9 KiCad for fallback/reference and do not alter official VER028/VER014 releases.

## Candidate comparison using manufacturer/retailer descriptions

| Candidate | Control | Load rating | Flyback | Suitability |
| --- | --- | --- | --- | --- |
| NCD PR22-5 4-channel IoT FET, BUK98150-55A | MCP23008 I2C via NCD IoT socket | Listed 12VDC, 5.5A *per channel*. 8W is **MOSFET power/heat dissipation**, **NOT solenoid rated power**. | Integrated diodes + suppression capacitors | **Reject for 6A design allowance**; insufficient electrical margin, expensive additional I2C/adapter dependency; listed $163.95 and out of stock at review. |
| NCD PR22-4 I2C variant | MCP23008 I2C | Also listed 5.5A/channel at 12V | Integrated | Same 6A problem; do not substitute until actual coil current measured and qualified. |
| **Electrokit EKM014, article 41024394, 4-channel MOSFET board** | Four active-high digital inputs accepting 3.3V/5V logic; TI UCC27524 FET drivers | Supplier lists **up to 10A/channel**, 5–50VDC load supply (VH), **4.5–12VDC independent gate-driver supply (VL)**; VH must be >= VL | Schottky flyback diodes per channel | **Preferred candidate** for at most one 6A-class release at a time, subject to tests, one shared **10A slow-blow non-resettable VH fuse**, and missing independent safety functions. |
| Two Cytron MDD10A dual motor driver boards | PWM+DIR at 3.3V/5V | 10A/channel continuous, 5–30VDC | Motor-driver H bridges | Electrically a potential alternative but physically large (each 84.5×62mm) with extra reverse/manual test controls, not preferred for on/off solenoids. |

References: [NCD PR22-5](https://store.ncd.io/product/4-channel-solenoid-driver-valve-controller-8w-12v-fet-4-channel-gpio-with-iot-interface/), [NCD PR22-4](https://store.ncd.io/product/mcp23008-4-channel-8w-12v-fet-solenoid-driver-valve-controller-4-channel-gpio-with-i2c-interface/), [Electrokit product](https://www.electrokit.com/en/mosfet-drivkort-4-kan-50v-10a), [Electrokit EKM014 wiring manual](https://docs.electrokit.com/en/modules/EKM014/), [Cytron MDD10A](https://www.cytron.io/p-10amp-5v-30v-dc-motor-driver-2-channels).

**Mechanical discrepancy:** Electrokit product page prints dimensions 32×30mm but mounting holes 41×25mm, whereas EKM014 online manual lists mounting centers 27×25mm. These cannot all represent the same drawing. Obtain physical module drawing/vendor clarification and avoid asserting fit within existing 77×79mm carrier. All high-current MOSFET modules should be on removable harness, away from sensitive GNSS/RF, with their own mount as necessary.

## Proposed module-first wiring (physical implementation NOT YET VALIDATED)

```
3S4P BATTERY / BMS / MASTER SWITCH
    |---> independent 3S->5V supply ---> HELTEC approved 5V input
    |                                +--> EKM014 VL 5V gate-driver power (separately fused)
    |---> R13 ready-made RECOM 3.3V ---> independent hardware STOP/POR/wdg logic
    |---> independent primary nRF24 3V3 rail (RF bursts)
    '---> independent, fault-LATCHED physical high-side cutoff (not designed/qualified)
             |----> separate MDD20A propulsion motor supply (2x brushed 550)
             '---> independently protected COIL SUPPLY branch (VH, nominal 3S)
                                |
                         EKM014 VH (5-50V, shared 10A slow fuse)
                                |-- +LH / -LH ---> LH bait FW0630-009
                                |-- +RH / -RH ---> RH bait FW0630-009
                                |-- +LK / -LK ---> LK hook FW0630-001
                                '-- +RK / -RK ---> RK hook FW0630-001

Heltec SOL_REQ[4] ----> INDEPENDENT hardware one-hot, pulse limit, reset
                        (draft R10/R11/R12; not completed)
                        ----> EKM014 IN1/IN2/IN3/IN4 (3.3V HIGH)
          safety STOP ---hardware cutoff--> removes supply to both motor and coil rails
```

- **Do NOT connect Heltec GPIO directly to EKM014 inputs** without independently validated pulse limiter; the FET board doesn't provide maximum energized pulse time, fault latching or no-repeat.
- Do not power EKM014 **VL** directly from 3S battery: fully charged 3S **12.6V exceeds its 12V logic supply maximum**. Use a regulated, rated 5V separate supply with VH>=VL. Do not use R13's 3.3V safety rail for VL; it is below minimum 4.5V.
- Active HIGH IN1..IN4 mapping by label proposed LH/RH/LK/RK. Verify input voltage thresholds, pin location, reset pulldown and supplied header.
- The board's included **single 10A slow-blow VH fuse** is a shared supply fuse, **not four individually matched coil fuses**. Add proper external per-coil fusing/protection as required by coil current, harness ampacity and connector ratings; measure peak/warm coil current and BMS conditions. Enforce **exactly one active coil physically**, not only in MCU software.
- NCD or EKM014 flyback suppression may slow magnetic release; bench-test coil release times. 3S supply varies ~9–12.6V; verify FW0630-009 and FW0630-001 coil voltage and release thermal envelope. Select a regulated 12V coil converter if required, rated for measured pulse current.
- This approach can eliminate a **custom four-MOSFET driver PCB**, but cannot safely eliminate physical STOP/actuator cutoff, hardware pulse limiter, release token, current-zero feedback and POR. These need ready-made qualified safety/timer modules or dedicated isolated interlock board/harness solution; exact commercial parts NOT chosen. Software command lease/ACK logic alone is not hardware protection.
- Motors remain on **one separate Cytron MDD20A**. Do not change to MDD10A for propulsion or add rudder/RP2040. Sonar not selected/implemented.
- Pending measurements: coil resistances, current at min/nominal/max battery voltage, time-to-release, worst-case temperature, motor startup/stall and physical fuse/BMS. No actuator energization authorized until full independent physical acceptance.

## Revision protocol

This R14 analysis **supersedes R9 as the *preferred prospective power switching approach only***; the R9 native PCB schematic remains unmodified as an archived design/reference. No vendor module has been purchased, fitted or powered. Keep R10–R13 as unqualified engineering reference until a specific off-the-shelf physical interlock/supervisor solution is approved. Do not claim safety-system/ERC/PCB completion. Remote VER028, Boat VER014 remain held.
