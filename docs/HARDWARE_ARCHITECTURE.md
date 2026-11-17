# Hardware Architecture — Current Decisions

## Modular architecture

Use one central carrier PCB on the boat and a separate carrier/interface arrangement in the remote. Functional modules remain separate and replaceable, connected by removable, keyed cables. This is not a plug-in backplane architecture.

Keep high-current motor and solenoid power wiring physically separate from logic, radio and sensor wiring. Define connector IDs, pin names, voltage domains, polarity and cable pinouts before assigning MCU GPIOs. Do not hot-plug powered motor or solenoid modules.

## Boat modules

1. Main controller: Heltec Wireless Tracker V1.2 (ESP32-S3 + UC6580 GNSS).
2. Navigation sensor: GY-91-style MPU-9250 + BMP280, initially I²C; verify the exact board and pinout.
3. Radio: nRF24L01+ PA/LNA with a dedicated, low-noise 3.3 V supply and local decoupling.
4. Motor driver: Cytron MDD20A dual brushed-motor driver, separate physical board.
5. Solenoid driver: independent two-channel low-side MOSFET board with per-coil suppression and branch protection.
6. Power distribution: battery input protection, fusing and regulated logic rails.
7. Sonar: separate CHIRP electronics/transducer module; select only after live echogram/raw echo-profile capability is demonstrated.

The boat carrier PCB available envelope is 77 × 79 × 14 mm. Do not place the high-current MDD20A or solenoid power stage on this small carrier.

## Remote modules

- Waveshare ESP32-S3-Touch-LCD-4.3 standard display/controller, 800×480.
- nRF24L01+ PA/LNA radio module on a dedicated low-noise 3.3 V rail.
- Joystick and physical button modules.
- Battery/power module for the planned 2×18650 parallel pack and USB-C charging.
- Reserved expansion connector.

The Waveshare display board has not yet been purchased.

## Radio and failsafe

Use matching nRF24L01+ PA/LNA modules. Begin testing at 250 kbps. The 500 m range is a field-test target, not a guarantee. Implement packet sequence numbers, application-level authentication/replay protection, acknowledgements for safety-critical commands, heartbeat and a boat-side stale-command watchdog. Hardware CRC alone is not authentication.

On manual link loss, stop propulsion. On autopilot link loss, return home only if GNSS, heading, home point and system health are valid; otherwise enter a safe stopped state. Keep sonar echo data local; send compact telemetry only.

## Motor power

The selected Cytron MDD20A provides two motor channels. Its ratings do not mean the battery pack can supply the combined current. The reported 3S4P battery and 20 A BMS need verification. Measure each 550 motor's running, startup and stall current before finalizing fuse, wire, connector and thermal design.

## Solenoid interlock

Bait and hook release are one-at-a-time. If both requests are active, both outputs remain OFF. Outputs default OFF on reset or loss of ARM. Include independent hardware mutual exclusion and a hardware time limit/watchdog; firmware timeout alone is insufficient. The 6 A per coil figure is a design assumption, not a measured value. Verify actual coil current and 12 V rating against the 12.6 V full-charge battery voltage.

## Navigation sensor placement

Calibrate the MPU-9250 magnetometer after final installation. Keep it away from motors, solenoids, steel and high-current wiring. Validate heading with motors off and running. GNSS course-over-ground is useful while moving but is not a stationary compass replacement.

## Pin assignment status

No final MCU GPIO assignments are approved yet. Verify the exact Heltec board pinout, GNSS UART/power, antenna clearances, sensor-board revision and connector electrical requirements before allocating pins or producing schematics.
