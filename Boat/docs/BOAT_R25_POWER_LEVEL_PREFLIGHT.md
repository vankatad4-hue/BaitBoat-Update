# R25 — Boat isolated power/logic preflight and physical evidence gate

Date: 2026-11-22. **DESIGN + LOCAL HOST TESTS ONLY**. Boat VER014 / Remote VER028 unchanged. No hardware purchased, powered, approved or commissioned.

## Key R25 risk: unpowered ADS1115

R21's 150kΩ / 33kΩ divider produces (12.6V × 33/183) ≈ **2.272V** at AIN0 while powered by 3.3V. However, **the voltage at AIN0 must remain between GND−0.3V and VDD+0.3V at ALL times**, including startup and shutdown. At VDD=0 this passive divider could attempt to drive AIN0 above the permitted range; the actual circuit can also clamp/back-power the ADC, so a simple resistor calculation cannot prove compliance. TI ADS1115 datasheet §5.1: https://www.ti.com/lit/ds/symlink/ads1115.pdf.

**Mandatory hold:** Do not connect the 3S pack sense feed to the ADC until the power-up/down relationship is engineered and independently reviewed for the actual breakout. Do not intentionally create an over-limit ADC condition in a live test. A common battery switch alone is not reliable sequencing evidence. If safe sequencing cannot be demonstrated with the selected premade modules, propose an additional commercially manufactured analog disconnect/conditioning solution for owner approval; no substitute is presently selected.

## R25 logic-only electrical order (no motor or solenoid current)

1. Record photographs, manufacturer PCB revisions and actual connector cavity order for ESP32-S3-DevKitC-1-N32R16V, M100 PRO SH1.0-6, E22-900T22S **26-pad** carrier, nRF24 2×4 and Waveshare MCP23017. Never infer cavity order from logical pin names. Keep MDD20A motor VIN and EKM014 VH physically disconnected.
2. With each module isolated from ESP pins, check 5V-powered **M100 TX/SDA/SCL** and **E22 TXD/AUX** high-state voltage using a current-limited supply, with common measurement ground. Do not plug a signal exceeding measured MCU input limits directly into the ESP. Confirm radio logic levels separately from 5V VCC supply limits.
3. Check separately the four selected regulator rails at actual low-current operating loads: DCDC6-16-TO5 5V main; TSR 2-2450 5V EKM VL; Pololu D24V10F3 #1 nRF24 3.3V; #2 GY91/ADS1115/MCP 3.3V. Also record rail ripple and thermal observations; the screening limits in the script are NOT a complete DC-DC qualification.
4. Check 3.3V I²C pullup-high levels, addresses and the MCP23017 0x27 reset/IODIR/OLAT behavior. The R24 joint register audit is only a software diagnostic; an I²C acknowledgement or register readback is not power isolation/current feedback.
5. Check **MDD20A PWM GPIO39/40 boot/reset maxima** and EKM014 IN1–IN4 boot/reset maxima with logic-only supply and an oscilloscope. EKM014 manufacturer specifies internal 120kΩ input pull-downs; this does not prove output isolation. Validate MCU-off, MCP-off and mixed supply order. Keep high-current paths open.
6. Engineer ADC sequencing with source physically isolated until demonstrated within the VDD-relative limit, then capture *measured* maximum (AIN0−VDD) during safe power-up and down. R25 records only a stricter screening requirement of peak (AIN0−VDD) ≤0.0V (recommended operating condition, rather than the +0.3V absolute maximum); also check negative minimum against GND−0.3V, scope uncertainty and overshoot manually. R25 is **not** an instruction to perform an unsafe over-limit experiment.
7. Independently select, procure and verify DC-rated hard STOP/cutoff, external watchdog, physical current-zero feedback and bounded one-hot pulse limiter before any actuator load is connected. These remain UNSPECIFIED. Software, EKM014 10A fuse, MCP readback and BMS do not close the gap.

## Machine-readable worksheet and checks

- `BOAT_R25_POWER_LEVEL_EVIDENCE.csv` has 28 mandatory rows, all currently `NOT_RUN`.
- `Boat/tools/verify_r25_preflight.py --baseline`: checks the source sheet stays completely unsigned. CI passing here is evidence that **no physical test is being claimed**, not proof that a circuit is safe.
- `--evaluate`: rejects any row without PASS plus on-repository evidence path `Boat/evidence/r25/*`, board revision, operator, date, and numeric voltage where applicable. It rejects out-of-range measurements and FAIL rows. An accepted CSV still requires independent review of scope captures, electrical ratings, ripple, transients, and the actual hardware; its numeric ranges are conservative initial screening bounds.
- Each evidence file must be committed deliberately after actual measurements. Synthetic fixture files used by unit tests are never physical evidence.
- `Boat/tests/test_r25_preflight.py` covers missing/duplicated rows, bad readings, E22/M100 5V logic errors, unsafe ADC power-down delta, boot PWM highs, evidence omission, attempted early sign-off and explicit FAIL.
- Existing R23 32-item GPIO/level sign-off sheet remains ALL `NOT_RUN`. R24 diagnostic does not connect or commission actuator outputs.

## Release authority

R25 cannot change `BOAT_NRF_PIN_AUDIT_SIGNED=0`, `BOAT_E22_UART_PIN_AUDIT_SIGNED=0`, `trackerLocalProfile.pinAuditSigned=false` or `independentHardwareCommissioned=false`. No code here touches real I²C, GPIO, power, RF or actuators. No additional electronic module or custom PCB is introduced. Physical high-current motors/solenoids remain **DISCONNECTED** pending qualified independent safety hardware and dummy-load acceptance.

Manufacturer references: [TI ADS1115](https://www.ti.com/product/ADS1115), [Espressif DevKitC-1 v1.1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html), [EKM014](https://docs.electrokit.com/en/modules/EKM014/), [EBYTE E22-900T22S](https://www.cdebyte.com/products/E22-900T22S/2), [MCP23017](https://www.microchip.com/en-us/product/MCP23017).

## R26 candidate follow-up — approved hold remains (2026-11-22)

R26 identifies TI's INA226 VBUS measurement architecture as a possible factory-assembled *replacement* for the ADS1115 divider, with the chip explicitly allowing bus voltage when unpowered. This does **not** authorize ADS1115 R21 wiring, alter R25's all-NOT_RUN measurement register, or commission any physical relay/motor/coil output. An exact INA226 breakout remains unselected and unverified: inspect VBUS/shunt routing, I²C supply-off SCL and address, and low-current ground before considering replacement. See [R26 design comparison](BOAT_R26_INA226_VOLTAGE_ONLY_REVIEW.md). 3S ADS1115 sense still disconnected.

## R27 supersession — no active ADS1115 pack-sense wiring (2026-11-22)

The R25 ADS1115 AIN0 power-off safety investigation remains valid historical evidence but **R27 selects a different voltage-only INA231 module**, Ardboard product 1491. The original R25 ADC-specific calibration and ramp checks apply to ADS1115 **only**, not INA231. Continue R25 low-voltage rail and actuator-startup physical acceptance. Current battery sense installation must follow [R27 active 21-point acceptance](BOAT_R27_INA231_ACCEPTANCE.csv), with protected low-current BUS pin and R100 shunt excluded from power path. No physical acceptance has been run.
