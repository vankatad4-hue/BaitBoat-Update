# Boat Electrical Schematic Design R2 — test hardware

Supersedes R1 LoRa pairing assumptions. Initial prototype uses matching E22-900T22S modules on both endpoints; Heltec Tracker's onboard SX1262 is not used for backup in R2.

## Pack and main bus
Reported pack: 3S4P Li-ion, 20A BMS (confirm cells, BMS and actual pack).
- PACK+ → main fuse (candidate 20A, final after current measurements) → master switch (retained) → BUS_3S.
- PACK− → high-current GND.
- Original battery indicator retained; verify voltage range/polarity.
- Separate branches and fuses:
  - motor driver supply (fuse coordination with measured stall/start currents)
  - electronics 5V buck (controller + E22)
  - solenoid supply (regulated 12V if coil voltage tolerances require, or direct pack only after test)
- The 20A BMS, main fuse, motor driver, battery wiring and connector must be treated as a coordinated current limit. Two motors at stall may exceed pack/BMS ratings.

## Electronics rail
BUS_3S → 5V buck converter, candidate ≥2A peak (select after Tracker/controller + E22 + GNSS load measurement).
- 5V powers the Heltec Tracker/controller via its approved power input and E22 backup module.
- Add branch fuse (candidate 2A; adjust after load).
- Converter input/output capacitors and layout follow selected converter reference.
- Controller local 3V3 rails power sensors and nRF; verify current capability and RF bursts. If inadequate, add dedicated 3V3 buck.
- Controller and RF grounds return to electronics star point; motor current must not flow through sensor/antenna ground paths.

## Primary nRF24 radio
External nRF24L01+ PA/LNA module:
- 3V3_RADIO; local 100nF+10µF+22–47µF candidate.
- SPI MOSI/SCK/MISO, direct CE/CSN, IRQ to MCU.
- Pins TBD from exact Heltec Tracker board schematic. Avoid onboard SX1262 SPI, GNSS, USB, strapping pins.
- 2.4GHz antenna, separate from LoRa/GNSS antennas.
- Start at 250kbps, channel76, PA_LOW and auto-ack/retries for bench; validate coexistence with integrated radios and Wi-Fi.

## Backup E22 LoRa module
Matching E22-900T22S UART module:
- 5V_ELECTRONICS rail (module's supply range and max-power conditions verified).
- 3.3V UART logic, direct MCU UART pins after exact pin audit; 9600 8N1.
- M0/M1 pull-down/controlled; AUX input; reset access.
- 50Ω antenna to dedicated 868MHz connector.
- Local 100µF+10µF+100nF candidate; scope rail at TX.
- Initial module settings same as remote: channel18 868.125MHz, 62.5kbps air, 32-byte subpacket, 10dBm TX, transparent mode, network/address 0.
- Duty-cycle budget: 1% band. Initial 25ms/frame estimate, ≤1 frame/3s per endpoint; measure. Duty accounting must persist across reset before field use.

## Solenoid power
Two coils nominal DC12V:
- Bait FW0630 009; hook FW0630 001.
- Each coil positive to SOL_12V branch; coil negative to individual low-side MOSFET drain.
- MOSFET source to solenoid power return. Gate through 100Ω candidate from MCU; 100k pulldown gate-source. Gate drive must be 3.3V-compatible and MOSFET selected by measured coil current/thermal.
- Flyback diode across each coil (cathode to coil +, anode to switched negative), rating based on measured current and release-time requirement. TVS clamp may be evaluated.
- Individual branch fuse. Outputs default OFF on reset/watchdog/link loss.
- SOL_12V: raw 3S bus is 9–12.6V and may over/under nominal coil rating. Test coil temperature/force/release time and pack range. If direct pack is not within coil spec, use regulated 12V buck-boost sized for both coils' simultaneous peak/duty. Do not assume coil can tolerate 12.6V.
- No solenoid may be held energized indefinitely; software pulse limit and lockout.

## Motor power and driver
Cytron MDD20A remains candidate external module. It is 88.9×78.74mm and exceeds the 77×79mm carrier footprint. Keep it outside the low-voltage carrier footprint unless the compartment is revised. Main motor bus fused separately.
- Left/right 550 motors connect to M1A/M1B and M2A/M2B.
- PWM/DIR input harness from carrier, 3.3V logic; both PWM default LOW and DIR defined on reset.
- Confirm driver voltage, current, thermal, polarity and current limit. Motor startup/stall current measurement required; BMS is 20A reported, so simultaneous motors must be checked.
- Main bus and motor return use heavy copper/wires and a low-impedance return; do not route motor current through controller ground.
- Reverse polarity protection upstream of driver; MDD20A has no reverse protection.

## Controller interfaces / connector schedule
See BOAT_HARDWARE_PIN_CONNECTIONS_R1.csv; all MCU GPIO numbers remain TBD pending board schematic.
- J_CTRL_MOTOR: DIR1/PWM1/DIR2/PWM2/GND.
- J_NRF: SPI+3V3+CSN+CE+IRQ.
- J_E22: 5V, GND, UART_RX/TX, M0, M1, AUX, RESET, antenna RF (module-specific RF connector/pigtail).
- J_BAIT/J_HOOK: coil positive and low-side switched negative.
- J_SENSORS: I2C, 3V3, GND.
- J_GNSS/SONAR: exact module connector and UART levels to be finalized after module selection.
- Battery monitor: divider into ADC, values sized to board ADC range; battery voltage and low-voltage return failsafe.

## Fit and heat
- Control carrier: 77×79×14mm measured preliminary.
- MDD20A larger than carrier; separate placement or revised compartment.
- Lid clearance reportedly ~30mm. Check module terminal height, coax bend, heat, waterproofing and service access with CAD/physical fit.
- Do not combine high-current motor drivers and sensitive RF/ADC traces on a tiny PCB without thermal/current-path design.

## Test-build gate
Power rails/current, motor startup/stall, solenoid coil resistance/current, fuse/cable/connector thermal, nRF and E22 RF tests, link watchdog, action dedupe, output-off reset and waterproofing all require bench validation. No boat actuators to be connected until firmware safety path is reviewed.
