# Flytec V080 Boat hardware selection — R19

Date: 2026-11-22  
Branch: `hardware-ready-dual-radio-r1`  
Status: selected parts are confirmed as the design baseline but are still to be purchased and physically verified. This revision supersedes the older Heltec Wireless Tracker / integrated SX1262 candidate lists in R15/R18.

## Active selected hardware

| Function | Selected part | Status |
|---|---|---|
| Main controller | ESP32-S3-DevKitC-1-N32R16V | Confirmed |
| GNSS | HGLRC M100 PRO | Confirmed |
| Compass | QMC5883L integrated with M100 PRO | Confirmed |
| IMU/barometer | GY-91 (MPU-9250 + BMP280) | Confirmed |
| Primary radio | nRF24L01+ PA/LNA | Confirmed |
| Redundant radio | EBYTE E22-900T22S | Confirmed |
| Motor driver | Cytron MDD20A dual channel | Confirmed |
| Four-channel solenoid driver | EKM014 | Confirmed |
| Low-voltage I/O expansion | MCP23017 | Confirmed |
| Controller/GNSS 5V rail | Olimex DCDC6-16-TO5 | Confirmed; one fitted, second optional only if measurements justify |
| EKM014 logic VL rail | TRACO TSR 2-2450 | Selected |
| 3.3V rail for E22 | Pololu D24V10F3 #2830 | Selected |
| 3.3V rail for GY-91 and assigned sensor/I/O branch | Pololu D24V10F3 #2830 | Selected |
| Battery | 3S4P 18650 pack, nominal 11.1V, about 10Ah, 20A BMS | Existing design baseline |

The QMC5883L is on the M100 PRO module; GY-91 provides the MPU-9250/BMP280 sensor set. Both heading sources must be calibrated/compared before the active heading source is frozen.

## Superseded candidates

- Heltec Wireless Tracker v1.2 as the main Boat controller/GNSS/LoRa board is superseded by the ESP32-S3-DevKitC-1-N32R16V + HGLRC M100 PRO + external E22 design.
- The integrated SX1262/Heltec LoRa path is not the active Boat backup radio. Use the selected external E22-900T22S UART module.
- RECOM R-78E3.3-0.5 is not in the selected power design.
- No custom carrier PCB or custom driver PCB is to be introduced. Keep the selected ready-made modules and removable, keyed wiring.

## Confirmed rail allocation

| Rail | Source | Loads |
|---|---|---|
| 5V main logic/GNSS | 1× DCDC6-16-TO5 | ESP32-S3 board and M100 PRO; validate combined peak current and noise |
| 5V EKM014 logic | 1× TSR 2-2450 | EKM014 VL only |
| 3.3V RF | D24V10F3 #2830 unit 1 | E22-900T22S |
| 3.3V sensor/I/O | D24V10F3 #2830 unit 2 | GY-91 and assigned low-voltage sensor/I/O branch |
| High-current motor | 3S battery through the MDD20A input path | MDD20A; fuse/cable sizing remains open |
| Solenoid power | 3S battery through the EKM014 VH input path | Four solenoid channels; fuse/current/pulse validation remains open |

Do not parallel regulator outputs. The nRF24 PA/LNA supply/carrier and decoupling must be checked against the exact purchased module before it is connected. The EKM014 VH high-current path is separate from its logic VL rail.

## What remains to be decided or physically verified

The component list is no longer the main blocker. The remaining work is pin-level and load-level engineering:

1. **Exact GPIO/header map:** sign off the exact ESP32-S3 DevKitC-1 N32R16V board revision and header pins for GNSS UART, nRF24 SPI, E22 UART/M0/M1/AUX, GY-91 bus, MCP23017 I2C, MDD20A PWM/DIR, EKM014 IN1–IN4 and all available feedback. Current firmware deliberately leaves actuator GPIOs disabled.
2. **nRF24 and E22 wiring:** confirm exact carrier/socket, supply filtering, logic levels, UART/SPI pin mapping, antenna connectors, antenna separation and RF configuration. Verify the E22 frequency/channel, legal EU settings and airtime before range testing.
3. **Power integrity:** measure peak current and ripple on DCDC6-16-TO5, both D24V10F3 rails and TSR 2-2450 during ESP32 boot, GPS acquisition, radio transmit and EKM014 logic operation. Confirm no backfeed and acceptable temperatures. Decide on the optional second DCDC6-16-TO5 only from these measurements.
4. **Fuses and high-current wiring:** calculate fuse ratings, wire gauges, connector ratings and battery distribution from measured loads. Measure both 550 motor startup/stall current and each solenoid current/pulse. Check combined demand against the 20A BMS and battery wiring. Do not guess final fuse values before these measurements.
5. **MDD20A integration:** confirm motor polarity, PWM/DIR levels, current limit, output feedback and thermal behavior using dummy loads first.
6. **EKM014 integration:** confirm VL/VH wiring, logic-level compatibility, channel mapping, pulse duration, idle/current-zero feedback and one-at-a-time behavior using dummy loads. No real solenoid is connected during this stage.
7. **Sensor placement/calibration:** physically separate compass/GNSS from motor cables, driver and radio power loops. Calibrate/compare the M100 PRO QMC5883L and GY-91 magnetometer, then freeze the heading source and orientation.
8. **Mechanical installation:** finalize module mounting, water protection, cable strain relief, antenna placement and service access in the actual hull. The earlier compact board-bay limit is no longer the controlling constraint because modular placement alongside the boat is allowed.
9. **Physical acceptance:** buy/assemble the selected parts, run the R78 dummy-load test, then perform controlled water testing. No physical result is inferred from host tests or target compilation.

## No-new-hardware constraint

The user's instruction is to implement the radio watchdog without adding new hardware elements. R78 therefore uses the ESP32-S3 software watchdog and the controller's internal NVS; it does not add a separate safety PLC, contactor, hardware watchdog, monostable, current-sensor module or custom PCB.

This is not an independent physical cutoff. A single-MCU software watchdog cannot protect against MCU lockup, loss of the MCU power rail or a failed motor-driver transistor. This limitation must remain visible in the acceptance record; do not describe the software watchdog as an independent hardware safety channel.

## Sonar

Sonar model selection remains deferred. Do not integrate a sonar module until the user selects the model.
