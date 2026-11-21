# R15 bench acceptance — zero custom PCBs
**Scope:** Boat and Remote. No wet tests until independently reviewed dry bench pass.

## Stage 0 — source and architecture
- [ ] R15 premade-only policy and actual vendor model numbers available on each procurement order.
- [ ] Manufacturer pinout, supply maximums and 3S / 1S compatibility verified, not from a generic lookalike product page.
- [ ] All premade modules physically dimensioned in actual V080 hull and 3D remote enclosure; MDD20A offboard and safe high-current clearance.
- [ ] All cable gauges, fuses, keyed polarities, common grounds, moisture sealing and strain relief specified; no custom PCB.
- [ ] Manufacturer support for selected LoRa partner and RF/EU868, current and antenna exposure verified; Boat SX1262 fallback remains DISABLED otherwise.

## Stage 1 — unpowered and limited logic bench
- [ ] Check the 3S battery BMS layout, pack polarity, cell balance and current limit; never direct 3S to Heltec or Waveshare 1S connector.
- [ ] Visual continuity and power-path check against active premade wiring contract and manufacturer module labels.
- [ ] Independently test RECOM R13 3.3V; Pololu 5V modules, separate 3.3V RF rails and their fuses **without motors or solenoids** using current-limited lab supply; measure start/ripple/ramp/thermal and stop behavior under input dips.
- [ ] Measure Waveshare onboard 5V and planned Pololu remote RF/LoRa voltages with 1S input. Do not parallel booster rails.
- [ ] Verify MCP23017 0x20, SC16IS750 0x48, ADS1115 0x49/0x4A after actual power-up; test joystick neutral, broken wires and button de-bounce without actuator.
- [ ] Verify nRF radio power and RF compatibility under load; no motor/coil power.

## Stage 2 — independent electrical safety gate (REQUIRED BEFORE PHYSICAL ACTUATION)
- [ ] Select ready-made, properly DC-inductive-rated cutoff contactor/relay with physical normally-closed STOP, feedback, fuse/battery derating and controlled manual rearm, verified **independently of ESP32**.
- [ ] Independent brownout/POR/fault-latch cuts actuator supply and stays off after power recovery until local ARM.
- [ ] On MCU GPIO held HIGH indefinitely, hardware onehot/pulse modules cannot repeat, cannot latch ON and cannot energize more than one channel; verify under all 16 channel-request masks and skewed input edges.
- [ ] With multiple requests, loss of timing power, I2C bus or supervisor signal, power cutoff defaults to OFF. A software-only pulse limiter **fails this gate**.
- [ ] Input from stuck-on GPIO, shorted MOSFET, lost watchdog, disconnected physical STOP, welded contactor and feedback failure must be reviewed with actual fault injection using non-actuator dummy loads.
- [ ] Confirm all planned contactors/timers and cutoff sensors are compatible with 3S min/sag/max voltage and with exact manufacturer input/output thresholds.
- [ ] Prove one-hot timing and cold/warm coil current zero independently before allowing next pulse, with no automatic rearm after brownout.
- [ ] No helper custom electronic PCB is fabricated: use only fully assembled COTS interlock/safety/sensing and terminal adapters.

## Stage 3 — dummy load and driver acceptance
- [ ] EKM014 powered by separate approved VL 5V and VH tested at 9–12.6V; control pins LOW at boot and detached.
- [ ] Confirm its 10A shared slow fuse coordinates with up-to-6A class individual solenoids; install commercial fuse/cable appropriately.
- [ ] With current-limited dummy loads only, verify each LH/RH/LK/RK output, channel mapping, and independent safety power-off.
- [ ] Check pulse times at min/typ/max temperature and output clamp/diode energy; safe duration **must be measured**, not chosen from software defaults.
- [ ] MDD20A motor driver supply and PWM/DIR cable verified with dummy loads and independently cutoff on STOP; BMS 20A total load coordination.

## Stage 4 — controlled load acceptance / releases
- [ ] Qualified electronics + individual real coil currents/resistances measured with suitable bench supply and current limiting, within thermal/fuse envelope.
- [ ] Two brushed motors characterized at forward/reverse, start/stall and water resistance on suitable safe rig.
- [ ] Verify Radio/GUI STOP, failsafe RTH vs physical hardware fault STOP, GNSS/IMU and telemetry, waypoint/map storage, live battery sensing, and no command replay.
- [ ] Water ingress/leak/thermal and antenna location reviewed before any buoyant operation.
- [ ] Remote VER029 / Boat VER015 approval requires explicitly signed hardware acceptance; unverified automated CI/ZIP packaging is **NOT** that acceptance.

## Required evidence log
A dated bench record must include module serial/revision and cable harness revision, instrument/supply, input voltage, peak/rms current, voltage droop/overshoot, minimum and maximum pulse measured, coil temperature and physical safe-reset steps, photographs, independent cutoff actual proof, and test pass/fail. No bench evidence is currently available.
