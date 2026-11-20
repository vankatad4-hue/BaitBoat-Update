# Flytec V080 bait boat and remote — master project record R2

**Consolidated:** 2026-11-20 (schematic design update). Repository source is canonical; latest schematic capture specification is documented below.

## Archive versioning
ZIPs use Flytec_Remote_verXXX.zip and Flytec_Boat_verXXX.zip. Never overwrite a prior release. ver005 archives exist but are not fully synchronized to current repository work.

## Project
Retain Flytec V080 hull, motors, battery indicator and main switch; replace boat control electronics. Remote is a new handheld design using existing Waveshare 4.3 touchscreen board.

## Boat baseline
- 12V 550 brushed motors; motor startup/stall current unmeasured.
- Reported pack 3S4P, BMS 20A; verify actual cell/pack data.
- Solenoids FW0630 009 DC12V bait and FW0630 001 DC12V hook; coil current/resistance/duty not measured.
- Carrier PCB envelope 77×79×14mm, compartment lid clearance ~30mm.
- Heltec Wireless Tracker V1.2 ESP32-S3/UC6580 GNSS candidate.
- GY-91 MPU-9250 + BMP280 motion/heading candidate; exact breakout/pinout and calibration pending.
- nRF24L01+ PA/LNA 250kbps initial; 500m range goal unvalidated.
- Cytron MDD20A motor-driver candidate; confirm motor current before use.
- Sonar: full CHIRP echogram/fish arches desired, module/interface still unselected.
- Autopilot mission: save waypoint, send, release bait/hook, return; failsafe return-home only with valid navigation/system health.

## Remote schematic design progress (2026-11-20)
- GPIO/peripheral assignments checked against official Waveshare online docs; PORT1 physical pin-number crosswalk remains provisional due schematic/document discrepancy. Confirm exact board revision/orientation before wiring; see `WAVESHARE_HEADER_MAPPING_AND_PIN_ASSIGNMENTS_R2.md`.
- Capture-sheet design and net-level drafts created: `SCHEMATIC_CAPTURE_SHEETS_R1.md`, `SCHEMATIC_NETLIST_BLOCKS_R1.md`, `SCHEMATIC_CAPTURE_CHECKLIST_R1.md`.
- Battery protection/power branches specified; controller power via onboard CS8501 boost, external TPS61023 fallback only; radio TPS63070 dedicated rail.
- Controls: ADS1115 @0x48 for four joystick axes, MCP23017 @0x20 for buttons, second ADS1115 @0x49 for battery/NTC/optional radio rail monitoring, MCP23017 @0x21 for radio CE/CSN/IRQ candidate.
- Shared SPI GPIO11/12/13 with onboard TF card; radio priority and bus serialization required.
- Expander-based radio control remains an explicit validation gate; no native KiCad capture/ERC/bench test has yet been performed.

## Remote baseline
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480; onboard ESP32 antenna.
- 2 joysticks, 16 buttons (2×4 banks on each side), touch UI; ergonomic remote concept remains.
- Pack 1S4P 18650; exact cells and BMS not selected.
- nRF24 PA/LNA module with external antenna. Wi-Fi/Bluetooth coexistence requires testing.

## Revised remote power tree
- Pack/BMS output → fuse candidate → reverse-protection P-MOS candidate → BAT_PROT.
- Controller/display 5V: use Waveshare onboard CS8501 battery boost via single-cell PH2.0 connector. Manufacturer documentation says boost from single cell to 5V and board consumption 5V/450mA. Verify current/transient/thermal performance. External TPS61023 remains fallback only; do not parallel converters.
- Radio 3.3V: dedicated TPS63070 buck-boost. 1.5µH candidate; feedback 470k/150k, nominal 3.3067V; effective capacitors and configuration pending tests. EN candidate controlled after boot, default OFF.
- Protection shortlist: Littelfuse 0458004.DR 4A fast-acting fuse; onsemi FDS6575 P-MOS (Drain pack side, Source protected output); battery ADC divider and NTC. Parts/coordination not validated.
- Current estimate at 3V loaded: controller 5V/450mA at assumed 90% ≈0.83A battery; radio 3.3V/300mA at assumed 90% ≈0.37A; total ≈1.20A. Estimate only.

## Remote controls/peripherals
- Board I2C GPIO8/9 shared with touch/CH422G/RTC.
- ADS1115 candidate (I2C 0x48) for four joystick axes, 16-bit, up to 860SPS aggregate; verify latency/noise.
- MCP23017 candidate (I2C 0x20) for 16 button inputs; active-low contacts and software debounce.
- Battery sensing GPIO6/Sensor-AD candidate.
- nRF SPI candidate GPIO11/12/13 shared with TF card; separate CS. Candidate CSN GPIO15 and CE GPIO43 are provisional; GPIO attachments and header exposure must be verified. IRQ omitted/polled initially.
- I2C addresses/pullups, SPI sharing and GPIO map not frozen.

## Engineering status
Power and interface documentation updated. Native KiCad schematics, ERC, PCB layout, fabrication BOM, exact pin/connector assignment, bench validation and hardware bring-up are still not completed. Do not treat docs/net drafts as fabrication-ready.


## New radio redundancy decision — 2026-11-20
- **Added LoRa as backup communications**, alongside nRF24L01+ PA/LNA primary. Both remote and boat must carry matching LoRa modules for the backup link.
- Candidate module: EBYTE E22-900T22S-V2 UART/SX1262 EU868-capable family; exact SKU/configuration is not finalized or purchased. UART pins remain unassigned until GPIO/header audit.
- Remote antenna ports reassigned: RIGHT = EU868 LoRa; LEFT = nRF24 2.4GHz. ESP32 Wi-Fi/Bluetooth uses the Waveshare board's onboard antenna; no external Wi-Fi antenna connector planned.
- LoRa is for heartbeat, low-rate telemetry, reduced-rate/manual safety control, STOP and RETURN. It will not carry sonar streaming. Mission start and bait/hook release are disabled over backup by default.
- Firmware requirements: dual-link LinkManager, primary/backup/no-link UI, handover hysteresis, duplicate command suppression, packet freshness/sequence checks, authenticated/replay-protected commands, independent boat failsafe. Failover must be bench/field tested.
- LoRa module power is provisionally from Waveshare CS8501 5V rail with local bulk/decoupling; simultaneous display/backlight+nRF+LoRa load/thermal test required. If margin inadequate, design a separate boost branch (never parallel boosts).
- EU868/Bulgaria channel, radiated power, duty cycle, airtime and antenna gain/cable loss compliance must be verified; module's 22dBm capability is not an approved setting.
- Design record: Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R1.md. Capture/net/power/BOM documents have LoRa addenda. No firmware code has yet been merged or bench-tested, and no hardware purchased.

