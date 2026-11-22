# R79 Boat battery and radio-loss dummy-load bench test

Status: procedure prepared; physical test not run. Actual motors and solenoids remain disconnected.

## Hard safety rule

Use current-limited bench supplies. Keep both 550 motors and all four real solenoids physically disconnected and insulated. Use logic indicators first, then rated resistive dummy loads within measured driver and supply ratings. Do not guess resistor wattage or current. Keep EKM014 VH disabled until logic and one-at-a-time checks pass.

## A. Power rails and ADS1115 voltage-sense calibration

### Rail assignment

| Rail | Source | Loads |
|---|---|---|
| 5V main | DCDC6-16-TO5, 5V/2A | ESP32-S3 DevKitC-1, M100 PRO and E22-900T22S VCC |
| 5V EKM014 logic | TSR 2-2450 | EKM014 VL only |
| 3.3V RF | D24V10F3 #2830 unit 1 | nRF24 PA/LNA |
| 3.3V sensor/I/O | D24V10F3 #2830 unit 2 | GY-91, ADS1115, MCP23017 |
| 3S battery motor | Fused battery distribution | MDD20A motor supply |
| 3S battery actuator | Separate fused battery distribution | EKM014 VH |

E22 VCC is 5V for its rated RF output-power condition; E22 UART/M0/M1/AUX remain 3.3V TTL. Do not connect 5V to its logic pins. Do not parallel regulator outputs. Keep EKM014 VH disconnected during the logic and ADC stages.



1. Confirm the purchased ADS1115 breakout is the ADS1115 (not ADS1015), I²C address 0x48, powered from 3.3V with pull-ups to 3.3V.
2. Install the 100kΩ/33kΩ 1% divider from pack positive through AIN0 to ground and a 100nF capacitor from AIN0 to ground. Verify polarity and no shorts before applying battery voltage.
3. Use a current-limited supply or protected pack simulator. Measure pack voltage and AIN0 voltage with a calibrated DMM at several points across the usable 3S range.
4. Verify 12.6V pack input gives approximately 3.13V at AIN0. Confirm AIN0 never exceeds ADS1115 supply voltage or its permitted input range.
5. Calculate actual divider gain/offset from DMM measurements. Update the commissioned sensor profile only after the measured error is acceptable.
6. Verify ADC I²C address, fresh conversion timestamp and voltage sample age. Disconnect the ADC or interrupt I²C: the Boat must enter STOP when the battery sample becomes stale (>1 second).

## B. Battery threshold simulation

Use a calibrated programmable supply in place of the 3S pack; do not use the actual battery to force undervoltage.

1. At 12.0V, verify green status and approximately 80% estimated charge.
2. At 11.3V, verify yellow status and the Remote message `СРЕДНА БАТ. — ЗАРЕДИ`.
3. Hold 10.7V: outbound mission motion must be inhibited immediately; at 2,999ms no RTH request; at 3,000ms the low-battery RTH latch must set.
4. Verify Remote battery label turns red and shows Boat voltage/estimated charge with `НИСКА БАТ. — RTH`. At critical STOP, verify the separate `КРИТ. БАТ. — STOP` warning.
5. Raise voltage above the RTH threshold after latching: RTH must remain active and the old outbound mission must not resume.
6. Hold 10.1V for at least 1 second: critical STOP must latch and all dummy motor/release outputs must go OFF.
7. Apply 9.8V: critical STOP must be immediate.
8. Remove or stale the ADC sample: all propulsion must stop; the Remote must show unavailable battery telemetry rather than a stale green value.

## C. Mission and actuator behavior

1. Store a valid Home point through Remote mission preparation. Confirm the Boat writes the matching two-slot CRC-verified Home NVS record.
2. Start an eligible mission with dummy motor loads and no real solenoids.
3. Simulate red battery. Verify outbound target motion is held and RTH is requested after the confirmation interval.
4. If a release pulse is active, verify the output is deenergized, no further release is replayed, and RTH waits for a newer physical release-idle/current-zero feedback sample. If that feedback is unavailable or fails, the boat must remain STOP; do not assume the EKM014 output readback is coil-current feedback.
5. With valid Home/GNSS/heading/geofence/STOP/release interlocks, verify only Home navigation outputs are allowed and all four release outputs remain OFF.
6. Corrupt/miss Home, invalidate GNSS/heading/geofence, assert physical STOP, fail watchdog/feedback or force critical voltage: the boat must remain STOP.
7. With no active autonomous mission, verify red battery warns and blocks new missions/releases, but does not invent an autonomous mission. Manual steering remains available above critical for operator-controlled return.
8. Repeat radio-loss tests: both radios stale immediately STOP; 10-second radio-loss RTH and low-battery RTH both permit only local Home navigation. Critical battery always overrides either RTH and stops all outputs.

## D. Dummy-load driver tests

1. Verify MDD20A direction, neutral, Home navigation and STOP with rated dummy loads.
2. Verify EKM014 channel mapping: CH1 left hopper, CH2 right hopper, CH3 left hook, CH4 right hook.
3. Verify one channel at a time, pulse duration, idle/current-zero feedback and that no release occurs during RTH.
4. Measure rail voltage, current, ripple, driver temperature and ESP32 reset count.
5. Do not connect real motors or solenoids until all A–D tests and wiring/fuse checks pass.

## Acceptance record

Record firmware commit, board revision, ADS1115 address, DMM readings, divider gain/offset, pack simulator voltage, estimated percentage, band, RTH/critical transition timestamps, Home NVS CRC/sequence, GNSS/heading/geofence validity, STOP/feedback state, every dummy output, supply current/ripple and temperatures.

Physical acceptance is separate from host tests and target compilation.
