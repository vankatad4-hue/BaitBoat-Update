# Flytec V080 Boat hardware selection — R20

Date: 2026-11-22  
Branch: `hardware-ready-dual-radio-r1`  
Status: active procurement authority; selected parts are not yet purchased/commissioned. R20 supersedes R19 and the archived Heltec/SX1262 candidate records.

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
| Battery voltage ADC | ADS1115 16-bit breakout, I²C 0x48 | Required to complete the existing firmware ADC path; not yet purchased |
| Motor driver | Cytron MDD20A dual channel | Selected |
| Four-channel release driver | EKM014 | Selected |
| Main 5V rail | Olimex DCDC6-16-TO5 | Selected; one fitted, second optional only if measurements justify |
| EKM014 logic rail | TRACO TSR 2-2450 | Selected |
| 3.3V E22 rail | Pololu D24V10F3 #2830 | Selected |
| 3.3V sensor/I/O rail | Pololu D24V10F3 #2830 | Selected |
| Battery | 3S4P 18650 Li-ion, 11.1V nominal, ~10Ah, 20A BMS | Existing design baseline |

The ADS1115 was already assumed by the Boat's I²C ADC acquisition code at address 0x48 but was omitted from R19's parts table. R20 makes that module explicit. It is a measurement module, not a separate safety controller or power cutoff.

## Battery voltage input

- Use a premade ADS1115 breakout powered from the Boat 3.3V sensor/I/O rail.
- I²C address: 0x48; use AIN0 for pack voltage.
- Divider candidate: 100kΩ from battery positive to AIN0, 33kΩ from AIN0 to ground; 1% resistors.
- Add 100nF from AIN0 to ground near the ADC input.
- Divider ratio: approximately 4.0303:1. At 12.6V pack voltage, ADS1115 AIN0 is approximately 3.13V.
- ADS1115 input configuration remains ±4.096V, single-shot AIN0-to-GND, 128SPS; supply and I²C pull-ups must be 3.3V.
- Firmware example stores the candidate ratio with `commissioned=false`. Do not enable the commissioned profile until actual ADC readings are checked against a calibrated DMM at several pack voltages and under representative load.
- Final I²C GPIO pins remain part of the outstanding ESP32-S3 header audit.

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

| Rail | Source | Loads |
|---|---|---|
| 5V main logic/GNSS | 1× DCDC6-16-TO5 | ESP32-S3 board and M100 PRO; validate combined peak current/noise |
| 5V EKM014 logic | 1× TSR 2-2450 | EKM014 VL only |
| 3.3V RF | D24V10F3 #2830 unit 1 | E22-900T22S |
| 3.3V sensors/I/O | D24V10F3 #2830 unit 2 | GY-91, ADS1115, MCP23017 and assigned low-voltage branch |
| Motor power | 3S battery through fused distribution | MDD20A |
| Solenoid power | 3S battery through fused distribution | EKM014 VH |

Do not parallel regulator outputs. Confirm nRF24 PA/LNA supply filtering against the exact purchased carrier. EKM014 VH high-current power is separate from VL logic power.

## Superseded selections

- Heltec Wireless Tracker v1.2 / integrated SX1262 is not the active Boat controller or backup-radio design.
- Active controller is ESP32-S3-DevKitC-1-N32R16V with HGLRC M100 PRO and external E22-900T22S.
- No custom carrier PCB or custom driver PCB is part of the active design.
- Older Heltec and central-carrier architecture records are in [the historical archive](../../docs/archive/2026-11-22/README.md).

## Remaining hardware decisions and acceptance

1. Complete exact ESP32-S3 board-revision GPIO/header mapping for GNSS UART, nRF24 SPI, E22 UART/M0/M1/AUX, GY-91, ADS1115, MCP23017, MDD20A and EKM014.
2. Verify ADS1115 divider ratio and offset against a DMM; commission sensor profile only after calibration.
3. Measure supply current, ripple and temperature on DCDC6-16-TO5, both D24V10F3 rails and TSR 2-2450 during boot, GNSS acquisition and simultaneous radio transmission.
4. Measure each 550 motor startup/stall current and each solenoid pulse current; calculate fuses, cable gauges and connector ratings against the 20A BMS.
5. Verify MDD20A and EKM014 mapping, direction, pulse limits, current-zero feedback and one-at-a-time behavior using dummy loads.
6. Calibrate/compare M100 PRO QMC5883L and GY-91 magnetometers and physically separate compass from motor wiring and power loops.
7. Finalize mounting, waterproofing, cable strain relief and antenna placement in the hull.
8. Complete the radio-loss and low-battery dummy-load bench tests before connecting real motors or solenoids.
9. Perform physical RF, GNSS, power, actuator and controlled water acceptance.

No physical result is inferred from CI. Sonar model selection remains deferred.
