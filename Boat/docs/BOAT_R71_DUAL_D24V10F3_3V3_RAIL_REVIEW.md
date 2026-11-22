# R71 — Two independent Pololu D24V10F3 (3.3 V / 1 A) Boat rail proposal

**2026-11-22 | STATUS: USER-PROPOSED / ENGINEERING-COMPATIBLE, NOT FINAL OWNER PART CONFIRMATION, NOT PURCHASED OR COMMISSIONED.**

## Decision

The user asks whether **2× identical Pololu D24V10F3, part #2830** can replace the previous one D24V10F3 E22 radio rail + one RECOM R-78E3.3-0.5 GY-91/MCP23017 rail. **Yes**: both modules are 3S-compatible fixed 3.3 V buck regulators with 1 A typical maximum per rail; **use two completely independent VOUT branches, never parallel their outputs**. This replaces the *preferred engineering proposal*, not owner-confirmed purchase model. RECOM remains a *not selected alternate*, and no new active fifth 3S converter is required. Two identical modules simplifies sourcing and spare inventory.

- **Pololu manufacturer product 2830:** https://www.pololu.com/product/2830 ; [specs](https://www.pololu.com/product/2830/specs). 3.3 V (±4%), 3.4–36 V nominal allowed VIN with dropout caveat; selected Boat 3S pack ~9.0–12.6 V is within range. 1 A is typical maximum, *dependent on VIN and enclosure thermal conditions*, not a guaranteed blanket derated continuous current. ISL85410 synchronous buck ~500kHz nominal outside power-save mode; automatic low-load PWM pulse-skipping/power saving can affect sensor-line ripple. ~18×13×3.5 mm. PG open-drain, SHDN input, overcurrent and over-temperature protection. **No reverse-polarity protection**. PG roughly 80% undervoltage/120% overvoltage is not an independent safety-rated brownout threshold. Separate independent safety/STOP remains mandatory.
- **Trusted Bulgarian/EU sourcing:** https://www.tme.eu/bg/katalog/zakhranvashti-moduli_113688/p%2Cpololu_959/ lists **POLOLU-2830**, and [TME exact English product](https://www.tme.eu/en/details/pololu-2830/converter-modules/pololu/3-3v-1a-step-down-d24v10f3/) showed ~63 warehouse stock at research time (stock/checkout to Bulgaria still unverified). [Pololu official authorized distributor](https://www.pololu.com/distributors/0J51) also names Robotev Bulgaria. Do NOT order from OLX/eMAG or anonymous sellers.
- **E22 rail (D24V10F3 #1):** receives its own protected/fused 3S VIN, drives **only Boat EBYTE E22-900T22S UART LoRa**, matching factory-made SMD carrier not yet verified. EBYTE official model supply 2.3–5.5 V, 3.3 V UART-compatible, typical manufacturer listed transmit around 150–155 mA at 22 dBm (dependent on device and 3.3 vs 5.0V). Some EBYTE revisions recommend >=5 V for guaranteed full 22 dBm RF output; project uses a *lower-power legally limited EU868 backup profile*, so 3.3 V is plausible **but actual RF/link/sensitivity testing required**. [EBYTE official product](https://www.cdebyte.com/products/E22-900T22S/1). E22 radio is **not** the nRF24; nRF24 continues with the **Ardboard 1127 AMS1117 socket fed from separately regulated 5V**, R68A.
- **Sensor/I²C rail (D24V10F3 #2):** receives its *own separate protected/fused 3S VIN*, powers **GY-91 (MPU-9250+BMP280)**, **Waveshare MCP23017 I/O Expansion Board SKU 15391 at configured/default 0x27**, and QMC5883L compass *only if the exact GNSS breakout has verified 3.3V VCC*. No MCU power injection and no solenoid/PLC or high-current loads. The regulator 1 A headline is not a reason to drive MCP23017 expander outputs into EKM014. No live outputs.
- **Noise and common ground:** Buck #2 should be placed away from GY-91 magnetometer and high-current 550 motor/solenoid wiring; use short I²C cables, a measured appropriate local ceramic capacitor and optional commercial ferrite/filter after measuring switching ripple. Two *different* converter outputs reduce direct radio load modulation on sensor rail, but there remains a shared 3S bus and electrical ground. Common reference GND is required for ordinary I²C/UART (not galvanic isolation). Never join the two **3.3V VOUT** rails. Check SDA/SCL pull-ups, grounds, power sequencing and accidental back-powering of unpowered ESP32 GPIOs via external I²C boards; no arbitrary mixing of 5V GPIO.
- **Inputs and assembly:** protect both from reverse input polarity and fuse the two separate VIN harness branches based on measured current/wiring. Do not make a custom PCB. Use preassembled/approved compatible header-to-locking-connector adapters and covered terminals. PG pins need approved 3.3V-level pull-ups to be read; PG **does not** qualify an independent STOP. Fixing SHDN high is optional by the manufacturer pull-up, but never use SHDN as the only stop.
- **Rail boundaries:** Main ESP32-S3 5 V is still Olimex DCDC6-16-TO5 candidate; EKM014 VL 5 V is still Pololu S13V20F5 candidate. Nothing in this R71 update changes independent DC-rated motor/solenoid contactors, one-hot one-shot pulse safety and watchdog, or sonar deferral.

## Preliminary proposed topology, non-actuating

```text
3S PACK + BMS -> protected logic distribution
   |
   +-- fused VIN1 -> D24V10F3 #1 -> 3.3V_E22 only -> E22-900T22S factory carrier TBD
   |
   +-- fused VIN2 -> D24V10F3 #2 -> 3.3V_SENSORS only -> GY-91
   |                                                  -> Waveshare MCP23017 15391
   |                                                  -> QMC5883L (after VCC verification)
   |
   +-- Olimex 5V -> ESP32-S3 DevKitC
   |                 -> filtered 5V nRF Ardboard 1127 (conditional)
   +-- Pololu 5V -> EKM014 VL logic only

3S PACK -> fully independent tested emergency cutoffs -> MDD20A motors / EKM014 VH
```

## Acceptance gates

1. **Voltage/current:** measure 3.3 V at E22 and GY-91 across 3S ~9/11.1/12.6 V, startup, brownout, load steps and thermal soak. Aim for margin below 1 A current limit in the actual enclosure; don't infer a continuous load rating from 1 A nameplate.
2. **Radio:** verify UART levels, matching E22 SMD breakout VIN and RF output at intended *EU868 low-power* configuration. Scope LoRa TX burst droop/EMI and packet reliability.
3. **Sensors:** separate #2 supplies GY-91 MPU/BMP280 real chip IDs, QMC, MCP23017 at I²C **0x27** (unless jumpered); measure pressure/compass stability, I²C errors, heading noise with radio TX and motors OFF; test motor magnetic field separately with safe power isolation.
4. **Power faults:** PG threshold observed but never considered independent cutoff; verify no GPIO/I²C backfeed, independent fuse, reverse-polarity protection and recovery without auto actuator arming. Motor/solenoid VH disconnected.
5. **Procurement:** TME Bulgaria POLOLU-2830 quantity **2**, real purchase stock and delivery recheck. This is a preferred engineering proposal, **NOT yet a frozen owner order**.

**Release hold:** Boat VER014 / Remote VER028 unchanged; no radio/actuator GPIO commissioned, no new release ZIP or manufactured custom board.
