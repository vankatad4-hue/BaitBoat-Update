# R68 — Boat 3.3 V and 5 V ready-made regulator selection proposal (updated by R68A)

**Date:** 2026-11-22 | **Status:** ENGINEERING SHORTLIST, NOT OWNER CONFIRMED / NOT PURCHASED / NOT COMMISSIONED.
**R68A nRF power supersession:** The owner proposed the **Ardboard 1127 socket adapter**, which has its own AMS1117-3.3. See [R68A source audit](BOAT_R68A_ARDBOARD_NRF_SOCKET_POWER_REVIEW.md). The adapter will be evaluated from a filtered *regulated 5 V spur*, not 3S battery or 3.3 V rail. Pololu D24V10F3 is now proposed for **E22 backup alone**, conditional on the SMD carrier. The Ardboard regulator's input specification and assembled thermal/ripple capability are **NOT manufacturer-verified**; not approved to energize a PA/LNA module yet.

**R71 active proposal supersedes R68 sensor supply row:** The user proposed and engineering accepted **2× Pololu D24V10F3 #2830**, with **separate isolated-from-each-other 3.3 V outputs**: one for E22 and one for **GY-91/QMC5883L/Waveshare MCP23017 SKU15391**. The prior `RECOM R-78E3.3-0.5` sensor rail in the historical R68 table and R68 diagram below is **NO LONGER the preferred proposal**; it remains an unselected alternative. Authoritative updated active BOM: [R68/R71 rail BOM](BOAT_R68_POWER_RAIL_BOM.csv) and [R71 compatibility/qualification](BOAT_R71_DUAL_D24V10F3_3V3_RAIL_REVIEW.md). Neither Pololu module is yet finally owner-locked, purchased or physically powered. 3.3 V is not galvanic isolation: both branches share a suitable common logic GND. **Never join their VOUT pins.**

No custom electronic PCB designs. All four converters must be commercial, preassembled modules with verified detachable harness/approved header adapters. **No actuator power or GPIO output enable authorized.**

## Upstream assumptions

Boat pack: 3S4P Li-ion, nominal 11.1 V, fully charged ~12.6 V, nominal low operating endpoint studied at 9.0 V but actual BMS cutoff and motor-load sag must be measured. Battery is not a regulated 12 V power source. Regulator VIN rating must include measured inductive/battery transients as well as DC input and qualified branch fusing/reverse-polarity protection. Keep supply rails physically separate from high current motor/coil returns and from each other on the output side; share only documented low-current reference at an appropriate star point. Do **not** parallel regulator outputs.

## Recommended four rail architecture — exact product proposals, NOT purchases

| Proposed role | Manufacturer assembled device and MPN | Spec manufacturer | Loads / limitations |
|---|---|---|---|
| `BOAT_5V_MCU` | **Olimex DCDC6-16-TO5** ×1 | [Olimex](https://www.olimex.com/Products/Power-Supply/Modules/DCDC6-16-TO5/), 6–16 V input, regulated 5 V / 2 A; BG manufacturer page listed in stock at check (not ordered) | Power **Espressif ESP32-S3-DevKitC-1-N32R16V** via one manufacturer-supported USB or **5V/GND entry**. Hardware revision/USB cable current/voltage-drop and programming setup must be verified. Olimex offers **barrel jack VIN, USB-A 5V OUT**, not screw-terminal outputs; no invented terminal pinout |
| `BOAT_5V_GATE` | **Pololu S13V20F5 #4085** ×1 | [Pololu specs](https://www.pololu.com/product/4085/specs), 2.8–22 V input, regulated 5 V / **nominal 2 A**, buck-boost; current derates with input/temperature | Exclusively **Electrokit EKM014 VL (4.5–12 V)** *logic/driver supply*, optionally monitored branch if separately qualified. **Not** its VH battery load feed. No reverse input-polarity protection on this device; supply through independently fused/protected upstream harness, confirm headers/sockets and independent limiter |
| `BOAT_3V3_E22` | **Pololu D24V10F3 #2830** ×1 | [Pololu specs](https://www.pololu.com/product/2830), ~3.4–36 V input, fixed 3.3 V / **1 A rated**, buck with power-good indicator, reverse-polarity protection **not built in** | Secondary matching **E22-900T22S backup LoRa ONLY** if its specific UART carrier accepts 3.3 V VCC and **3.3 V TTL levels**. The primary **nRF24 PA/LNA uses the separately proposed Ardboard article 1127 AMS1117-3.3 socket fed from a filtered regulated 5 V spur** (see R68A). Measure E22 transmit peaks, startup and PA noise; provide separate radio branches and local decoupling, ferrite/filtering if needed. Do not assert RF reliability or safety solely from 1 A nameplate. Keep independent 5V-fed nRF adapter and E22 3.3V regulator; if tested headroom or interference fails use a separate approved 5V feed for Ardboard, never the sensor rail |
| `BOAT_3V3_SENSORS` | **RECOM R-78E3.3-0.5** ×1 | [RECOM manufacturer](https://recom-power.com/en/rec-s-R-78E-0.5.html), 6–28 V VIN, 3.3 V / 500 mA; [Farnell Bulgaria](https://bg.farnell.com/c/power-supplies/dc-dc-converters/linear-regulator-replacements?brand=recom-power) code 2218602 | Selected **GY-91 MPU-9250+BMP280**, **Waveshare MCP23017 IO Expansion Board SKU15391** at factory I²C `0x27`, verified compass I²C/pullups, and low-current logic/measurement only. Installed through genuine preassembled SIP3 terminal/header adapter; note VIN pin1, GND pin2, VOUT pin3, optional manufacturer-rated decoupling and min-load/ripple. **500 mA is NOT suitable for the ESP32 or high-power radios**, nor a fully validated independent safety PLC. No onboard PG; it cannot replace physical POR/brownout and power-cutoff chain |

**Why D24V10F3 instead of prior D24V22F3?** 3.3 V at nominal 1 A is a candidate dedicated **E22 backup rail** now that the Ardboard adapter offers local nRF regulation. The adapter must still be validated from a genuine regulated 5 V supply. The first RF rail was originally designed for both radios, but this is conditional on measured combined peaks and RF brownout behavior. Previous D24V22F3 #2857 is still a technically appropriate 3.3 V, 2.6 A-capable fallback but was **manufacturer-labelled 'rationed'** at review and listed at ~$26.95 in the US. #2830 is listed active/preferred manufacturer SKU ~$13.95; Pololu stock, Bulgaria delivery and fitted harness remain unverified. [D24V22F3 specs](https://www.pololu.com/product/2857/specs), [D24V10F3 specs](https://www.pololu.com/product/2830/specs). This is a **conditional design refinement, not owner authorization to substitute parts.**

## Functional rail topology (no commissioning wiring permit)

```text
  3S PACK + BMS
     |
     +-- protected/filtered low-current 3S bus
     |     |
     |     +-- Olimex DCDC6-16-TO5 -> regulated 5 V -> ESP32-S3 DevKitC
     |     |                               |
     |     |                               +-- filtered fused 5 V spur -> Ardboard 1127
     |     |                                                      AMS1117 3.3 V -> nRF24 PA/LNA
     |     +-- Pololu S13V20F5 -> 5 V EKM014 VL logic ONLY
     |     +-- Pololu D24V10F3 -> 3.3 V E22 UART ONLY (carrier-dependent)
     |     +-- RECOM R-78E3.3-0.5 -> 3.3 V GY-91 / compass / MCP23017
     |
     +-- independently qualified DC load cutoff -> MDD20A motors
     +-- independently qualified DC load cutoff -> EKM014 VH coils

  Ardboard adapter is NOT a direct 3S-to-3.3 V regulator.
  All load cutoffs, fuses, current feedback and interlocks remain unapproved.
```

**No cross-feed**: do NOT use an external 3.3 V regulator to inject into ESP32 3V3 when MCU is already powered from 5 V/USB, and do not backfeed ESP32 GPIO via I²C pullups if sensor 3.3 V is present while MCU is off. The exact board has **three mutually exclusive supply options**: USB, header 5V/GND or header 3V3/GND ([Espressif official DevKitC-1 guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html)). Use exactly **one 5 V injection path** at a time; when external header 5V is fitted, avoid concurrent USB power unless a verified prebuilt isolating accessory is used. USB power can be chosen through appropriate cable; do not assert the attached connector style until board revision is inspected. Never power full 3S directly into ESP32 5 V, GY-91, MCP23017 or 3.3 V radios.

**EKM014 hardware detail:** [Electrokit OEM documentation](https://docs.electrokit.com/en/modules/EKM014/) gives VL 4.5–12 V and VH 5–50 V; **VH must be >= VL**, and 5 V VL + independently switched raw pack VH is compatible over expected 3S range. EKM014 VL being powered **does not** prove its FETs are off: 12 V battery supply to VH must be physically independently cutoff. Do not bridge VL/VH by accident. EKM014 driver power isn't an independently rated four-channel pulse interlock.

**E22:** Manufacturer's exact `E22-900T22S` supply 2.3–5.5 V typical family, 3.3 V nominal planned. Verify actual carrier VCC and UART level-shifting; do not connect if a breakout has a different required input. The active matched SMD unit is still without an approved premade carrier. 3.3 V **RF power branch must not be conflated with safety logic**.

## Provisional rail-current budgets, not hardware measurements

- `5V_MCU`: **design allowance** peak 1.2 A / nominal unspecified, module rated 2 A; measure Wi-Fi disabled/boot/RF/peripherals and exact DevKit 5 V input as populated.
- `5V_GATE`: **design allowance** peak 0.5 A / nominal unspecified, selected module nominal max 2 A; EKM014 VL actual current and radio/noise coupling to be measured.
- `3V3_E22`: **design allowance** to be established from the matched E22 carrier, not both radios. Pololu 1A rail proposed solely for E22. Exact carrier is missing; do not energize.
- `3V3_NRF_LOCAL`: nRF is powered by the **Ardboard 1127 AMS1117-3.3 socket** from a filtered *regulated 5V* spur of Olimex after full load/thermal qualification. Ardboard PA/LNA module SKU 1037 lists 115 mA TX only as a typical product specification; it does **not** establish an adapter continuous rating.
- `3V3_SENSORS`: **design allowance** peak 0.25 A / actual unknown; converter 0.5 A. MCP23017 current includes GPIO output current into independently powered prebuilt safety interface. GPIO requests must not directly power 24 V logic/coils.
- The budgets are conservative **planning limits only** (not estimated actual nominal current and not purchase validation). They must be replaced by a measured spreadsheet at 9.0, 11.1, 12.6 V, peak current, boot, all-low current and hot-enclosure test. Need input fuses and protected connectors selected from real currents and cable ampacity, not these provisional allowances.

## Procurement checks / test gates

1. Check exact 5 V/3.3 V module maker SKUs and vendor stock with trusted BG or EU authorized stores: **Olimex manufacturer/BG**, **Comet**, **Farnell/DigiKey BG**, **Pololu manufacturer and Pololu verified distributors**. Official listings ≠ reserved inventory; Pololu outlets often marketplace-shipped directly from manufacturer.
2. Confirm all low-voltage power outputs in isolation over 9–12.6 V and on measured transients, with hot/cold load efficiency and ripple/noise on scope (especially nRF adapter and E22 simultaneous TX).
3. Verify connector/USB cable/harness strain relief, fuses, reverse battery polarity, anti-backfeed, 3.3V I²C high-level logic, E22 carrier supply, start/reset and brownout behavior.
4. Confirm GY-91 0x68/0x69 and BMP280 0x76/0x77, QMC5883L actual address and Waveshare MCP23017 factory `0x27`; all low-voltage measurements are **read-only** while actuator branches physically disconnected.
5. Verify independent safety stack (STOP, DC contactors, watchdog, current-zero, pulse control) separately before wiring EKM014 IN/VH or motor output; NONE is approved today.
6. **No custom PCB manufacture; no live GPIO/firmware change; no release/ZIP marker bump.** Boat VER014 and Remote VER028 remain frozen.

**Owner decision still required for exact regulator model lock:** the four modules here are an R68 engineering recommendation, **not** an owner-confirmed purchase selection.

**R68A final note:** R68A changes the **nRF regulator location**, not the quantity of independent upstream 3S-to-regulated power converters. The matched E22 still uses the separately proposed Pololu D24V10F3 pending E22 SMD carrier. **The specific Ardboard adapter is owner-proposed for investigation, not confirmed by an electrical acceptance test.**
