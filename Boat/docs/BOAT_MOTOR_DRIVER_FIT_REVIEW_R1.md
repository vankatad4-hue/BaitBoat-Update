# Boat Motor Driver Fit Review R1

## Candidate: Cytron MDD20A
The MDD20A is a dual brushed motor driver rated 6–30V, 20A continuous per channel, 60A peak, PWM/DIR, with 3.3V-compatible logic. It has overcurrent, thermal and undervoltage protection but **no reverse-polarity protection**. Datasheet/product information reports approximate board size 88.9×78.74mm.

The previously measured control-carrier envelope is 77×79×14mm, therefore the MDD20A is **larger than the available carrier footprint** (length exceeds by ~12mm). It is not an onboard-carrier footprint and should not be assumed to fit inside the electronics compartment. Its component height and screw terminals also need clearance.

## Options before hardware freeze
1. Place the MDD20A in a separate compartment/position and run a short protected motor harness; verify heat, waterproofing, wiring and accessibility.
2. Revise boat compartment/PCB envelope to accommodate a separate driver module.
3. Design a compact custom H-bridge carrier with external MOSFETs/gate drivers only after measuring each motor's startup/stall current and reviewing thermal/copper. This is not yet designed.

## Current requirements
- Motor currents (free-running, loaded, startup, stall) measured with current-limited fixture.
- Simultaneous motor current compared with pack BMS, main fuse, connector, cable and driver thermal capability.
- MDD20A reverse polarity protected externally and power connector keyed.
- Driver power and motor returns separate from logic ground path; star/low-impedance return.
- Both channels must have PWM LOW and DIR defined on reset.
- No test buttons accessible once mounted, or guard them to prevent accidental full-speed run.

## Decision status
MDD20A remains a candidate for bench testing, not a confirmed final fit. The carrier footprint must be revised or driver placed elsewhere before the final boat hardware is frozen.


## R2 control-carrier and radio fit
- Low-voltage carrier remains measured/provisional 77×79×14mm; Heltec Tracker controller board and E22/nRF radio modules must be placed with connector/coax and thermal clearances, likely with external module/harness where needed.
- The E22 module requires a separate 5V electronics branch; the 3S battery pack is not connected directly to E22 VCC. nRF module uses 3.3V rail.
- MDD20A still cannot be placed within the 77×79 carrier area. Use separate mount or revise compartment dimensions. Motor current is unknown, so don't replace it with a low-current board without stall/start tests.
