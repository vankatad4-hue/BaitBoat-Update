# R8 Heltec GPIO feasibility: two **unapproved** 10-signal drafts

2026-11-21. Published Heltec Wireless Tracker **V1.1** schematic and pin definition
are the evidence reference; the target hardware is the as-yet-unverified **V1.2**.
Neither candidate is a firmware pin assignment, circuit acceptance, or PCB release.

Sources:
- Heltec official product / schematic download: https://heltec.org/project/wireless-tracker/
- Heltec V1.1 schematic (HTIT-Tracker_V0.5): https://resource.heltec.cn/download/Wireless_Tracker/Wireless_Tacker1.1/HTIT-Tracker_V0.5.pdf
- Heltec published V1.1 datasheet pin descriptions: https://resource.heltec.cn/download/Wireless_Tracker/Wireless_Tracker%20_1.1.pdf
- Machine-readable structural-only constraints: `Boat/tools/boat_gpio_candidate_r8.py`.

## External native requirements
| Function | Candidate A | Candidate B | Reason / incomplete proof |
| --- | --- | --- | --- |
| Independent gate permit | GPIO4 | GPIO4 | Uncontested candidate; independent power latch defaults OFF regardless of GPIO |
| Watchdog WDI | GPIO17 | GPIO17 | Separate native output, serviced only by verified healthy application cycle |
| nRF24 CSN | GPIO5 | GPIO5 | External 3.3 V input, pull HIGH while resetting |
| nRF24 CE | GPIO18 | GPIO37 | A: onboard LED load, B: DQS/flash/peripheral risk; inspect actual V1.2 PCB |
| I2C SDA | GPIO6 | GPIO6 | 3.3 V, actual pullup and breakout revision |
| I2C SCL | GPIO7 | GPIO7 | 3.3 V, shared sensor and passive coil selector option |
| MDD20A PWM_L | GPIO15 | GPIO15 | Verify ESP32 external 32k crystal is not populated/used |
| MDD20A PWM_R | GPIO16 | GPIO16 | Same 32k oscillator and boot-state check |
| MDD20A DIR_L | GPIO43 | GPIO43 | UART0 TX; debug must migrate to native USB; ensure reset pulses cannot drive propellers |
| MDD20A DIR_R | GPIO44 | GPIO44 | UART0 RX; boot-state/termination; ensure reset pulses cannot drive propellers |

**Only ten external native signals**: no Boat UART E22, no RP2040, no rudder. Signals like four coil selectors, current/STOP idle feedback require passive hardware logic or a safe verified I2C expander on the shared bus, plus an electrically independent one-hot pulse limiter. Never place actuator safety timing/WDI/permit on that expander.

### Critical external SPI check
The Tracker's onboard SX1262 uses GPIO8 NSS / 9 SCK / 10 MOSI /
11 MISO / 12 RESET / 13 BUSY / 14 DIO1 in the published V1.1 reference.
nRF24 can share **SCK9/MOSI10/MISO11** only if the V1.2 physical header
connectivity and signal-integrity are proved, all CSN outputs default HIGH
and the two radio driver stacks serialize SPI transactions. Merely seeing a
GPIO number in a datasheet does not make that net usable on a carrier.
At 3.3 V RF PA/LNA peak burst, regulator current and decoupling must be measured.

### Do not freeze these allocations
Candidate A demands proof that GPIO18 LED circuitry is compatible with
nRF CE. Candidate B demands proof that GPIO37 is not needed by onboard
flash/peripherals. Both consume GPIO15/16 and 43/44, which are
hardware/boot/debug-contested. GPIOs 4 and 17 are **relative risk
preferences**, not inherently fail-safe outputs. External passive
default-OFF pulls, a physically independent latching power cutoff and
bench boot checks are mandatory before allowing actuation.

Neither GPIO table is copied to the board's `boat_config.h` or driver
profiles. Default Boat diagnostic sketch remains disabled and pin=-1.
Source and target compilation cannot prove real header connections.

## MDD20A harness and steering constraints
Two independent 550 motors: M1 port, M2 starboard as a tentative
service-label mapping; actual motor wiring and forward polarity must
be tested. Carrier *logical* pin order PWM_L,DIR_L,PWM_R,DIR_R,GND
is NOT manufacturer MDD20A input signal order DIR1,PWM1,DIR2,PWM2,GND.
Harness net crossmapping is mandatory (see R8 pin connection CSV).
No rudder, no steering servo or servo GPIO. Autopilot source now
clips propulsion requests to nonnegative PWM before independent
motor direction inversion; manual mode may still reverse through
its own lease and reversal deadtime.

## Exit gates
1. Obtain exact physical V1.2 board/schematic, check assembly marks and all competing internal consumers; verify headers with DMM.
2. Verify native shared SPI wires and CSN/CE behavior with both radios present, first at RF receive-only and actuator supply disconnected.
3. Prove WDI startup, power-off hardware watchdog latch, mechanical STOP, undervoltage and overcurrent independent of firmware.
4. Characterize both motors, coil current and 3S protection; freeze connector orientation and fuse ratings.
5. Achieve native KiCad ERC/PCB DRC and safe dummy-load commissioning; only then assign GPIO and consider active firmware. Current R7 schematic still has 15 ERC violations; no assertions of zero-ERC here.

### Automated harness crossover checks
`Boat/tools/mdd20a_harness_r8.py` plus host unit test now reads the R7 KiCad J2 connector's five **actual net labels** and enforces `PWM_L,DIR_L,PWM_R,DIR_R,GND`. It rejects same-order/incorrect Cytron vendor-label mating in favor of the explicit `PWM1,DIR1,PWM2,DIR2,GND` crossmap in `Boat/docs/MDD20A_CARRIER_CROSSOVER_R8.csv`. This validates only logical signal mapping; the vendor's physical socket order and motor winding polarity are unverified, and no coil/motor power is authorized.

### R8 differential-thrust / pin-risk / harness test verification — 2026-11-21
- Autonomous differential motor mixer now **forward-only** (no counter-rotating automatic pivot), while manual TANK signed reverse and joystick override remain separate supervised paths. Change SHA `ad487c77128fadd54d333362d36ab8eceb6a0855`: Boat target ESP32-S3 diagnostic build **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868652840; Boat host test **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868652827.
- Two structural-only Heltec V1.2 GPIO candidate plans (10 native signals; WDI17, permit4; nRF CE18 or 37) and a manufacturer-label MDD20A crossover harness check committed. No firmware pin values were assigned or actuators enabled. See `Boat/docs/BOAT_GPIO_PIN_STUDY_R8.md`, `Boat/docs/MDD20A_CARRIER_CROSSOVER_R8.csv`.
- GPIO candidate and host regressions **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868795058 (`9e7ed85`). Initial new KiCad J2 label parser matched J5 feedback labels as well, so the first harness regression `0d9e527` failed; corrected by bounding the connector's y-coordinate (`122141d`), and added `Boat/tools/**` to CI triggers (`fe08663`). Latest Boat host CI **PASSED**, https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868963948, on `fe08663af3174003eedebddac0cb7eb1f8b9b682`.
- This is **source, offline-test and documentation evidence only**. Heltec exact V1.2 pin access/boot states and nRF/SX1262 shared SPI, watchdog latch, independent four-coil hardware and motor orientation are not physically verified. R7 KiCad still has 15 ERC errors; no fabricable high-current gate, coil sheets, PCB or hardware acceptance. Integrated Boat SX1262 backup remains intentionally disabled because Remote E22 RF interoperability is unverified. Sonar remains on hold, no hardware/software selected.
- Official Remote VER028 and Boat VER014 retained; VER029/VER015 remain withheld.
