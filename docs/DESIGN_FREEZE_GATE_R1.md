# Flytec V080 Retrofit — Design Freeze Gate R1

**Purpose:** Do not finalize schematics, PCB layout, or fabrication files until every blocking item below is resolved or explicitly accepted as a documented design exception.

## 1. Remote
- [ ] Confirm exact Waveshare ESP32-S3-Touch-LCD-4.3 **Standard** board revision; obtain matching official schematic, pinout and mechanical drawing.
- [ ] Verify touch-controller, IO-expander, display interface, USB/charging/power circuitry and all GPIO conflicts against the exact board.
- [ ] Verify the proposed 1S4P pack configuration, cell model, cell holder/welded pack construction, protection, fuse, connector and measured maximum load.
- [ ] Validate controller/display 5V source under simultaneous display brightness, Wi-Fi and radio activity.
- [ ] Confirm TPS63070 3.3V radio rail output/current/transient response against the selected nRF PA/LNA module.
- [ ] Confirm TPS61023 boost implementation and E22 current/transient/thermal requirements; no converter-output paralleling.
- [ ] Verify SC16IS750 necessity, I2C address, voltage levels, UART baud, M0/M1/AUX behavior and bridge reset strategy.
- [ ] Resolve nRF CE/CSN/IRQ pin mapping and every pin conflict with board peripherals; verify by continuity or a test sketch on the exact board.
- [ ] Confirm both antenna assemblies, connectors, cable loss, separation and mounting/grounding.

## 2. Boat
- [ ] Identify the exact Heltec Wireless Tracker PCB revision and map all usable GPIOs, boot/strap pins, display/USB/SD/GNSS resources.
- [ ] Confirm battery pack chemistry, cell model, 3S4P wiring, BMS rating and measured pack voltage under load.
- [ ] Measure each 550 motor's running, startup and stall current at relevant voltage/load; identify motor wiring and polarity.
- [ ] Select a motor driver only after currents, thermal conditions, mounting and compartment dimensions are known. Cytron MDD20A candidate is larger than the original 77×79mm carrier footprint.
- [ ] Confirm solenoid exact model/coil resistance, current, pull-in/hold behavior, duty-cycle and thermal limit for both FW0630 009 and FW0630 001.
- [ ] Select and validate each solenoid driver, flyback/clamp, gate/base drive, fuse, wiring and independent hardware output-disable.
- [ ] Confirm controller supply, radio 3.3V rail, GNSS/sensor rail and noise filtering with motors/solenoids isolated.
- [ ] Select exact sonar module and document power, interface, mounting, transducer position, data rate and display integration.
- [ ] Verify compass/GNSS mounting and interference from motors, wiring, batteries, radios and steel components.

## 3. Radios and safety
- [ ] Confirm exact nRF24 PA/LNA module manufacturer/revision, supply needs, antenna type and RF power setting.
- [ ] Confirm matching E22 modules, antenna, channel/frequency, air rate, UART framing, power and configuration readback.
- [ ] Implement authentication/key provisioning, replay protection, ACK/command-result, duplicate action handling, link-loss and recovery state machine.
- [ ] Define safety-rated STOP behavior, actuator inhibition, manual override priority and independent local failsafe.
- [ ] Validate RETURN_HOME prerequisites and fail-closed behavior when GNSS, heading, battery, geofence or system health is invalid.
- [ ] Assess EU/Bulgaria frequency, power, duty-cycle, listen-before-talk/adaptive-frequency and antenna requirements; measure actual airtime and RF behavior.

## 4. Mechanical and harness
- [ ] Measure all compartments, lid clearance, mounting holes, cable bend radii, connector access and antenna clearances.
- [ ] Create CAD placement for carrier boards, motor driver, converters, radio modules, sonar, harnesses and service access.
- [ ] Freeze keyed connector families, pin numbering, wire gauge, current rating, strain relief and separation of high-current/logic/sensor/radio wiring.
- [ ] Perform enclosure water ingress, vibration, corrosion and thermal assessment.

## 5. Evidence required before freeze
- [ ] Exact-component BOM with manufacturer part number, datasheet and sourcing/revision notes.
- [ ] Native schematics with ERC clean or documented exceptions.
- [ ] Pin map and connector tables cross-checked against schematic and actual board.
- [ ] PCB DRC clean or documented exceptions; high-current/RF/power/thermal layout review.
- [ ] Bench plan with staged power-up: logic only → sensors → radios → simulated actuator loads → motors on secured test fixture → water tests only after safety review.
- [ ] Test logs for supply rails, current/temperature, RF links, failover, STOP, lost-link behavior, recovery and actuator interlocks.

## Current blockers
Exact display/controller revisions, motor and solenoid electrical data, motor-driver selection, sonar module, power converter validation, antenna/radio module details, mechanical fit and safe firmware integration remain open. The current repository is a development baseline, not a fabrication release.
