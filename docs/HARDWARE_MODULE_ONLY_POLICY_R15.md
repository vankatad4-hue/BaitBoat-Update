# Flytec V080 R15 — MODULE-ONLY ELECTRONICS POLICY
**Effective:** 2026-11-21 | **Branch:** `hardware-ready-dual-radio-r1` | **Status:** Architecture decision adopted, NOT hardware commissioned

## Binding engineering rule
**NO custom electronic PCB design, no bespoke carrier PCB, no custom power/safety board, no user-fabricated MOSFET breakout.** Both Boat and Remote electronics are assembled only from commercially manufactured, independently replaceable modules and unmodified certified/rated mechanical electrical accessories: wiring looms, modular plug-in connectors, premade distribution/fuse blocks, inline wire fuses, mechanical switch assemblies, premade screw-terminal breakout boards, and commercially manufactured weatherproof enclosures. Ready-made electronic development/breakout modules are allowed. Hand-soldering wires to already manufactured module pads/headers or fitting prefabricated adapters is allowed with strain relief; drawing, etching, ordering or fabricating a new circuit board is not.

Custom **non-electronic** 3D-printed remote shell/grips, brackets, gaskets and cable guides are still allowed. Keep high-current paths physically separated from RF/GNSS, I2C and other low voltage logic. Each module must be replaceable without reworking another module.

**All existing R7/R9/R10/R11/R12/R13 KiCad schematics and Remote PCB design studies are ARCHIVED REFERENCE ONLY**. They are NOT active manufacturing plans, and their partial ERC/netlist success is NOT an acceptance gate for the module build. Archive files remain available for engineering history/reuse of interface intentions; new KiCad custom PCB work is explicitly removed from the active roadmap.

## Freeze of previously chosen functions
- **Boat:** Heltec Wireless Tracker **v1.2** controller/GNSS, two independent Cytron **MDD20A** brushed 550 propulsion channels, 3S4P pack/BMS 20A nominal (ratings pending verification); nRF24L01+ PA/LNA primary radio; Boat integrated SX1262 backup **DISABLED** until Remote E22 link compatibility is verified; no RP2040, no rudder, no separate autopilot ECU. Four nominal 12V bait/hook solenoids LH/RH/LK/RK, each planned as **up to 6A until measured**.
- **Remote:** Waveshare **ESP32-S3-Touch-LCD-4.3 Standard**, LVGL 9.6.0, 16 keys (including OK) arranged 2x4 on each side, two 2-axis joysticks, touchscreen/SD, 1S4P pack/BMS, separate nRF PA/LNA and E22 868MHz (remote) modules, dual antenna exits; power button above display.
- **Sonar:** TBD CHIRP module, **planning ONLY**; no sonar code, connector, or purchase authorized.
- **Software security:** Key provisioning, hardware pins and commissioning remain disabled/fail-closed; Remote VER028 and Boat VER014 official markers unchanged. Never claim hardware ready due to host tests or drawings.

## Active module shortlist; procurement authorization IS NOT granted
- **Boat coil driver:** Electrokit **EKM014** prebuilt 4-channel MOSFET board, supplier claims 10A per channel and 5–50V load supply, 4.5–12V driver supply VL; uses 3.3V/5V logic inputs. One 10A slow-blow fuse is **shared** at VH; required protection per coil, active-high default-off and wiring are unverified. NCD 4ch board 5.5A/channel is under provisional 6A requirement, therefore NOT preferred. **EKM014 is a candidate, not qualified.**
- **Boat propulsion driver:** separately mounted premade Cytron MDD20A; 88.9 x 78.74 mm cannot be placed in legacy 77 x 79 mm low-voltage carrier envelope; choose alternate mounting before buying enclosure parts.
- **Boat safety logic 3.3V:** preassembled **RECOM R-78E3.3-0.5**, 6–28V input / fixed 3.3V 0.5A; only a manufactured terminal adapter, not a bespoke PCB. Critical: no PG pin; independently supervised.
- **Boat controller 5V and EKM014 VL 5V:** separate off-the-shelf **Pololu S13V20F5** (5V 2A 2.8–22V) modules CANDIDATES, one per clean power island; exact power-injection pins on Heltec unverified. Never connect raw charged 3S 12.6V to EKM014 VL (max 12V), and never apply 3S to Heltec single-cell charger connector.
- **Boat nRF PA/LNA 3.3V:** premade **Pololu D24V22F3** (3.3V 2.6A 4–36V) buck candidate, physically separate from safety logic and antenna RF clearances.
- **Remote buttons:** prebuilt Adafruit **MCP23017 STEMMA QT breakout**, 16 inputs at 3.3V, I2C address proposal 0x20; independent each-key default passive safe wiring.
- **Remote joystick ADC:** **TWO** premade Adafruit ADS1115 4-input breakouts: proposed 0x49 for 4 joystick axes, proposed 0x4A for battery/NTC/radio diagnostics. **Do not use 0x48**: Remote SC16IS750 serial bridge is reserved at 0x48. Address strapping with actual device/header must be verified. ADS1115 single-ended max input must not exceed supply; battery requires proper off-the-shelf divider or a ready-made higher-voltage monitor, not direct pack input.
- **Remote nRF rail:** replace earlier bare TPS63070 custom buck/boost PCB with premade **Pololu S9V11F3S5** 3.3V/1.5A, VIN 2–16V candidate; do not select the legacy S7V8F3 for the Boat 3S rail (max 11.8V input < full 3S).
- **Remote E22 5V rail:** replace earlier custom TPS61023 boost PCB with premade **Pololu S13V20F5** 5V/2A from protected 1S pack candidate. Do not parallel the Waveshare onboard CS8501 5V.
- **Remote controller/display 5V:** Waveshare's integrated CS8501 preassembled battery supply, no custom replacement. 1S4P BMS/charge system to be validated by manufacturer instructions before battery hookup.
- **Boat independent STOP/fault control, power-on reset, non-retriggerable pulse interlock, motor/coil current-zero, high-current DC disconnect:** **NOT YET SELECTED AS A MATCHED SET**. The R10–R12 discrete-chip schematics are retired and NOT permitted for fabrication. Consider ready-made contactor/latching safety controller/timer and current monitors only after verified DC ratings, startup fail-OFF, remote power-loss behavior, reset and timer bounds, compatibility with 3S and physical fit. **Never substitute a plain FET switch, generic autorepeating relay timer, or MCU software only for the independent safety function.**

## Module acceptance contract
Before a module becomes selected/frozen: record source URL + exact stocked part number; supplier pinout revision; VIN/VOUT peaks; continuous and pulse current derating; grounded/transient behavior; board dimensions and connector pin pitch; power and fault state on startup, during watchdog failure, after brownout, stuck HIGH; availability/lead time to Bulgaria; RF/regional compliance; serviceability/waterproofing.

**No vendor module or actuator may be powered as an integrated load** until the independent hardware cutoff, pulse limiter and one-hot behavior are physically qualified. Software simulation and module nameplates are not evidence of real current/thermal suitability.

## Required next work (not custom schematic)
1. Prove actual 12V coil resistance/peak-current/time and both 550 motor stall/start peaks, including 3S minimum/nominal/full and BMS tests.
2. Buy/sample exact EKM014 only after current and fit gate; independently bench-test a single unloaded MOSFET output with a current-limited supply before using dummy loads.
3. Find manufacturer-documented **premade** independent safe-OFF current-rated DC contactor/interlock and non-retriggerable pulse relay modules with manual rearm and brownout defaults; check space and 3S voltage; no specific replacement is yet safety-approved.
4. Update physical wiring/harness matrix; fuse/wire/connector and water ingress review, using no PCB artwork/Gerber.
5. Map actual Heltec v1.2 and Waveshare GPIO/power/bus pins; keep all actuator output GPIO disabled until verified; test new module-only adapter software using simulated hardware.
6. Physically validate commissioning; only then consider Remote VER029 and Boat VER015. Defer sonar selection/integration.
