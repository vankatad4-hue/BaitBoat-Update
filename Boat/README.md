# Flytec V080 Boat — active hardware and firmware

**Current hardware authority:** [BOAT_HARDWARE_SELECTION_R20.md](docs/BOAT_HARDWARE_SELECTION_R20.md)  
**GPIO/header map:** [BOAT_GPIO_HEADER_MAP_R20.md](docs/BOAT_GPIO_HEADER_MAP_R20.md) (candidate, unsigned).  
**Current failsafe:** [R79 low-battery failsafe](docs/BOAT_R79_LOW_BATTERY_FAILSAFE.md)  
**Bench procedure:** [R79 battery/radio-loss dummy-load test](docs/BOAT_R79_LOW_BATTERY_BENCH_TEST.md)

R20 is the active procurement baseline. Older Heltec Wireless Tracker/SX1262 and central-carrier PCB documents are historical; see [the archive index](../docs/archive/2026-11-22/README.md).

## Selected Boat modules

- ESP32-S3-DevKitC-1-N32R16V main controller.
- HGLRC M100 PRO GNSS with integrated QMC5883L compass.
- GY-91 (MPU-9250 + BMP280).
- nRF24L01+ PA/LNA primary radio and EBYTE E22-900T22S backup radio.
- MCP23017 I/O expansion and ADS1115 battery ADC breakout at I²C 0x48.
- Cytron MDD20A dual-channel motor driver.
- EKM014 four-channel bait/hook release driver.
- Olimex DCDC6-16-TO5 main 5V, TRACO TSR 2-2450 EKM014 logic 5V, 2× Pololu D24V10F3 #2830 3.3V rails.
- 3S4P 18650 pack, 11.1V nominal, ~10Ah, 20A BMS.

Ready-made modules and removable harnesses only; no custom electronic PCB.

## R79 battery policy

- Green above 11.4V; yellow 10.8–11.4V; red at or below 10.8V.
- Red holds outbound mission movement immediately. After 3 seconds, an eligible active mission is diverted to validated Boat-NVS Home.
- Critical STOP at or below 10.2V for 1 second; immediate STOP at or below 9.9V.
- Missing/stale ADC data (>1s) fails closed.
- Low-battery RTH requires valid Home, GNSS/heading/geofence and local STOP/release interlocks. All release outputs remain OFF.
- If there is no autonomous mission, the Remote warns and manual return is the operator's responsibility. New missions are blocked at red.

Critical battery is displayed separately as `КРИТ. БАТ. — STOP`. The ADS1115 divider is a commissioning candidate (100kΩ/33kΩ, 1%, 100nF); calibrate against a DMM before enabling the sensor profile. Estimated percentage is voltage-based, not coulomb-counted.

## Current status

Software changes are under CI validation. Selected modules remain unpurchased/uncommissioned. Target compile and host-test success do not replace ADC calibration, dummy-load testing or physical acceptance. Keep actual motors and solenoids disconnected until the bench procedure passes.
