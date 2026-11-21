# Boat modular electrical plan R1 — 2026-11-21
Status: engineering design / NOT wiring-approved or hardware-tested. No custom PCB. Supersedes earlier E22-on-boat, RP2040 and carrier-board assumptions; existing firmware and older drawings must be reconciled separately.

## Functional single-line schematic (not an electrically certified schematic)
3S4P Li-ion pack -> correctly rated BMS -> master disconnect -> appropriately rated main fuse -> protected distribution terminals.
Branch A -> individually fused MDD20A VIN/GND -> left and right brushed 550 motors.
Branch B -> individually fused EKM014 load supply VH and driver supply VL (per manufacturer's actual wiring guide) -> four 12V solenoid positives and four independent low-side switched returns. Ensure VH >= VL. Never connect load directly to MCU GPIO.
Branch C -> individually fused 3S-capable buck regulator 5V -> Heltec Tracker approved power input (verify board schematic and supply input). Separate clean 3.3V regulator if radio module requires it; do not feed 5V to a 3.3V pin.
Branch D -> sensor supply matching actual MPU-9250/BMP280 breakout rating, shared logic ground; add separate GNSS antenna placement and RF separation.
Optional sonar: isolated/fused reserved power branch, interface TBD; no sonar selection.
Use a star distribution point for power returns, low-current signal ground referenced at a quiet point, twisted motor pairs, and physical separation between power, GNSS, compass and RF antennas.

## Module signal netlist (logical names; GPIO assignments deliberately UNRESOLVED)
Tracker MOTOR_L_PWM -> MDD20A left PWM; MOTOR_L_DIR -> left DIR; MOTOR_R_PWM -> right PWM; MOTOR_R_DIR -> right DIR. Verify MDD20A board input mode, logic threshold, enable and PWM frequency.
Tracker SOL_L_BAIT, SOL_R_BAIT, SOL_L_HOOK, SOL_R_HOOK -> EKM014 inputs 1..4, respectively. Power-on and reset must leave all channels OFF; external default-off bias / hardware inhibit must be verified, not assumed.
Tracker nRF24 CE, CSN, SCK, MOSI, MISO, IRQ(optional) -> primary 2.4GHz radio. Use 3.3V stable supply and decoupling. Integrated LoRa uses board's internal SPI; do not attach E22 on boat.
Tracker I2C SDA/SCL -> MPU9250/BMP280 breakout (confirm actual silicon; counterfeit MPU9250 risk). Battery telemetry -> protected ADC divider or ready-made monitor module; never feed 3S voltage directly to GPIO.
Tracker GNSS UART, SX1262 LoRa, TFT and boot/strapping pins reserved per exact v1.2 official pin map. No pin numbers assigned until revision-specific GPIO audit. GPIO shortage may require off-the-shelf I2C GPIO expander (MCP23017/TCA9555), but timing-critical PWM/SPI must remain on direct verified pins. If not enough safe pins remain, architecture needs review rather than inventing pin assignments.

## Safety requirements
Motor direction/PWM must default to zero, independent watchdog/disable path must be tested, STOP must latch, radio loss must command safe stop or conditional RTH only with healthy GNSS/heading/battery and safe navigation. Only one solenoid may energize at a time; per-channel pulse timeout and cooldown, power-on inhibit and mutual exclusion must be enforced and tested with dummy loads. EKM014 is not itself a one-at-a-time controller. Add emergency physical cutoff and appropriately sized wiring, fuses and connectors. Solenoid 6A is provisional; measure resistance and operating current.
The 3S battery spans approximately 9–12.6V; 12V nominal solenoids must be evaluated at full charge and depleted voltage. Regulator must tolerate full range and transient spikes. Never infer fuse sizing from BMS 20A alone.

## Purchase/acceptance gates
1. Confirm exact Heltec v1.2 SKU/schematic and header pinout; reserve GNSS, onboard LoRa, display, boot and flash pins.
2. Measure each 550 motor startup/stall current, actual running load and 4 solenoid resistance/current/duty; evaluate MDD20A and battery BMS headroom.
3. Validate EKM014 3.3V trigger, VL/VH wiring, flyback and 6A thermal behavior on bench.
4. Select documented 5V DC/DC module with automotive/transient input margin; verify peak current, ripple and temperature.
5. Finalize fuse values, connector series, conductor gauge and waterproof gland details from measured loads.
6. Produce physical point-to-point wiring diagram and continuity checklist, run dummy-load tests, then update firmware pin map. No operation before signoff.
