# R70 — Ardboard product 1577 XL6019 automatic buck-boost module evaluation

**Date:** 2026-11-22 | **Status:** EVALUATED_OPTION_NOT_SELECTED; NO OWNER REGULATOR LOCK; NOT PURCHASED / NOT COMMISSIONED.

## Exact product, not an arbitrary boost-only board
Supplier: [Ardboard Bulgaria XL6019 Automatic Step-Up Step-Down Converter](https://www.ardboard.com/xl6019-automatic-step-up-step-down-converter), catalogue code **1577**, observed **€4.90 including VAT** and **7 listed in stock** at lookup 2026-11-22; price/stock are not verified checkout.
Merchant claims: automatic buck-boost, 3.8–32V DC input, adjustable 1.25–35V output, max 3A input, ~1.5–2A output dependent on delta-V, efficiency up to 94%, 400kHz switch, claimed ripple <50mV, 53×26×13 mm, cooling required above 1.5A/15W; **NO reverse-polarity input protection**. Input/output pads need soldered leads: in line with module-only policy, obtain factory-prepared locking connector harness/approved assembly, never improvise an exposed wiring arrangement.

**Critical inconsistency:** [actual XLSEMI XL6019 datasheet Rev 1.5](https://www.xlsemi.com/datasheet/XL6019-EN.pdf) specifies **5–40V chip input, 180kHz nominal fixed oscillator (144–253kHz variation)** and 5A **internal switch current LIMIT** (not 5A output rating). The generic chip supports SEPIC/buck-boost topology only with appropriate external inductors/components. Ardboard says 3.8–32V and 400kHz for a module purportedly containing XL6019. The vendor's assembled board component list, schematic, switching frequency and low-input capability are not documented; **cannot assert its claims as manufacturer-qualified**. The discrepancy is a vendor clarification block before purchase. Both published voltage-input bands encompass boat 3S ~9–12.6V under normal planned operation, but neither gives guaranteed brownout/overvoltage performance on the actual board.

## Compatibility with selected Boat power architecture

| Branch | Evaluation | Design decision |
|---|---|---|
| 3S -> 5V MCU | possible by output range; **not preferred** for selected ESP32-S3-DevKitC-1-N32R16V because 5V/2A is advertised as conditional and adjustable trimpot can drift/fail | Retain **Olimex DCDC6-16-TO5**, R68 proposed fixed 5V/2A, and prevent USB/5V pin simultaneous feed |
| 3S -> 5V EKM014 VL logic | **best non-safety experiment**: 5V @ low measured VL draw (<1.5A nominal) plausibly fits; must be separately fused, protected, preset unloaded and loaded, lock trimpot and measure output ripple/transients | Retain **Pololu S13V20F5** as preferred 5V VL converter until bench qualification. EKM014 **VH** remains separate safety-rated battery cutoff, never tied to 5V converter |
| 3S -> 5V Ardboard 1127 -> nRF PA/LNA | possible if isolated filtered 5V branch with verified combined budget and no backfeed; adjustable supply would add ripple before local AMS1117 | Prefer Olimex fixed 5V, as in R68A; no substitute without RF soak and transient tests |
| 3S -> 3.3V GY-91/QMC/MCP23017 | output adjustable but not preferred for sensitive 3.3V I2C silicon; output overshoot can damage 3.3V chips | Retain fixed **RECOM R-78E3.3-0.5** |
| 3S -> 3.3V E22-900T22S | current could fit but adjustable output and RF EMC risk; E22 factory-made SMD carrier unselected | Retain fixed **Pololu D24V10F3** candidate |
| 3S -> 24V industrial safety PLC | possible power stage in principle but **NOT approved** as an independent safety controller/STOP-supply device given unknown startup/EMC/thermal/fault mode and 5V output max current claims not enough to prove 24V power budget | Retain manufacturer-documented **MEAN WELL DDR-30G-24** as R66 research candidate only, pending actual PLC system |
| Remote 1S4P raw battery (3.0–4.2V) -> 5V | **not suitable** under XLSEMI 5V chip minimum and even merchant 3.8V module claim cannot cover full discharge | Do not apply this module to Remote pack |

**No buck-boost required on the Boat for 3S -> 5V/3.3V** if power policy requires controlled shutdown before 3S drops below those outputs; a fixed-output buck is simpler with fewer topology uncertainties. Even if 3S falls below 5V, this should be a critical STOP/brownout event, not an argument to keep actuators energized.

## Exact bench gates before considering a module substitution

1. Manufacturer/vendor response: exact actual IC marking, assembled board circuit/SEPIC topology, exact minimum VIN, frequency discrepancy 180kHz vs listed 400kHz, 5V/2A continuous derating, 5V/3.3V thermal test and no-load/ramp ripple, fuse/OVP and terminal polarity.
2. Use current-limited power supply at **9.0/11.1/12.6V**; adjust output with **no sensitive electronics attached**, measure precise 5.0V loaded and unloaded; test load at **0.1, 0.5, 1.0, 1.5A** (and 2A only if vendor provides actual allowable rating plus heat management).
3. Scope startup and shutdown/output overshoot (must not exceed connected 5V or 3.3V absolute limits), load-step 0↔measured EKM014 VL draw, ripple at the intended physical connector, current, hot enclosure operation.
4. Verify genuine reverse-polarity protected/branch-fused 3S wiring, BMS sag and independent physical STOP. Potentiometer position is **not** considered a reliable safety mechanism; mechanical/adjustment stability to be addressed.
5. Confirm radio and GNSS EMC if ever used near them. Probe EKM014 IN1..4 outputs for zero-power safe default, **with VH and motor branch physically disconnected**.
6. Recheck Ardboard stock/delivery and seek preassembled connector harness; its pads need a manufacturer-appropriate terminated connection rather than exposed/loose Dupont wires.
7. Do not change live GPIO, enable motors or solenoids, or substitute current purchase BOM without explicit owner selection and validated acceptance results.

**Conclusion:** Yes, exact module can step down in merchant-described buck-boost construction, so reject previous generic XL6019 boost-only blanket claim. However actual board datasheet/spec discrepancy and adjustable construction mean **conditional evaluation for EKM014 VL 5V only**; not selected for MCU, RF, sensors, E22 or safety-critical PLC. Core Boat R68 5V and 3.3V regulator proposal remains intact. **Boat VER014 / Remote VER028 frozen**.
