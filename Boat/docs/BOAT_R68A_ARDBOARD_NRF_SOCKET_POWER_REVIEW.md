# R68A — Ardboard nRF24L01 socket adapter (article 1127) power audit

**2026-11-22 | Status: OWNER-PROPOSED ADAPTER; research-screened only, NOT electrical acceptance, NOT PURCHASED.** Scope: the Boat's nRF24L01+ PA/LNA link. The Remote nRF24 power arrangement is a *separate* task (Remote uses 1S4P, cannot apply raw battery to the 5V-input adapter).

## Exact Bulgarian listing and manufacturer evidence

- Ardboard [nRF24L01 socket adapter](https://www.ardboard.com/nrf24l01-socket-adapter): **Ardboard article 1127**, listed product/model “24L01 board,” **€1.99 incl. VAT**, **24 available** when checked on 2026-11-22 (live quantities/prices can change). Listed dimensions **26 × 18 × 10 mm**, weight 3.8 g; manufacturer field says **China**, not a traceable AMS/Pololu/OEM assembled rating.
- The page explicitly specifies an **onboard AMS1117-3.3 linear regulator**, 8-pin nRF24 socket, host signal header, and separate **VCC/GND input**. It **does not state** the module's exact *input-voltage range*, regulator continuous current rating, capacitance values, regulator manufacturer/thermal PCB design, proof that long-range PA/LNA variant fits, or any 5 V logic-level translation. Verify those physically with the vendor before purchase approval.
- Ardboard also sells [long-range nRF24 module, article 1037](https://www.ardboard.com/nRF24L01-long-range-tranceiver), listed **3.6 V maximum radio supply**, **115 mA TX current**, 8-pin, 48 × 15 × 18 mm with antenna and 14 available/€4.55 incl VAT when checked. These are **vendor listing claims for that article** only; they are **not measured worst-case peaks for all long-range nRF24L01+ PA/LNA variants**.
- [AMS1117-3.3 manufacturer datasheet](https://www.alldatasheet.com/html-pdf/205691/ADMOS/AMS1117-3.3/474/3/AMS1117-3.3.html) states dropout about **1.1 V typical / 1.3 V maximum at rated high current** (variant and load-dependent); **do not power its VCC from 3.3 V** and expect regulated 3.3 V. Nominal **5 V** upstream is the normal design case *subject to actual Ardboard board input rating and connector pinout verification*. **Do not feed 3S pack voltage directly**: AMS1117 thermally dissipates all (VIN−3.3 V)×I and vendor has published no adapter input-vs-thermal capability.

## Electrical conclusion and actual architecture impact

**Accepted as a candidate dedicated nRF24 local 3.3 V regulator**, but not a qualified independent 3S-to-3.3 V supply. It needs a **regulated and separately protected/filtered 5 V upstream branch**. The proposed **Olimex DCDC6-16-TO5 5V/2A** (already proposed for selected ESP32-S3-DevKitC-1-N32R16V) could feed a second local nRF power branch **after** total peak current, cable/connector/ripple and ESP32 startup margins pass. If 2A headroom/EMC are inadequate, allocate a separate ready-made **5V** regulator solely for the adapter, not an extra 3.3 V LDO in series.

```text
3S4P PACK + BMS
   |
   +-- separately fused/qualified 3S -> 5V converter (Olimex candidate) -> MCU ESP32-S3
   |                                      |
   |                                      +-- filtered short 5V branch -> Ardboard 1127
   |                                                                     AMS1117-3.3 -> nRF24 PA/LNA
   |
   +-- separately fused 3S -> Pololu S13V20F5 5V -> EKM014 VL only
   +-- separately fused 3S -> RECOM R-78E3.3-0.5 -> GY-91, QMC, Waveshare MCP23017
   +-- separately fused 3S -> Pololu D24V10F3 3.3V -> E22 UART **only** (conditional compatible carrier)
   |
   +-- independently verified safety cutoff branches -> MDD20A motors and EKM014 VH
```

**Important R68 correction:** the previously proposed **Pololu D24V10F3 is no longer the nRF supply** if the Ardboard adapter passes testing. It is now an **E22-only candidate**, *pending exact E22 preassembled carrier VIN/IO contract*. This does NOT eliminate the need for an external 3S→5V regulator; the Ardboard adapter only converts the regulated 5 V to its local nRF 3.3 V. It does NOT feed the E22 or ESP32; do NOT parallel any 3.3 V regulator outputs.

## Quantitative thermal sanity check (NOT a rating of the assembled adapter)

Assuming 5.0 V actually at adapter VCC, its linear-regulator dissipation is `P_loss ≈ (5.0−3.3)V × I_radio`, plus small quiescent consumption:
- Ardboard long-range article 1037 **listed** 115mA TX: `P_loss ≈ 0.196 W`.
- Hypothetical **250mA** worst-case variant/peak: `P_loss ≈ 0.425 W`.
- Hypothetical **500mA** sustained: `P_loss ≈ 0.850 W`.

Nominal AMS1117 chip datasheet current or thermal shutdown is **NOT** a legitimate guaranteed rating for this tiny 26×18mm assembled socket. Thermal resistance depends on copper area, airflow and water-sealed housing; no board thermal data is published. Verify regulator package marking, input/output capacitor populated values/ESR, measured 3.3 V transient minimum/maximum on the actual PA/LNA transmission burst, actual temperature after long transmit/receive cycles and continuous-current requirement.

## Signal and interface cautions

- ESP32-S3 SPI/CE/CSN remain **3.3V logic** from the existing fail-closed pin proposal. Ardboard provides a host connection header, **not a published schematic verifying level shifting or pin numbering**; don't assume it protects a 5 V controller from overvoltage or that every PA/LNA clone has same pin order.
- Adapter AMS1117 dropout means upstream 5V rail at the adapter must stay above output voltage + applicable dropout, allowing wiring sag. At 3.3V input it cannot deliver a regulated 3.3V output. Keep 3.3 V radio local decoupling near socket and grounded return isolated from motor/coil current loops.
- On power-off or partial-rail failure, ensure SPI/CE/CSN do not backpower the nRF through protection diodes. Software should hold CE low and CSN high if safe; neither firmware state nor adapter capacitor is independent STOP hardware.
- Backup **E22-900T22S SMD** still needs a verified factory-made carrier and independent supply/voltage-level contract. Do not treat a 5V socket adapter for nRF as an E22 regulator. RF/stop backup safety behavior unchanged.

## Purchase approval and bench validation

1. Confirm with Ardboard actual **1127 adapter VIN allowed min/max, output guaranteed at 5 V, local capacitors and fully assembled 8-pin socket and host pin order**; whether their **PA/LNA article 1037** is physically compatible.
2. With a current-limited regulated **5.0 V bench supply**, *radio only, all actuators disconnected*, measure 3.3V at socket at idle and during TX/RX, burst droop, output ripple, PA/LNA peak/average input current and regulator temperature after soak, also at worst-case hot enclosure.
3. Confirm ESP32 + adapter combined 5V demand versus Olimex rated **2 A**, including cable resistance and startup; ensure independent fusing/filtered spur, no ESP32 USB/header dual feed and no I/O backfeed during unpowered states.
4. If failure: retain external genuine 3S→3.3V radio rail (Pololu D24V10F3 or higher-capacity approved) with correct rated cable/socket scheme; do not power nRF through undervolted AMS1117 from 3.3V.
5. This module is **suitable for evaluation**, not yet owner-confirmed exact-part lock or commissioning. No firmware pin authorization; no physical motor/coil enable, no sonar integration, no operational version increment. **Boat 014 / Remote 028 held.**

## Source snapshots

- [Ardboard adapter article 1127](https://www.ardboard.com/nrf24l01-socket-adapter) and [Ardboard long-range article 1037](https://www.ardboard.com/nRF24L01-long-range-tranceiver) consulted 2026-11-22.
- [AMS1117 electrical characteristics](https://www.alldatasheet.com/html-pdf/205691/ADMOS/AMS1117-3.3/474/3/AMS1117-3.3.html), generic regulator electrical data, **not** the assembled adapter's guaranteed datasheet.

## R73 5 V capacity change
The shared provisional **Olimex 5 V 2 A** supply now also includes the **HGLRC M100 PRO GPS with its onboard QMC5883L** (complete assembly rated **3.6–5.5 V**). nRF adapter/ESP32/whole GPS aggregate startup draw, 5V cable drop, GNSS RF noise and I²C/UART high-level safety must be qualified before a single 5 V converter is accepted. The previous 3.3 V sensor-bus reference to `QMC` does not indicate separate QMC hardware. R73 notes: [R73](BOAT_R73_M100_PRO_INTEGRATED_COMPASS_POWER.md).
