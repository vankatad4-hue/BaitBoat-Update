# R26 — ADS1115 sequencing problem / INA226 voltage-only modular candidate

**2026-11-22 | DESIGN ALTERNATIVE — NOT SELECTED, NOT PURCHASED, NOT COMMISSIONED**

## Decision and physical boundary

- **Active R20 baseline remains ADS1115** (I²C 0x48) and the uncommissioned R21 150kΩ/33kΩ divider. Because the ADS1115 input may exceed VDD+0.3V when its supply is absent, the 3S voltage-sense path remains **physically DISCONNECTED** until a suitable manufacturer-compliant sense isolation/sequencing design exists.
- Investigated **TI INA226** as a **substitution candidate** for the *battery-voltage measurement module only*. TI states in its INA226 Rev C datasheet, §6.3.1, that monitored bus voltage may remain present while the INA226's VS supply is off. VBUS measures **0–36V**, independently of 2.7–5.5V chip supply; the register LSB is **1.25 mV**. This avoids the specific ADS1115 VDD-relative analog sensing issue *at the INA226 VBUS silicon pin*. This is not permission to ignore the wiring of the breakout, VS/I²C sequencing, transients or common grounds.
- The candidate **does not add a custom PCB** or alter R20 radio, GNSS, regulators, ESP32 pins, motor driver, 4-solenoid driver or battery. It would replace rather than supplement ADS1115, after explicit owner approval and inspection of an exact factory assembled board. No active purchase/BOM update has been made.
- An INA226 is a **measurement IC**, not an independent watchdog, low-voltage cutoff, safety relay or current-zero sensor. It does NOT resolve the physical STOP/watchdog/current-zero/pulse limiter gate. The R79 software RTH and critical STOP thresholds remain unchanged.

## Vendor evidence and reservations

TI primary source: [INA226 device and datasheet (Rev C, 2026)](https://www.ti.com/product/INA226), [full datasheet](https://www.ti.com/lit/ds/symlink/ina226.pdf) §§5.1, 6.3.1, 7.1.3, 7.1.9–10.

| Property | Verified chip-level value | Procurement / module-level limitation |
|---|---|---|
| Bus voltage | 0–36V measurement, VBUS absolute max 40V | Protect against surge/ESD and verify purchased breakout VBUS trace |
| Supply | VS 2.7–5.5V; existing 3.3V sensor regulator is candidate | Actual 3.3V rail ripple/rise-time and module pull-up domain remain unsigned |
| Power sequencing | TI explicitly supports bus present while VS is off | SCL absolute maximum is **VS+0.3V**; verify unpowered I²C does not back-drive SCL |
| I²C | 16 address options; candidate 0x40 if actual A0/A1 straps support it | Address must be measured; no 0x40 assumption from an arbitrary breakout |
| Identification | Manufacturer 0xFE = 0x5449; die 0xFF = 0x2260/0x2261 | IDs are diagnostics, not authentic-manufacturer proof |
| VBUS code | Register 0x02, 1.25 mV/LSB, positive-only | Do not confuse with ADS1115 0.125 mV/count; do not accept stale register without a new conversion |
| Current path | No series shunt needed for **VBUS-only** reading at the chip | Many manufactured boards tie VBUS to an IN− shunt terminal; inspect full schematic, never route 20A BMS current through a small preinstalled shunt |

**Sourcing leads, NOT purchases:** [TI INA226EVM through Comet/RS Bulgaria](https://store.comet.bg/en/CatalogueRS/Product/10585775/) (listed with **0 Comet BG stock** at review; request availability/price); [HESTORE INA226-M](https://www.hestore.eu/en/prod_10048984.html) (EU merchant listing, exact VBUS routing, board manufacturer and BG checkout unverified). Both are **candidates only**, not approved drop-in harnesses. No marketplace listings approved.

## Proposed voltage-only candidate harness — DO NOT CONNECT UNTIL BOARD APPROVED

| Proposed connection | Module-level condition |
|---|---|
| Existing D24V10F3 #2 3.3V sensor rail → INA226 VS | Only after exact breakout 3.3V supply/pull-up rail verification |
| Existing sensor GND → INA226 GND | Common low-current ground, no motor current through sensor harness |
| ESP GPIO7 (SCL), GPIO8 (SDA) → INA226 I²C | 3.3V idle pull-ups, verify SCL never > VS+0.3V during VS-off |
| Protected/switched 3S sensing branch → INA226 **VBUS** | Only low-current, suitably protected sense tap after inspecting board; **not** in motor or EKM power circuit |
| INA226 A0/A1 | Exact physical board wiring, address expected 0x40 is only a candidate |
| INA226 ALERT / current sense IN+/IN− | No approved connections; exact manufacturer's VBUS/shunt arrangement must be inspected. Do not use the shunt for 20A battery feed. |

No connector cavity numbering is approved here. R23 GPIO7/8 I²C remains the same. The R21 ADS1115 harness must **not** be repurposed or plugged into INA226 without the updated pinout being separately signed.

## R26 software artifacts — host only

- `Boat/source/ina226_voltage_candidate_r26.h` contains a **pure identity/voltage/freshness decoder** and an always-false `INA226_HARDWARE_SELECTED` lock. The guarded wrapper **always returns invalid** while the module is unselected. No `Wire` transactions, target I²C initialization, actuator access, firmware profile changes or status authority.
- `Boat/tests/ina226_candidate_voltage_r26_test.cpp` verifies both documented die IDs, manufacturer mismatch, big-endian 16-bit word decoding, voltage thresholds (12.60V=10080; 11.40V=9120; 10.80V=8640; 10.20V=8160; 9.90V=7920), failed/stale/new-conversion conditions, and compatibility with the existing BoatBatteryMonitor critical STOP policy. All inputs synthetic.
- The CI step compiles and executes this test with strict C++11 warnings. A passing CI run is **not** a board approval.
- The R26 physical worksheet remains all NOT_RUN. Existing R25 and R23 acceptance tables remain unchanged.

## R26 exact-module physical gates

1. Identify real manufacturer, PCB revision, connector markings, VBUS-to-IN− trace/shunt topology and measured I²C address. Reject modules without credible schematic or that require 20A through a small shunt.
2. Qualify low-current 3.3V VS and 3.3V I²C pull-ups, especially SCL when VS is absent; do not intentionally exceed absolute maximum limits.
3. Only with the specific board safely evaluated, apply a current-limited simulated 3S voltage to the **voltage-only sense path**, not 550 motors/EKM014 power.
4. Read registers FE, FF and 02, record new-conversion evidence, raw code, DMM voltage and identity, at 9.0, 9.9, 10.2, 10.8, 11.4, 12.0 and 12.6V, with both normal supply and safe supply-off scenarios.
5. Validate 1s bad-sample/stale fail-closed, R79 3s low-battery Home diversion eligibility and the critical STOP timing in dummy-load simulation. Independently confirm physical safety equipment BEFORE powering MDD20A/EKM014 high-current branches.
6. **No module switch** until owner confirms the exact part and the R26 physical records demonstrate suitable voltage-only wiring. Until then, R20 ADS1115 remains the selected-but-blocked design.

**Next development option:** choose a commercially assembled INA226 voltage module with independently accessible VBUS; separately select a qualified independent safety/power-interlock module. Do not silently switch the R20 BOM or remove the R25 ADC safety hold.
