# R7 — Heltec-only MDD20A feasibility and preliminary native schematic

**Architecture decision:** retain one Heltec Wireless Tracker ESP32-S3, nRF24 primary, external E22 backup, 4.3" Waveshare Remote, Cytron MDD20A dual independent 550 motors, four independently wired hoppers/hooks. **Do not reintroduce RP2040.** Sonar model remains undecided; absolutely no sonar GPIO, firmware, power or protocol integrated.

## Verified accomplishments
- Boat diagnostic #22 target build passed before R7; no new actuator GPIO enabled. Host CI R7 GPIO and power/cutoff models passed in later Boat runs. The hardware model itself is NOT commissioned.
- Native editable KiCad control/watchdog draft: `Boat/hardware/Direct_MDD20A_R7/Boat_MDD20A_Control_R7.kicad_sch`. The schematic embeds its own symbols: Tracker logic header, MDD20A harness, watchdog/cutoff interface, four solenoid request signals, local measured feedback header, TPS3431 (pins 1–9), 10 kΩ open-drain pull-ups, provisional 150 pF C0G timing cap and 100 nF bypass. All MDD20A harness pins are **carrier-defined logical order only**, not a claim of manufacturer connector positions.
- GitHub KiCad workflow installs KiCad 9 CLI, structurally checks the file, exports a netlist and produces an ERC report. Import/netlist export is not ERC acceptance. The first report showed **83 violations**. Full PCB/high-current cutoff, regulator, load-switch, MOSFET driver and copper layout are still missing; this single control sheet must not be manufactured.
- The independent safety cutoff truth-table model checks 64 fault combinations and mandates physical STOP, watchdog, UVLO, overcurrent, thermal and rail validity **all** healthy; a fault latches the gate off and simply restoring the signal cannot rearm it. This is an engineering requirement; it is not physically implemented by software.
- The peak-current budgeting script intentionally fails without actual motor stall current, the highest coil current, 5 V logic peak draw and converter efficiency. It assumes one active coil, and explicitly does not sum motor and solenoid loads simultaneously. A real hardware interlock must enforce that policy; BMS 20 A marking alone does not certify startup or short-circuit protection.

## Exact GPIO feasibility — blocked

| Native ESP32 signal demand | Count |
|---|---:|
| nRF24 CE and CSN | 2 |
| E22 UART TX/RX (M0/M1/AUX can be routed via a slow expander after validation) | 2 |
| I²C sensor and optional expander SDA/SCL | 2 |
| MDD20A left/right PWM and left/right DIR | 4 |
| Independent watchdog WDI + actuator permit | 2 |
| **Native minimum** | **12** |

Provisional V1-family accessible candidates from published V1.1 pinout and community-identified V1.2 pin definitions: GPIO 4, 5, 6, 7, 15, 16, 17, 18, 37, 43, 44 = **11**. This is **at least one GPIO short even if every tentative pin is safe**. GPIO15/16 may conflict with low-frequency oscillator usage, 18 with LED, 37 with flash/ADC/GNSS control, 43/44 with UART0 console. GPIO4–7 are also capacitive touch capable; confirm any fitted circuits. This is **not a signed V1.2 board allocation**. A conservative list that excludes those contested pins has only five.

A mere MCP23017 for coil commands and measured idle feedback does **not** solve the 12-vs-11 native minimum. Candidate non-programmable I/O redesigns to evaluate (neither selected):
- Move external E22's UART to an I²C UART bridge; the primary application radio driver must be replaced/tested, and interrupt latency, buffer and failsafe behavior budgeted. Count-only minimum decreases from 12 to 10.
- Place two MDD20A DIR signals on a verified I²C GPIO expander (native motor PWM and WDI/permit stay direct). Count-only minimum also decreases to 10, **but** frozen direction on bus fault is hazardous: independent power cutoff, readback, boot pulldown, one-way turn-safe policy and timeouts mandatory. This is not equivalent to the original direct native PWM/DIR assumption.
- Alternative onboard SX1262 backup could reduce E22 wiring, **but is not authorized**: owner explicitly chose E22 backup and did not request changing radio models.

Even a count-only pass would not guarantee a legal functional mux, safe boot state, voltage domains, CPU/serial performance or enough physical feedback pins. An approved GPIO expansion strategy and exact V1.2 schematic/board inspection are required before native wiring freeze. **No fixed pins have been asserted in active firmware.**

## Watchdog electrical calculation and limitations
TPS3431 TI equation with ideal CWD=0.15 nF:
`tWD(ms)=77.4*CWD(nF)+55 ≈ 66.61 ms`.
With a 5% capacitor and TI's 0.905 minimum / 1.095 maximum timing factors, approximate bounds are **59.8–73.6 ms**. The true **energy cutoff at motor and coil terminals** must be measured and meet the project's 100 ms design target; WDO assertion alone is insufficient. WDO is an open-drain active-low output which can reassert HIGH after its reset pulse: feed a **physical fault latch** that stays disabled until an independent deliberate rearm. The STOP, undervoltage, overcurrent and thermal signals must revoke the gate *independently of* CPU/software. Do not pet WDI from PWM timer, ISR, DMA or unrelated heartbeat. Retain the software 200 ms manual lease as a distinct and much weaker layer.

## Power sizing gate
Reported 3S4P Li-ion/BMS 20 A, but cell specification, actual pack discharge curve, motor 550 stall currents, MDD20A thermal load, coil current, fuse/connector/wire and BMS response need measurements. The power budget models simultaneous **two-motor peak** vs **one-solenoid peak**, both plus independently supplied 5 V electronics. Unmeasured inputs block selection. Keep a separate logic regulator, protected/filtered RF rails, and physically separated motor/coil current paths. Never connect the boat's 3S pack directly to Tracker's 1S battery charger connector. Future sonar supply has **no budget reserved yet** because the model is unselected.

## Evidence and next step
- Heltec vendor V1.1 docs (not V1.2 certification): https://resource.heltec.cn/download/Wireless_Tracker/Wireless%20Tracker1.1.pdf
- Independent V1.2 community board definitions: https://github.com/jbjardine/HTIT-Tracker-v1.2-forIDF/blob/main/components/board/board_pins.h
- Cytron MDD20A datasheet: https://cdn.robotshop.com/media/c/cyt/rb-cyt-287/pdf/cytron-20a-6v-30v-dual-dc-motor-driver-datasheet.pdf
- TI watchdog TPS3431: https://www.ti.com/lit/ds/symlink/tps3431.pdf
- Source of GPIO constraint checks: `Boat/tools/gpio_feasibility_r7.py`, `Boat/docs/BOAT_GPIO_CANDIDATE_MATRIX_R7.csv`.
- Source of power, fault latch spec: `Boat/tools/power_budget_r7.py`, `Boat/tools/safety_gate_spec_r7.py`.
- Sonar: `Boat/docs/BOAT_SONAR_INTEGRATION_PLAN_UNDECIDED_R1.md`; planning-only, unchanged.

Next engineering order: obtain **authoritative V1.2 wiring** or inspect the exact board; select the least-risk non-programmable GPIO/serial expansion method; assign and validate every required GPIO, I²C address and radio CS; capture the complete high-current power, gate latch and four coil drivers in KiCad with real footprints; iterate KiCad ERC until all required errors are resolved; then layout, load/cutoff bench measurements and physical commissioning. Releases Remote VER029 / Boat VER015 remain blocked. Current markers Remote 028 / Boat 014.


### R7 completion checkpoint — 2026-11-21

**Active architecture unchanged:** single Heltec Tracker + external Cytron MDD20A, nRF24 primary/E22 backup, four independent solenoid channels, no RP2040. Sonar model explicitly **undecided**, sonar code/wiring **not implemented**.

**Verified software and project checks:** Boat host #59 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714160) PASSED all existing portable regressions plus GPIO feasibility, pack-current safety and gate truth-table tests (24 groups by workflow stages). ZIP packaging #1103 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866714149) PASSED for source SHA 358ed10687fbd09606aafbaf18a730cc0ef80e49. Boat target #22 remains last verified ESP32-S3 diagnostic source build; subsequent commits modify tests, docs, and preliminary CAD only. Remote target #87 remains last verified Remote build. Official Remote VER028 and Boat VER014 unchanged; 029/015 withheld.

**Native preliminary KiCad progress:** KiCad workflow #6 (https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44866942940, source e2ec4f6c8cfc3852c78d1f475a7f57af1c7a6ed4) PASSED parser/netlist export and geometry/project-symbol checks after adding project-local `Flytec.kicad_sym`, `sym-lib-table` and `.kicad_pro`. Unresolved ERC violations reduced from 83 to 26 after grid alignment, then to **15** after project library setup (13 unimplemented release/feedback net connections and 2 unpowered TPS3431 power pins). **ERC production acceptance FAILS/NOT COMPLETE**. Remaining physical power gate, watchdog fault latch, high-current motor/coil circuits, fuse/protection, final footprints, true V1.2 pin assignment and PCB are not captured. A successful *preliminary* KiCad check must never be represented as zero-violation ERC.

**Verified GPIO shortfall:** provisional best-case pin pool 11, minimum native demands 12 with both external radios retained; even this assumes contested oscillator/LED/UART/flash pins are available. New active `Boat/tools/gpio_feasibility_r7.py` and matrix explicitly block unsafe assignments. I²C-UART bridge or direction expander remain optional proposals, neither selected or implemented. All physically driven GPIOs remain unassigned/disabled. No hardware or physical acceptance evidence; no sonar model or implementation. Next: decide a safe extra I/O topology after exact Heltec V1.2 pin documentation, fill unresolved nets/rails in native KiCad, verify a separately latched electrical cutoff, run zero-violation ERC and physical commissioning before releases.
