# Boat Retrofit Electrical Interface Design R1

**Status:** interface-level test design. Exact controller pinout, motor stall current, solenoid current, PCB footprint and power protections are not finalized. Boat actuators must remain disconnected until current measurements and safety review.

## System power
Reported 3S4P Li-ion pack, 20A BMS (verify actual cells, pack and BMS).
- Pack → keyed connector → master switch → main fuse located close to battery positive → protected boat bus.
- Main bus branches separately to two motor-driver power inputs, electronics buck converter, and solenoid supply branch.
- Electronics branch fuse and regulated 5V/3.3V supply based on measured load. Controller GNSS/radio rails separated from motor current returns.
- Battery indicator and original switch remain in hull. Verify indicator polarity and pack voltage compatibility.
- Main return ground is common, but motor-current return routing must not flow through controller/antenna grounds.

## Primary radio
nRF24L01+ PA/LNA module on carrier:
- SPI MOSI/SCK/MISO from controller.
- Direct MCU CSN/CE/IRQ pins selected after Heltec board pin audit.
- Dedicated regulated 3.3V rail with 100nF + 10–47µF local capacitors; burst droop and RF interference scope test.
- Separate 2.4GHz 50Ω antenna/coax, mounted away from LoRa and GNSS antenna.

## Backup LoRa
Use integrated SX1262 on Heltec Wireless Tracker candidate if exact revision/band verified. No extra E22 module on boat by default.
- Reserve native SX1262 SPI, BUSY, DIO1, RESET and RF path according to exact Heltec schematic.
- Dedicated LoRa 868MHz antenna via board's LoRa antenna connector. Confirm GNSS antenna connector is distinct; don't interchange.
- Configure PHY identical to remote E22 UART module (frequency, bandwidth, SF, CR, preamble, sync word, CRC, IQ and packet format) and test raw packet interoperability.

## Motor-driver option gate
Cytron MDD20A candidate has 6–30V input, two channels, 20A continuous per channel, 60A peak, PWM/DIR logic; board dimensions 88.9×78.74mm, and no reverse-polarity protection. It exceeds the reported 77×79mm control PCB footprint. It is a separate motor-driver module, not a carrier-PCB footprint. It cannot be declared fit until compartment/placement is measured. Confirm motor stall/start currents and thermal behavior; the pack's reported 20A BMS may constrain both motors simultaneously.
- Motor 1: M1A/M1B outputs to left motor, DIR1/PWM1/GND from controller.
- Motor 2: M2A/M2B outputs to right motor, DIR2/PWM2/GND.
- Logic interface supports 3.3V; default PWM low and DIR defined on reset.
- Driver VB+ and VB− wired to protected motor bus with correctly rated fuse and cable; do not connect controller board ground through motor current path.
- MDD20A module reverse polarity is not protected; add keyed power connector and upstream protection.

## Solenoid low-side channels
Two independent drivers for bait solenoid FW0630 009 DC12V and hook FW0630 001 DC12V:
- Connector +12V protected solenoid bus → coil positive.
- Coil negative → MOSFET drain; MOSFET source → power return.
- Gate from controller through series resistor; 100k pulldown gate-source.
- Flyback diode across coil (cathode to +12V, anode to switched negative); choose diode based on measured coil current and release-time requirement. Consider TVS/zener clamp only after pulse/driver analysis.
- Each coil branch fuse. Connector keyed, labelled, output default OFF.
- Measure coil resistance/current before selecting MOSFET, diode, fuse, connector, pulse duration and thermal duty. Solenoid should not be held energized beyond its rated duty cycle.

## Carrier PCB functions
Central low-voltage carrier PCB includes:
- controller connector, nRF connector, GNSS/heading sensor connectors (if selected), sonar connector, motor-driver control harness, two solenoid-driver channels, pack/bus voltage monitoring, emergency stop input, main switch status.
- Use keyed locking connectors; physically separate motor/solenoid current paths from low-level sensor/radio.
- 77×79×14mm is the currently measured carrier envelope only. MDD20A external module exceeds that footprint. If driver must fit compartment, revise mechanical layout or design a custom H-bridge only after motor current data.

## Safe test sequence
1. Bench supply with current limit; motor and solenoid harness disconnected.
2. Verify controller rails, GNSS, heading, nRF and SX1262.
3. Verify command decoding, link states, watchdog, duplicate suppression and solenoid interlocks in test harness.
4. Driver input test with power stage disconnected / no motor.
5. One motor on current-limited test supply; record idle, startup, running, blocked/stall current briefly using safe fixture.
6. Test each motor direction and current limiting, then both motors; thermal test at realistic duty.
7. Measure each solenoid coil resistance, current, temperature, release force/time and rated duty; verify flyback waveform.
8. Confirm fuse sizing, cable/connector thermal rise and battery BMS limits.
9. Water/field testing only after all bench safety gates.
