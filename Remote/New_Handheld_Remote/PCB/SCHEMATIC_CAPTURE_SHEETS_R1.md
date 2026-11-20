# Remote Schematic Capture Sheet Design R1

**Status:** Detailed capture-ready sheet specification. This is not yet a native KiCad .kicad_sch file, ERC report, or hardware-tested circuit. All symbols, exact pin numbers, footprints, and datasheet revisions must be checked during KiCad capture.

## Sheet 00 — System Overview / hierarchical sheet plan
Power source: 1S4P protected Li-ion pack, nominal 3.6/3.7V, 4.2V max. Pack/BMS → fuse → reverse polarity MOS → BAT_PROT. BAT_PROT branches to (1) Waveshare ESP32-S3-Touch-LCD-4.3 Standard PH2.0 battery input and onboard boost/charge-management circuit; (2) TPS63070 radio supply. Controller I2C GPIO8/9 feeds ADS1115 x2 and MCP23017 x2; SPI GPIO11/12/13 shared between onboard TF card and radio. All grounds common. Controller display board and its onboard battery circuit remain intact.

## Sheet 01 — Battery input, protection and monitoring
### Connector and protection path
- J_PACK: keyed connector, 2 pins; pin 1 PACK+, pin 2 PACK−. Exact connector and rating TBD; select ≥4A continuous with adequate margin and keyed polarity.
- PACK− → GND.
- PACK+ → F_MAIN → PACK_FUSED.
- F_MAIN candidate Littelfuse 0458004.DR, 4A fast-acting 1206; confirm I²t, DC interrupt rating, derating, footprint and pack/BMS coordination before freeze.
- Q_REV candidate FDS6575 P-channel SO-8: Drain=P PACK_FUSED; Source=BAT_PROT; Gate=GATE_REV. Verify package pin map and body diode from manufacturer datasheet. Candidate topology based on reverse-polarity application reference; schematic orientation must be checked symbolically.
- R_GATE 100k from GATE_REV to GND. For the low-voltage 1S pack, add no gate clamp unless analysis demonstrates need. Add test points TP_PACK_FUSED and TP_BAT_PROT.
- C_BAT bulk: 100µF low-ESR plus 10µF and 100nF BAT_PROT-to-GND near converters (values candidate; check converter stability and connector transients).
- No TVS selected: 1S voltage is low; avoid unnecessary clamp/leakage. Review cable ESD/inductive threats separately.

### Controller battery branch
- BAT_PROT/GND → J_WAVE_BAT, mating PH2.0 2-pin harness to Waveshare board battery input.
- Pin 1 positive BAT_PROT, pin 2 GND; confirm physical connector polarity and board revision with meter before plugging.
- The Waveshare onboard CS8501 circuit supplies board 5V from single-cell pack. Do not add TPS61023 on this baseline; never parallel boost outputs.
- USB charging/back-power interactions are not part of our custom circuit; document board behavior and test with pack attached before permitting USB power/charging.

### Battery voltage sense
Do not use GPIO6/Sensor AD without tracing its onboard circuit. Use a second ADS1115 (U_ADC2, address 0x49, ADDR→3V3) shared I2C, AIN0=BAT_SENSE, AIN1=NTC_SENSE, AIN2=RADIO_SENSE optional, AIN3 spare. Divider: R_BAT_TOP=220k 1%, BAT_PROT→BAT_SENSE; R_BAT_BOTTOM=68k 1%, BAT_SENSE→GND; C_BAT_SENSE=100nF BAT_SENSE→GND. At 4.2V nominal ADC input ~0.977V; at 3.0V ~0.698V. ADC powered from 3V3; all measurements only while controller powered. Verify ADS1115 input/common-mode and absolute maximum; input must not exceed its rails.
- Battery divider continuous current at 4.2V ≈14.6µA.
- Optional battery monitor filtering/sampling handled in firmware.

### Pack temperature
- J_NTC: two pin harness to pack-mounted 10kΩ NTC, nominal B=3950 candidate; pin 1 NTC_SENSE, pin 2 GND.
- R_NTC_PULLUP=10k 1% from 3V3 to NTC_SENSE; NTC from NTC_SENSE to GND; C_NTC=100nF to GND.
- NTC_SENSE routed to U_ADC2 AIN1. Detect open/short in firmware; confirm exact thermistor curve and placement/insulation. Do not place sensor directly on exposed cell terminals.

## Sheet 02 — Controller power interface
- J_WAVE_BAT is a PH2.0 two-pin connector, board battery polarity verified against exact revision before connection.
- BAT_PROT to positive; GND to negative.
- Onboard CS8501 is used for cell charging/boost path. Manufacturer describes single-cell battery and boosted 5V, approx 580mA charging current and board load listed 5V/450mA. These are source specifications, not proof of transient margin for this build.
- External TPS61023 is fallback only after test. No parallel boost.
- Add test points TP_WAVE_BAT+ and TP_WAVE_GND on harness/PCB. Bench test 5V rail startup, display/backlight peak, radio simultaneous load, ripple, thermal, battery cut-off and USB interaction.

## Sheet 03 — Dedicated RADIO_3V3 buck-boost
U_RADIO TPS63070 (verify exact package/pin numbering from current datasheet and footprint):
- VIN pins → BAT_PROT.
- GND and PGND → GND plane.
- L1/L2 pins → L_RADIO 1.5µH XFL4020-152MEC candidate.
- VOUT pins → RADIO_3V3.
- FB → RADIO_FB; R_TOP=470k from RADIO_3V3 to FB; R_BOTTOM=150k FB→GND; nominal ~3.307V per existing calculation, recheck against datasheet reference.
- VSEL→GND candidate for selected output configuration; FB2→GND candidate; PS/SYNC strap candidate; verify current datasheet pin treatment before capture.
- VAUX 100nF to GND local.
- CIN: 2×10µF 25V X7S 0805 + 10µF 25V X5R 0603 candidates; COUT: 3×22µF 16V X6S 0805 + 10µF 0603 candidates. Calculate effective capacitance under DC bias, stability, ripple and layout requirements.
- EN=RADIO_EN. MCU control TBD; default off via 100k pulldown to GND and 1k series candidate only after GPIO source is chosen. Ensure EN not driven by unpowered controller.
- PG open-drain optional RADIO_PGOOD; do not connect until GPIO/pullup plan approved.
- J_RADIO rail decoupling: 100nF + 10–47µF near module; radio connector supply from RADIO_3V3 only, never 5V.
- Test points TP_RADIO_3V3, TP_SW nodes only if safe/appropriate, TP_RADIO_EN.

## Sheet 04 — I2C analog joystick interface
I2C nets from Waveshare PORT1: GPIO8=I2C_SDA; GPIO9=I2C_SCL. Shared with onboard GT911 touch, CH422G, RTC and other board I2C circuitry. Confirm bus scan, voltage, pullups and rise time. Do not blindly install additional pullups.
- U_ADC1 ADS1115 @0x48 (ADDR→GND), 3V3/GND, 100nF bypass.
- AIN0 JOY_LX, AIN1 JOY_LY, AIN2 JOY_RX, AIN3 JOY_RY.
- Each axis connector J_JOY_L/R: 1=3V3, 2=GND, 3=X, 4=Y, 5=NC/optional switch not used.
- Each axis wiper → 1k series → ADS AINx; 1µF AINx→GND after series resistor. Candidate RC corner ~159Hz; verify source impedance and ADC acquisition/config. Axis potentiometers nominal 10k from 3V3 to GND.
- ADS1115 single-ended read mode; PGA setting to cover 0–3.3V with margin (±4.096V setting); sample/conversion schedule, axis calibration and control latency in firmware. ALERT/RDY NC initially.
- Add connector pin-1 marking/keying and test pads for all four analog channels.

## Sheet 05 — Button bank expander
- U_BTN MCP23017 @0x20; A0/A1/A2→GND; VDD=3V3, VSS=GND; 100nF bypass at IC.
- RESET pulled to 3V3 via 10k; do not leave floating.
- GPA0–GPA7 = left button bank; GPB0–GPB7 = right bank. Each NO button connects pin to GND, active-low. Enable internal pullups; external pullups not baseline.
- INTA/INTB NC initially; polling 50–100Hz with 10–20ms initial debounce candidate.
- J_BUTTONS harness pin assignment and physical button labels are firmware/UI mapping to be established; connector should expose individual signal+common wiring or matrix only if user approves matrix architecture (current design direct inputs).
- Address scan and simultaneous button behavior to test.

## Sheet 06 — Radio control expander (VALIDATION REQUIRED)
- U_RADIO_IO MCP23017 @0x21; A0=VDD, A1/A2=GND; verify address strap combination against datasheet; I2C shared.
- VDD=3V3, GND, 100nF local; RESET tied to shared reset net with 10k pull-up (or reset sequencing documented).
- GPA0=RADIO_CE, GPA1=RADIO_CSN, GPA2=RADIO_IRQ_N; remaining I/O spare.
- CE and CSN outputs; IRQ input. IRQ_N active low, pull-up only if required after module/bus check. Polling baseline; interrupt output optional but not currently routed to ESP GPIO.
- **Not approved for final radio firmware yet:** I2C expander toggles may create millisecond latency. It may be sufficient for CE mode selection and CSN framing, but test register transaction timing, CE pulse widths, packet TX/RX, IRQ polling, retry/ACK behavior and 250kbps radio throughput. If tests fail, choose a verified spare MCU GPIO or alternate control strategy; do not assume GPIO35–37 are free.

## Sheet 07 — Shared SPI radio and TF
- ESP GPIO11=SPI_MOSI, GPIO12=SPI_SCK, GPIO13=SPI_MISO from PORT1 pins 19/20/21.
- Radio CSN from U_RADIO_IO GPA1; CE from GPA0; IRQ_N from GPA2.
- TF card uses onboard CH422G EXIO4 for SD_CS; do not duplicate/drive this net from custom PCB.
- J_RADIO 8-pin: 1 GND; 2 RADIO_3V3; 3 SCK; 4 MOSI; 5 MISO; 6 CSN; 7 CE; 8 IRQ_N. Confirm exact module breakout pinout before wiring.
- SPI initially 1–4MHz. Use software bus mutex/serialization, radio priority, and short SD transactions. Test SD card and radio together; confirm CS isolation and no cross-talk.
- Place radio connector at RF-edge with antenna keepout; separate PA/LNA supply return from joystick analog ground paths; solid ground plane with local supply decoupling. Do not place copper/metal/large display flex under antenna as module guidance permits.
- RADIO_3V3 local 100nF + 10–47µF; use short, wide supply/ground paths. Validate burst current and rail droop.

## Sheet 08 — Test points and assembly
TPs: GND, PACK_FUSED, BAT_PROT, WAVE_BAT+, RADIO_3V3, RADIO_EN, I2C_SDA, I2C_SCL, SPI_SCK, SPI_MOSI, SPI_MISO, JOY_LX, JOY_LY, JOY_RX, JOY_RY, BAT_SENSE, NTC_SENSE.
Connector selection, pin numbering, footprints, mounting and mechanical clearance remain open. Include no unverified harness/connector ratings.

## Design gates
1. Confirm exact Waveshare revision, PH2.0 polarity and USB/charger behavior.
2. Validate pack/BMS/fuse/MOS/connector current ratings and thermal.
3. Confirm TPS63070 datasheet pin configuration and effective capacitance/inductor saturation.
4. I2C scan: onboard and added addresses, bus pullups, waveform rise time.
5. ADC acquisition and joystick loop latency.
6. MCP23017 radio-control latency/timing and RF performance; revise if needed.
7. Shared SPI/TF-card coexistence.
8. Native KiCad capture, ERC, footprint/pin audit, independent review.
9. Bench bring-up before PCB release.
