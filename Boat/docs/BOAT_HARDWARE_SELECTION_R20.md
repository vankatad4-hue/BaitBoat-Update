# Flytec V080 Boat hardware selection — R20 baseline (R27 battery-sense successor)

Date: 2026-11-22  
Branch: `hardware-ready-dual-radio-r1`  
Status: active R20 module baseline, except **R27 replaces ADS1115 with Ardboard INA231 product 1491 for voltage-only sensing**. No parts purchased/commissioned. See [R27 manufacturer evidence and exact module wiring](BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md).

## Active selected modules

| Function | Part | Status |
|---|---|---|
| Main controller | ESP32-S3-DevKitC-1-N32R16V | Selected |
| GNSS | HGLRC M100 PRO | Selected |
| Compass | QMC5883L integrated with M100 PRO | Selected |
| IMU/barometer | GY-91 (MPU-9250 + BMP280) | Selected |
| Primary radio | nRF24L01+ PA/LNA | Selected |
| Backup radio | EBYTE E22-900T22S | Selected |
| I²C I/O expansion | MCP23017 | Selected |
| **Boat battery voltage sensor** | **Ardboard INA231 I²C, product 1491, BUS-only, address 0x40 candidate** | **R27 selected / unpurchased / uncommissioned; never series-wire its R100 0.1Ω shunt in the 20A pack circuit** |
| Motor driver | Cytron MDD20A dual channel | Selected |
| Four-channel release driver | EKM014 | Selected |
| Main 5V rail | Olimex DCDC6-16-TO5 | Selected; powers ESP32-S3, M100 PRO and E22-900T22S VCC; one fitted, second optional only if measurements justify |
| EKM014 logic rail | TRACO TSR 2-2450 | Selected |
| 3.3V RF rail | Pololu D24V10F3 #2830 unit 1 | nRF24L01+ PA/LNA |
| 3.3V sensor/I/O rail | Pololu D24V10F3 #2830 unit 2 | GY-91, **INA231**, MCP23017 |
| Battery | 3S4P 18650 Li-ion, 11.1V nominal, ~10Ah, 20A BMS | Existing design baseline |

The ADS1115 was already assumed by the Boat's I²C ADC acquisition code at address 0x48 but was omitted from R19's parts table. R20 makes that module explicit. It is a measurement module, not a separate safety controller or power cutoff.


> **R27 ACTIVE SELECTION OVERRIDE:** The sections below describing ADS1115 AIN0 and R21 150kΩ/33kΩ are retained solely as historical engineering context. **Do not buy or wire ADS1115 or the 150kΩ/33kΩ divider for the active Boat battery sensor.** Selected module is Ardboard INA231 product 1491 operated in BUS-voltage-only mode, with **3S sense lead disconnected until physical inspection** and all commissioning flags false. The original ADS1115 acquisition code has been blocked. [R27 selection and safe wiring](BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md).

## Battery voltage input

**The battery-divider values in the original R20 draft were replaced by R21.** See [R21 ADC commissioning and power sequencing](BOAT_R21_BATTERY_ADC_COMMISSIONING.md). Other R20 module selections remain active.

- Use a premade ADS1115 breakout powered from the Boat 3.3V sensor/I/O rail.
- I²C address: 0x48; use AIN0 for pack voltage.
- **R21 superseding divider candidate:** 150kΩ from switched/protected battery positive to AIN0 and 33kΩ from AIN0 to sensor ground; both 1% resistors. Check power sequencing and unpowered-ADC input voltage.
- Add 100nF from AIN0 to ground near the ADC input.
- **R21 ratio:** 5.54545:1; at 12.6V full charge, ADS1115 AIN0 is approximately **2.272V**, safely below the firmware's 3.0V sanity gate while the 3.3V ADC is powered.
- ADS1115 input configuration remains ±4.096V, single-shot AIN0-to-GND, 128SPS; supply and I²C pull-ups must be 3.3V.
- Firmware example stores the candidate ratio with `commissioned=false`. Do not enable the commissioned profile until actual ADC readings are checked against a calibrated DMM at several pack voltages and under representative load.
- Candidate I²C pins are GPIO7 (SCL) and GPIO8 (SDA) per [R20 GPIO/header map](BOAT_GPIO_HEADER_MAP_R20.md); remain unsigned until the exact board revision is inspected.

## Battery protection policy — software R79

The Boat measures battery voltage locally and does not depend on either radio to make the safety decision.

| Filtered pack voltage | Boat behavior |
|---|---|
| Above 11.4V | Green / OK |
| 10.8V to 11.4V | Yellow / mid battery; recharge recommended |
| At or below 10.8V for 3s | Red; hold outbound mission motion and request validated RTH |
| At or below 10.2V for 1s | Critical STOP, latched |
| At or below 9.9V | Immediate critical STOP |
| ADC sample invalid/stale >1s | Fail-closed STOP |

The filter is a 3:1 IIR average. RTH and critical STOP have separate confirmation times and latch. Critical STOP overrides an active RTH. The Remote shows the Boat's voltage and estimated percentage: green above 50%, yellow 21–50%, red at or below 20% or whenever the Boat reports low battery. A stale or invalid sample displays unavailable.

Percentage is an estimated 3S Li-ion voltage curve, not a coulomb counter. The actual cell model, temperature, internal resistance and voltage sag affect the estimate. Thresholds are commissioning defaults, not final certified limits.

## Low-battery mission behavior

- At red, outbound mission movement is held immediately; new mission start and release requests are blocked.
- After the 3-second confirmation, an active eligible mission is diverted to its locally stored Home point. The outbound target and remaining release sequence are discarded.
- If a release pulse is active, its output is deenergized first. RTH waits for a newer physical release-idle/current-zero feedback sample.
- RTH requires a valid matching Boat NVS Home record, fresh GNSS and heading, valid geofence/navigation, healthy battery above critical, released STOP and valid release interlocks.
- If any required gate fails, the application latches STOP; it does not guess a route.
- During RTH all hopper/hook outputs stay OFF. If battery reaches critical, all propulsion stops.
- If there is no active autonomous mission, the Boat warns but does not invent a new mission. Manual return is the operator's responsibility until critical STOP.
- The Remote blocks mission preparation if Boat battery telemetry is stale or the estimated charge is 20% or lower.

Full behavior: [BOAT_R79_LOW_BATTERY_FAILSAFE.md](BOAT_R79_LOW_BATTERY_FAILSAFE.md).

## Power distribution

E22-900T22S VCC is allocated to the 5V DCDC6 main rail for rated RF output power; UART/control logic remains 3.3V TTL. D24V10F3 unit 1 is reserved for nRF24 PA/LNA and unit 2 for GY-91/ADS1115/MCP23017.

| Rail | Source | Loads |
|---|---|---|
| 5V main logic/GNSS/RF | 1× DCDC6-16-TO5 (5V/2A) | ESP32-S3 board, M100 PRO and E22-900T22S VCC; validate combined peak current/noise |
| 5V EKM014 logic | 1× TSR 2-2450 | EKM014 VL only |
| 3.3V RF | D24V10F3 #2830 unit 1 | nRF24L01+ PA/LNA |
| 3.3V sensors/I/O | D24V10F3 #2830 unit 2 | GY-91, ADS1115, MCP23017 |
| Motor power | 3S battery through fused distribution | MDD20A |
| Solenoid power | 3S battery through fused distribution | EKM014 VH |

Do not parallel regulator outputs. Confirm nRF24 PA/LNA supply filtering against the exact purchased carrier. EKM014 VH high-current power is separate from VL logic power.

## Superseded selections

- Heltec Wireless Tracker v1.2 / integrated SX1262 is not the active Boat controller or backup-radio design.
- Active controller is ESP32-S3-DevKitC-1-N32R16V with HGLRC M100 PRO and external E22-900T22S.
- No custom carrier PCB or custom driver PCB is part of the active design.
- Older Heltec and central-carrier architecture records are in [the historical archive](../../docs/archive/2026-11-22/README.md).

## GPIO/header map

The unsigned candidate pin allocation is documented in [BOAT_GPIO_HEADER_MAP_R20.md](BOAT_GPIO_HEADER_MAP_R20.md). Firmware pin-audit flags remain false until the exact purchased board revision is verified.

## GPIO/header map

See [BOAT_GPIO_HEADER_MAP_R20.md](BOAT_GPIO_HEADER_MAP_R20.md) and [BOAT_R20_GPIO_RESOURCE_CROSSWALK.csv](BOAT_R20_GPIO_RESOURCE_CROSSWALK.csv). The candidate mapping is not signed; the board has not been purchased and pin-audit flags remain 0.

## Remaining hardware decisions and acceptance

1. Sign the candidate GPIO/header map in [BOAT_GPIO_HEADER_MAP_R20.md](BOAT_GPIO_HEADER_MAP_R20.md) against the exact purchased ESP32-S3 board revision.
2. Verify ADS1115 divider ratio and offset against a DMM; commission sensor profile only after calibration.
3. Measure supply current, ripple and temperature on DCDC6-16-TO5, both D24V10F3 rails and TSR 2-2450 during boot, GNSS acquisition and simultaneous radio transmission.
4. Measure each 550 motor startup/stall current and each solenoid pulse current; calculate fuses, cable gauges and connector ratings against the 20A BMS.
5. Verify MDD20A and EKM014 mapping, direction, pulse limits, current-zero feedback and one-at-a-time behavior using dummy loads.
6. Calibrate/compare M100 PRO QMC5883L and GY-91 magnetometers and physically separate compass from motor wiring and power loops.
7. Finalize mounting, waterproofing, cable strain relief and antenna placement in the hull.
8. Complete the radio-loss and low-battery dummy-load bench tests before connecting real motors or solenoids.
9. Perform physical RF, GNSS, power, actuator and controlled water acceptance.

No physical result is inferred from CI. Sonar model selection remains deferred.

## R23 physical connection design overlay

The current R20-selected module list above remains in force. Its manufacturer-checked 18-pin ESP32-S3 assignment and 64 explicit wiring nets are published in [BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md](BOAT_R23_GPIO_PINOUT_AND_HARNESSES.md) and [BOAT_R23_MODULE_CONNECTION_MATRIX.csv](BOAT_R23_MODULE_CONNECTION_MATRIX.csv). The [active purchase register](../../docs/PROJECT_PURCHASE_REGISTER_R20.csv) now reflects R20/R23 **nRF24 on Pololu D24V10F3 #1**, **GY91/ADS1115/MCP23017 on Pololu #2**, **E22 at regulated 5V**, and optional quantity-zero nRF24 socket adapter.

The manufacturer supports MDD20A 3.3V PWM/DIR control and EKM014 3.3V IN1–IN4 with 5V VL; EKM014 VH must be >=VL and is not to be shorted to the 12.6V full-charge pack logic supply. The original 3S motor/solenoid high-current feeds remain physically unapproved. Check HGLRC M100 PRO UART and I²C output levels while powered at 5V before connecting to the ESP32-S3. This design is **not a completed wiring/physical safety acceptance**.

## R26 procurement hold — INA226 not selected (2026-11-22)

R20's ADS1115 0x48 remains the **selected but uncommissioned** battery-sensing module. R25 identified an unresolved VDD-relative power-off limit for the 150kΩ/33kΩ divider; do not connect 3S sense until a manufacturer-compliant power path is accepted. R26 compared TI INA226 VBUS-only as a proposed preassembled replacement that can handle VBUS while chip VS is off, but **NO change to the purchased BOM, module choice, breakout harness or firmware profile is authorized**. See [R26 INA226 candidate review](BOAT_R26_INA226_VOLTAGE_ONLY_REVIEW.md). High-current motor/solenoid paths also remain disconnected until independent safety hardware approval.

## R29 active power authority — one shared 3S load bus (2026-11-22)

**Superseding owner instruction:** use **one** 3S4P BMS-protected battery supply with a **single common 3S high-current distribution/return** connected in parallel to Cytron MDD20A `VM` and EKM014 `VH`. DO NOT construct separately switched/isolated motor and coil high-current buses from older R20/R23 drafts. This replaces only actuator power wiring, **not** the four preselected voltage-conversion regulators, since EKM014 still needs 5V `VL` logic and sensors/radios need 3.3V/5V. [R29 common-supply wiring and design checks](BOAT_R29_SHARED_BUS_FAULT_SIMULATION.md).

No physical bus/protection device is installed or signed. The chosen 20A BMS, EKM014 factory 10A slow-blow fuse, unmeasured 550 motor stall, 6A class release coil, wire terminals and shared ground require actual test and safe upstream protection selection. Software motor/release exclusivity can prevent a *commanded* overlap but cannot independently disconnect a failed output. Existing independent safety contacts/feedback are unavailable: they stay rejected/uncommissioned. Boat VER014 / Remote VER028 unchanged.
