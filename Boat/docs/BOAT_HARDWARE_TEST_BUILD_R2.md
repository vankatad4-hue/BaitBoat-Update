# Boat Hardware Test Build R2

**Bench only; no motor/solenoid operation until safety integration is reviewed.**

## Hardware baseline
- Reported pack3S4P Li-ion, BMS20A; confirm actual cells/BMS.
- Retain main switch and battery indicator.
- Controller candidate Heltec Wireless Tracker ESP32-S3+UC6580 GNSS. Exact board revision/pin map verify; integrated SX1262 unused in first prototype.
- Primary nRF24L01+ PA/LNA, 2.4GHz.
- Backup matching E22-900T22S UART LoRa module.
- Motor driver Cytron MDD20A candidate, external placement required; 88.9×78.74mm board exceeds77×79mm carrier envelope.
- Bait FW0630 009 DC12V; hook FW0630 001 DC12V.

## Power distribution
3S pack → BMS → main fuse → retained switch → BUS_3S.
- Motor-driver branch with fuse sized after motor start/stall-current measurement.
- Electronics 5V buck candidate ≥2A peak for controller + E22; exact controller input and load confirm.
- E22 VCC from regulated5V; UART/M0/M1/AUX 3.3V logic. Local bulk/ceramic bypass.
- nRF 3.3V rail from verified controller rail or dedicated buck; local100nF+10µF+22–47µF candidates; measure burst droop.
- Solenoid supply: raw pack ranges9–12.6V. Direct pack only if coils tolerate this; otherwise regulated12V buck-boost sized for measured simultaneous coil current.
- Motor and solenoid returns separated from logic/RF ground; star/low-impedance return.
- Branch fuses, connector, cable gauge, BMS, main fuse and driver coordinated.

## Logical radio pin map (GPIO numbers TBD)
- nRF SPI, direct CE/CSN, IRQ; avoid integrated SX1262 SPI, GNSS, USB and strapping.
- E22 direct MCU UART 9600 8N1; M0/M1 default low; AUX input; reset accessible.
- Matching E22 settings: network/address0, air62.5kbps, packet32, 10dBm, channel18=868.125MHz, transparent.
- Boat E22 test sketch and nRF test sketch remain disabled until GPIO macros are assigned from exact board schematic.

## Motor and solenoid outputs
- MDD20A outside the low-voltage carrier or compartment revision. M1A/M1B and M2A/M2B to motors; PWM/DIR harness; default PWM low and DIR defined.
- Each solenoid low-side MOSFET, gate100Ω candidate, gate-source100k pulldown, flyback diode; individual fuse. MOSFET/diode current/thermal select after coil measurements.
- Software pulse limits, lockout, output-off reset/watchdog/link-loss, duplicate command-ID suppression and ACK/state confirmation required.
- No outputs are connected in radio bench test.

## Test order
1. Pin audit and connector continuity on exact Tracker revision.
2. Current-limited electronics-only power; measure5V,3V3 and thermal.
3. nRF bench pair; nRF pins still unset on boat.
4. E22 UART/config/readback; configure both endpoints identically.
5. Radio probe pair, short range, antennas attached; packet CRC and logs.
6. Simultaneous RF/converter transient test.
7. Motor startup/stall-current test on current-limited fixture with driver external and no bait/hook mechanisms.
8. Coil resistance/current/voltage tolerance and pulse test with each coil isolated.
9. Fuse/BMS/wiring/connector thermal and failsafe review before hull integration.

## Blockers
Exact controller revision/pin map, power-converter design, pack/current measurement, motor driver placement, solenoid driver schematic, native KiCad/ERC/DRC, authenticated session/key, app ACK and boat main loop/failsafe/actuator integration are incomplete.
