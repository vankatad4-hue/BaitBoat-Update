## Project archive versioning rule

- Every distributed project ZIP must use the filename format `Flytec_Remote_verXXX.zip`.
- The initial release in this naming scheme is `Flytec_Remote_ver001.zip`.
- Every modification to any project file that changes the project deliverable increments `XXX` by exactly 1 (for example: `ver001` → `ver002`).
- Never overwrite an earlier versioned ZIP. Keep each release as a separate archive so previous versions remain recoverable.
- The version number in the ZIP filename is the authoritative project release number; update this rule's examples or release notes only when needed, not the naming format.

# Flytec V080 bait boat and remote — master project record

**Last consolidated:** 2026-11-16  
**Purpose:** Carry forward the agreed hardware, UI, navigation, safety and implementation decisions between future development sessions. This file is the detailed project record; `README.md` is the quick-start and current-code index.

## 1. Overall project goal

Retrofit the existing Flytec V080 bait boat while retaining its hull, two 550 brushed motors, battery indicator and main power switch. Replace the original boat control electronics with a new central custom PCB using removable connectors. Retain the original Flytec remote enclosure but replace its internal electronics with a Waveshare ESP32 touchscreen controller.

The intended mission is: manually drive to a chosen fishing location, save it as a waypoint, select it later, navigate there autonomously, release bait and hook, then return home. The operator must also be able to take manual control while autopilot is active. The two agreed override behaviours are selectable: temporary manual override with autopilot resume, or switch fully to manual mode.

## 2. Boat-side hardware decisions and known measurements

| Item | Current project information |
|---|---|
| Hull | Existing Flytec V080 retained |
| Motors | Existing standard 550 brushed motors retained; 12 V design target |
| Battery | Reported 12 V, 3S4P Li-ion pack, 2500 mAh, BMS rated 20 A; verify pack label and actual wiring before final power design |
| Existing controls | Original boat electronics still installed and operational as of last inspection; final retrofit replaces the control system |
| Main switch / battery indicator | Retain existing parts |
| Bait/hook actuators | Solenoids desired; bait solenoid FW0630 009 DC12V; hook solenoids FW0630 001 DC12V |
| Boat PCB envelope | 77 × 79 × 14 mm measured usable area; approximately 30 mm from compartment floor to inside of lid |
| PCB mounting holes | Not measured yet |
| Motor current / stall current | Not measured yet; must measure before selecting MOSFETs, connectors, copper and protection |
| Solenoid coil current | Not measured yet; must measure before final driver and flyback design |
| ESP32 | User owns a 38-pin ESP32 development board; carrier-style mounting was preferred |
| Excluded item | DHT11 is not part of this project |

The user previously replaced one original motor MOSFET with a DMTH3002LK3-13 and reported similar/same-size MOSFETs used for motor and solenoid drivers. This is historical information, not a validated new-board component selection. Do not copy the old MOSFET choice without checking voltage, current, gate drive, thermal and transient requirements.

## 3. Boat navigation and sensors

- **Selected boat controller/GNSS board: Heltec Wireless Tracker V1.2**, using its ESP32-S3FN8 processor and integrated UC6580 GNSS. This replaces the planned generic 38-pin ESP32 carrier and separate HGLRC M100 PRO B101 GNSS module. Exact V1.2 pin map, GNSS power control, UART routing and antenna configuration must be verified before wiring.
- **Selected motion/heading sensor: GY-91-style MPU-9250 + BMP280 module.** MPU-9250 provides 3-axis accelerometer, gyroscope and magnetometer; it replaces the separate QMC5883L compass and MPU6050 proposal. BMP280 provides atmospheric pressure and temperature only, not underwater depth. Verify the exact breakout-board revision, chip markings, voltage and I²C address.
- The Heltec board also contains SX1262 LoRa, but the selected boat/remote radio remains external nRF24L01+ PA/LNA. Leave SX1262 unused.
- Mount and calibrate the MPU-9250 away from motors, solenoids, high-current wiring and steel; validate heading with propulsion off and running. No GPIO pin assignments or sensor drivers are approved/implemented yet.
- GPS telemetry should include latitude, longitude, fix type, satellites, horizontal accuracy, speed, heading, sequence/timestamp and integrity check.
- Compass work must include calibration, mounting-axis correction and magnetic interference checks with motors and power wiring active.
- **Selected radio:** matching nRF24L01+ PA/LNA modules on remote and boat at 2.4 GHz. Initial test data rate is 250 kbps. The 500 m mixed-environment range remains unverified. Exact module variant, antenna, channel, GPIO assignments, power rail and compliant EIRP remain to be selected/verified. This supersedes the earlier SX1278 433 MHz and SX1262 868 MHz LoRa proposals.
- If radio is lost during autopilot, the preferred boat-side behaviour is return home, subject to valid GNSS, valid compass, safe home point and boat-side watchdog/failsafe checks. This policy is not implemented in the current remote firmware.

## 4. Remote hardware and firmware target

- Original Flytec V080 remote enclosure retained; replace internal electronics.
- Preferred display/controller: Waveshare ESP32-S3-Touch-LCD-4.3 Standard, SKU 25948, 800 × 480, ST7262 RGB LCD, GT911 touch, CH422G I/O expander, 16 MB flash and 8 MB OPI PSRAM.
- Target framework: Arduino IDE with LVGL 9.6.0 (9.6.x patch releases only for this project baseline).
- The display board had not yet been purchased at the time of the latest project planning. Firmware has not been tested on the physical Waveshare board.
- User wants complete source files with explanatory comments, and prefers incremental steps with compile/test feedback between stages.
- Original display cavity dimensions: 110 × 67 × 7 mm. Candidate Waveshare display dimensions discussed: 106.8 × 67 × 8 mm; verify mechanical fit with the actual board, bezel and connectors.
- Remote battery idea: two 18650 1200 mAh cells in parallel (1S2P, approximately 2400 mAh) with USB-C charging. Waveshare documentation was noted as recommending a capacity below 2000 mAh for its battery connector; this pack exceeds that recommendation. Verify the board charger/protection design and battery arrangement before connecting. Do not parallel separate charger outputs.

### Waveshare board resource constraints

- RGB LCD: GPIO 5 DE, 3 VSYNC, 46 HSYNC, 7 PCLK; RGB data pins GPIO 1/2/42/41/40, 39/0/45/48/47/21, 14/38/18/17/10.
- GT911: GPIO4 IRQ, GPIO8 SDA, GPIO9 SCL; CH422G EXIO1 touch reset.
- CH422G EXIO2 backlight; EXIO4 TF-card chip select; EXIO5 USB/CAN selection.
- TF SPI: GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO.
- GPIO8/9 are shared I²C. GPIO15/16 are multiplexed RS485/CAN. GPIO19/20 are USB D-/D+.
- Do not assign these occupied pins to external joystick ADC, radio, buttons or other functions.
- The present code uses CH422G for backlight ON/OFF only. Variable brightness is not implemented; PWM control needs verified board-level hardware support.

## 5. Remote UI decisions

- 4.3-inch display shows map and sonar simultaneously in an equal split while driving, in both manual and autopilot modes.
- Optional full-screen map or full-screen echogram with a return control is desired.
- Sonar presentation: advanced, full CHIRP-style echogram with fish arches drawn on this same 4.3-inch display. User rejected a hybrid sonar / second-screen arrangement.
- Navigation preferences: boat-up orientation; route and approach display both enabled; arrival radius previously set to 3 (unit not yet confirmed); map style should support both standard/street and satellite; home point visible.
- Manual controls: tank/steering motor control selectable; separate switching; waypoint naming.
- UI prototype work in LVGL Pro Editor used Bulgarian labels and a main screen with map/sonar panels and Manual/Autopilot/STOP controls. That XML prototype is separate from the Arduino firmware source in this archive. The C++ firmware UI currently uses English labels.

## 6. Remote Settings screen and virtual keyboard

Current source modules: `settings_ui.h/.cpp`; main screen opens Settings through the header button.

Implemented in source:
- Category navigation: Wi-Fi, Display, Controls, Sonar, Map, System.
- LVGL virtual keyboard for Wi-Fi SSID and password fields.
- Wi-Fi scan request, SSID/password entry and connection request.
- Wi-Fi credentials and preferences stored in ESP32 Preferences/NVS.
- Display backlight ON/OFF using CH422G EXIO2.
- Day/night preference and screen-timeout preference stored, but not yet applied as a full theme engine or automatic dimming feature.
- Sonar profile saved locally in NVS: depth range, gain, fish sensitivity, noise rejection, ping-rate preference, echogram scroll speed, palette, fish arches/symbols, fish alarm, shallow/deep alarms and their thresholds.

Limitations:
- Sonar controls are only local preferences. There is no selected CHIRP module control API or radio settings packet, so settings do not reach sonar hardware.
- Controls, Map and System categories remain explanatory placeholders; they are not fully functional settings pages.
- Brightness is ON/OFF only, not a variable slider.
- Wi-Fi is a remote convenience function; it is not the boat command/telemetry link.
- Wi-Fi password is stored in NVS. Production credential protection requires a suitable secure-storage design and ESP32 flash encryption configuration.

## 7. GPS, map and offline storage implementation status

### Current remote code

- `boat_state.h/.cpp`: shared boat state; starts STOP-latched and unconfirmed.
- `boat_telemetry.h/.cpp`: telemetry structure and state application. GPS accepted only with an active reported radio link, GPS connected flag, reported 3D fix, coordinate ranges valid and telemetry receipt age no more than 2 seconds. This is a fresh-fix check, not a complete navigation-readiness check; accuracy and compass validity still need separate safety gates.
- `boat_ui.cpp`: coordinate, fix, satellite, accuracy and heading display; marker hidden without a fresh valid fix. Map background is still a diagnostic grid, not geographic imagery.
- `map_projection.h/.cpp`: WGS84 ↔ Web-Mercator world-pixel conversion and heading-based viewport offset rotation. Zoom guard now rejects unsupported zoom values above 22.
- `display_port.cpp`: board init, LVGL display/touch adapter and optional SD mount. TF SPI uses GPIO11/12/13 and CH422G EXIO4 for physical CS; GPIO6 is passed as a dummy CS to Arduino SD API. This arrangement remains unverified on the physical board.
- `offline_map.h/.cpp`: manifest check and single raw RGB565 tile reader into PSRAM; validates zoom, XYZ tile bounds, style, file size and read length. No multi-tile cache or LVGL geographic renderer yet.

### Offline tile pack format

```text
/maps/manifest.txt                  # begins with FLYTEC_MAP1
/maps/street/<z>/<x>/<y>.rgb565
/maps/satellite/<z>/<x>/<y>.rgb565
```

Each tile is an uncompressed 256 × 256 RGB565 image, 131,072 bytes, no header, XYZ/Web-Mercator scheme. No map imagery is included in this firmware archive. Map area/lakes have deliberately not been assumed or selected. Use lawful offline map sources and retain required attribution/licensing; do not bulk-download from the public OSM tile server.

### Still to implement

- Actual LVGL geographic tile renderer and PSRAM tile cache.
- Non-blocking SD tile loading so storage reads do not stall LVGL.
- Boat-centred map viewport, boat-up rotation using calibrated compass heading, zoom and recenter controls.
- Street/satellite style selection and home/waypoint overlays.
- Offline map pack generation/import tool and lake coverage selection.
- Waypoint persistence and map rendering integration.

## 8. Current code architecture and files

- `Flytec_Remote.ino`: setup/loop and application wiring.
- `boat_state.*`: shared boat state.
- `boat_commands.*`: application-level STOP, mode, motor, bait and hook commands with a transport callback.
- `boat_radio.*`: nRF24L01+ PA/LNA placeholder; no packet transport is active.
- `boat_telemetry.*`: telemetry structure and GPS validity/freshness handling; no live packet decoder calls it yet.
- `boat_ui.*`: main screen, diagnostic map/sonar panels, controls and Settings entry point.
- `settings_ui.*`: remote Settings and virtual keyboard.
- `joystick.*`: four-axis normalisation, calibration, dead-zone, tank/steering layouts, selectable autopilot override and safety gates.
- `joystick_calibration_ui.*`: calibration UI.
- `display_port.*`: Waveshare board/LCD/touch/LVGL adapter and SD mount.
- `esp_panel_board_custom_conf.h`: project-specific Waveshare board configuration.
- `offline_map.*`: offline map manifest and RGB565 tile loader.
- `map_projection.*`: map projection mathematics.
- `docs/FIRST_BUILD_CHECKLIST.md`, `docs/GPS_AND_OFFLINE_MAP_PLAN.md`, `docs/OFFLINE_MAP_STORAGE.md`: bring-up and subsystem notes.

## 9. Safety and not-yet-implemented functions

- Remote starts with emergency STOP active and unconfirmed. Do not clear this state without validated boat telemetry acknowledgement.
- Radio module is non-operational: `boatRadio_begin()` returns false, send returns false and update receives nothing.
- Joystick hardware reader in `Flytec_Remote.ino` intentionally returns false; no physical ADC pins are assigned. Motor commands are inhibited until hardware readings and all four-axis calibration are valid, radio is connected and STOP state is confirmed.
- Joystick calibration is RAM-only; no persistent calibration storage yet.
- No real motor command can reach the boat through this remote baseline. Bait/hook command types exist at application level but no radio transport exists.
- No boat-side firmware, GPS UART parser, QMC5883L driver, radio protocol, telemetry decoder, sonar driver or autopilot controller is included in this remote ZIP.
- Boat-side watchdog, failsafe return-home, GPS/compass quality checks, authenticated packets, command acknowledgements and independent motor/solenoid interlocks are mandatory before operation.
- Never connect this unfinished remote firmware to powered motor or solenoid stages or operate the boat based on placeholder UI indications.

## 10. Latest review changes in this archive

- Reworked Wi-Fi scan and connection status handling to use LVGL timers rather than blocking waits, preserving UI responsiveness.
- Reuses the Settings screen and its keyboard instance when the Settings window is reopened; the Settings page is kept allocated for the lifetime of the app.
- Sonar slider preferences are persisted on release rather than every drag update, reducing unnecessary NVS writes.
- Settings screen is reused when reopened instead of allocating a fresh screen and keyboard each time.
- Display timeout label reflects the stored preference accurately and states that automatic dimming is not active.
- Web-Mercator projection functions reject zoom values above 22; offline tile reader rejects invalid style enum values.
- README and subsystem notes consolidated to remove obsolete statements that SD mounting had not yet been added.

## 11. Validation status of this revision

The map projection module can be compiled and unit-tested independently. Static source checks (brace balance, expected module references and archive inventory) are useful but do not prove an Arduino build. The complete project has **not** been compiled against the actual installed Arduino-ESP32 core, LVGL 9.6.0, `ESP32_Display_Panel`, `ESP32_IO_Expander` and exact Waveshare board package in this session. Wi-Fi scan/connect, CH422G SD chip-select arrangement, LCD flush timing, touch orientation and all physical controls require hardware testing.

Before any water test, complete a compile against the real dependency versions, test display/touch/SD with motors disconnected, validate radio and STOP acknowledgement, then test motor outputs with the hull restrained and propulsion disconnected from water. Solenoids must be tested separately with current-limited supplies and appropriate flyback suppression.


## 10. Radio decision update — 2026-11-16

The user selected nRF24L01+ PA/LNA modules at both ends (remote and boat). Initial testing should start at 250 kbps. The remote-side module is not yet wired or operational. See `docs/RADIO_LINK_DESIGN.md` for the shared design requirements. No pinout, RF channel, antenna, exact module supplier/model, application packet protocol or legal EIRP configuration has been finalized.


## Hardware selection update — ver003 (2026-11-16)

See `HARDWARE_SELECTION_VER003.md`. The boat controller/GNSS selection is Heltec Wireless Tracker V1.2; the selected motion/heading sensor is GY-91-style MPU-9250 + BMP280. This supersedes the HGLRC M100 PRO B101 + QMC5883L and separate MPU6050 proposal. nRF24L01+ PA/LNA remains selected. This is an architecture/documentation decision only; hardware has not been purchased or tested and boat-side firmware drivers remain unimplemented.


## Modular hardware update — ver004 (2026-11-17)

The user approved a central carrier PCB with removable cables for both remote and boat. The remote carrier is an interconnect hub for the separate nRF24L01+ PA/LNA radio, joystick modules, physical buttons and power module. The Waveshare ESP32-S3-Touch-LCD-4.3 Standard remains the main display/controller. See `CARRIER_PCB_CONNECTION_PLAN_VER004.md` for logical connector schedules, power-domain rules and pinout gates.

No external GPIO numbers are approved. Existing RGB LCD, GT911, CH422G, TF, USB and RS485/CAN resource use must be respected; check the exact board schematic and ADC/boot constraints before assignment. The proposed 1S2P 2400 mAh battery pack exceeds the previously noted Waveshare recommendation of below 2000 mAh for its battery connector; do not connect it to that connector without manufacturer confirmation. Use a separately engineered protected charger/power module and a verified board input.

This is a documentation-only update. Radio and joystick ADC remain stubs/placeholders; no operational remote-to-boat link is claimed.

## 6. Alternative new handheld remote concept (preliminary)

A separate new handheld controller concept is preferred for exploration. It has a central 4.3-inch screen, two joysticks above symmetrical 2×4 button banks (16 total), no PlayStation symbols, no shoulder/rear buttons, two independent rear antenna systems (nRF24L01+ PA/LNA and ESP32-S3 external-antenna Wi-Fi/Bluetooth), and a proposed 1S4P pack using four 2500 mAh cells (10,000 mAh). The concept and preliminary button mapping are documented in `NEW_HANDHELD_REMOTE_CONCEPT.md`. It is not final CAD or a replacement of the current Waveshare remote baseline. Electrical, RF, battery, mechanical and safety details remain to be validated.


## 7. New handheld remote hardware workspace

The preferred alternative controller concept now has a separate design workspace at `Remote/New_Handheld_Remote/`. It includes a parametric OpenSCAD front/rear enclosure fit template, mechanical print/fit plan, modular PCB architecture, connector-interface plan, and schematic-sheet checklist. Additions include a dedicated power ON/OFF button centered above the display and a USB-C service port intended for charging and ESP32 programming/data. USB charging/programming coexistence requires a proper charger power-path and USB data design. Component footprints, connector pin numbers, power rails, CAD dimensions and circuit details remain provisional; no fabrication-ready schematic or PCB is claimed.


## New handheld remote mechanical refinement — 2026-11-18
- Started mechanical refinement R1 for the new game-controller-style remote.
- Target enclosure envelope remains provisional: 310 × 190 × 62 mm.
- Proposed internal zones: lower-central secured 1S4P cell pack, central/upper ESP32-S3 controller behind display, radio near antenna A, independent ESP32 external-antenna feed near antenna B, and charging/power management near USB-C.
- Front control coordinate plan and fit-validation sequence are documented in `Remote/New_Handheld_Remote/MECHANICAL/COMPONENT_PLACEMENT_R1.md`.
- These placements are not component-fit verified. Exact part models/dimensions, CAD revisions, test prints, and fit checks remain next steps.


## Internal layout mockup update — 2026-11-18
- Added internal placement CAD visualization and notes.
- Cell visual arrangement revised to four parallel 18650 rows, with cell axes along Y, to remove an obvious overlap in the first render.
- Main/display board overlap is still deliberately visible as a placement conflict to resolve after exact boards are selected.
- These volumes are placeholders, not component footprints or a validated battery holder.


## CAD geometry verification — 2026-11-18
- Corrected the front-panel screen aperture cut depth so the opening passes fully through the 3.2 mm panel.
- OpenSCAD export/render was verified in the current working environment; the front panel and rear shell each produced a simple 3D mesh.
- This validates basic geometry only. The component dimensions, mounting, fit, wall strength, assembly alignment and printability under chosen settings remain unverified.
- Internal layout mockup now assumes a single integrated touchscreen/ESP32-S3 controller board to avoid a separate overlapping controller-board placeholder. Exact board selection and external-antenna support are still TBD.


## Reference-based controller styling update — 2026-11-18
- User uploaded the visual reference and confirmed it as the target shape/style.
- New baseline: sculpted matte-black controller with angular shoulder transitions, deep screen bezel, joystick wells, textured outer grips/lower handle contours, rear service cover, two antennas, and centered lower USB-C.
- Remove blue LED rings, grip LED strips, illuminated button surrounds and decorative backlighting.
- Functional controls remain unchanged. Current CAD geometry has not yet been reshaped to this style; this update records the design decision and next mechanical revision target.


## Concept image upload verified — 2026-11-18
- Official concept JPEG uploaded and verified at `Remote/New_Handheld_Remote/CONCEPT/Flytec_Remote_Concept_Reference.jpg` (344,416 bytes).
- README and concept documentation now link to the image.
- Visual baseline remains sculpted matte-black enclosure; decorative blue LED rings, grip lighting and backlighting excluded.


## Mechanical refinement R2 — concept preview (2026-11-18)
- Generated a multi-view concept preview to explore front, angled, rear, side, and internal layout styling against the user's uploaded reference.
- Visual target: sculpted matte-black shell, textured grips, recessed display, joystick wells, rear access panel, dual antennas, lower USB-C.
- Decorative LED rings, grip strips, illuminated button surrounds, and blue backlighting remain excluded.
- Preview is illustrative only. Dimensions and internal arrangement shown are not verified CAD or component-fit results; next CAD work must translate the contours into parametric geometry and validate physical parts.


## Mechanical refinement R2 — expanded battery reserve (2026-11-18)
- Provisional shell depth allowance increased from 62 to 70 mm to provide additional internal volume.
- Reserved lower-central battery volume enlarged to 104 × 82 × 24 mm (placeholder envelope).
- Controller/display envelope moved upward in the space study to improve separation from the battery reserve.
- Four 18650 placeholders remain illustrative; pack retention, electrical protection, charger, wiring, and clearances are not designed or validated.
- OpenSCAD internal mockup R2 was rendered to STL successfully; PNG render attempt failed due to headless OpenGL/X-server limitation. A separate schematic space preview is available for review.
- This revision is a layout study, not verified assembly or print-ready CAD.


## Component selection / dimensional integration — 2026-11-18
- Added candidate component integration plan with dimension envelopes and power architecture for 1S4P 18650 pack.
- Waveshare 4.3 ESP32-S3 touchscreen remains a development candidate; onboard antenna does not meet external ESP32 antenna requirement.
- C&K THB001P selected as preferred joystick candidate, not yet purchased.
- TI BQ25895 selected as charger/power-path candidate, charge-current and thermal design not finalized.
- No parts purchased; BOM remains preliminary.


## Power/RF recommendation update — 2026-11-18
- User confirmed Waveshare ESP32-S3-Touch-LCD-4.3; onboard antenna accepted.
- Battery remains 1S4P 18650 side-by-side; pack protection assumed; charging schematic out of scope; exact cell choice TBD.
- Recommendations documented: nRF24 PA/LNA SMA module, dedicated 3.3 V rail, TPS61088 5 V boost, downstream 2 A provisional fuse, thermal test target.
- No purchase-ready BOM or schematic yet.


## Power distribution PCB — R1 (2026-11-18)
- Added `PCB/POWER_DISTRIBUTION_DESIGN_R1.md` with input/fuse, 5 V boost, dedicated 3.3 V radio rail, placement/grounding, thermal, test points, connector groups and prototype test sequence.
- Confirmed display board onboard antenna accepted; no ESP32 antenna requirement.
- Recommended 5 V boost candidate: TPS61088; TI reference designs cover 5 V/3 A and low-EMI layout/short-circuit protection concepts. 3 A capability is not assumed for our PCB until designed and validated.
- Recommended radio 3.3 V buck-boost: TPS63070 (input 2 V–16 V; 3.3 V output design TBD), chosen because battery voltage crosses 3.3 V.
- Fuse recommendation revised to 3 A time-delay provisional due to 1S input current and peak loads. Final fuse must be verified against load profile, wiring/connectors and pack protection; previous 2 A idea superseded.
- Battery pack protection assumed onboard; verify pack BMS specification. Charging design remains out of scope.


## Net-level power schematic draft R1 — 2026-11-18
- Created `PCB/POWER_SCHEMATIC_NET_DRAFT_R1.md`: BATTERY_INPUT, TPS61088 5 V boost, TPS63070 3.3 V radio buck-boost, SENSE_CONTROL, connector sheet/net groups, provisional power budget and unresolved component values.
- This is a reviewed net-level draft, not a KiCad schematic or fabrication-ready design. Feedback/compensation, inductors, current limits, exact capacitors, fuse, reverse-polarity circuit and connector pins remain to be calculated/selected.


## First converter schematic sheet — pin/net map R1
- Added `PCB/TPS61088_PIN_NET_MAP_R1.md` with pin-to-net connections for the 5 V boost converter, functional external-component list, and caveats requiring package/pin verification.
- This is not yet a native KiCad schematic; local KiCad CLI/editor was not available for generating or ERC-validating a schematic file in this environment.


## New handheld remote power PCB — TPS63070 pin map update (2026-11-18)
- Added `Remote/New_Handheld_Remote/PCB/TPS63070_PIN_NET_MAP_R1.md` based on TI TPS63070 datasheet Rev. B.
- Pin numbers and functional mapping were checked against the datasheet. Converter input/output range and nominal capability are recorded, but output current capability for our layout is not asserted.
- The radio supply is still a provisional 3.3 V buck-boost rail from the 1S4P pack. Feedback resistor values, capacitor/inductor selection, mode, enable bias, package footprint verification and layout remain to be engineered and tested.
- README updated to link/document this pin map. This remains a planning document; no native KiCad sheet or ERC was produced.


## New handheld remote — battery input/protection net map R1 (2026-11-18)
- Added `Remote/New_Handheld_Remote/PCB/BATTERY_INPUT_PROTECTION_NET_MAP_R1.md` for pack connector, fuse, reverse-polarity protection, protected distribution, ADC battery sense, optional pack NTC and test points.
- Main fuse remains 3 A time-delay provisional. Final fuse and connector ratings must be based on measured peak/startup current, pack/BMS limits, wire gauge and downstream converter requirements.
- BMS/protection is assumed, not verified. Charging design is outside scope. Reverse-polarity circuit, fuse, connector, capacitors, ADC divider and NTC values are not finalized.
- README updated. All current power-sheet artifacts are net/pin planning documents; native KiCad schematics, PCB layout, ERC and fabrication outputs have not been produced.


## Power-document review and net-name harmonization — 2026-11-18
- Read the current new-handheld remote power distribution, net-level draft, TPS61088 pin map, TPS63070 pin map, battery input/protection map and component-selection/integration document.
- Harmonized protected battery distribution naming to `BAT_PROT` across the power architecture and converter net maps.
- Corrected component-selection text to reflect that the Waveshare onboard antenna is accepted and the external ESP32 antenna requirement is waived.
- Removed charger IC/2 A charging-current selection from the current scope. Charging schematic remains out of scope; USB-C power/charging/data roles still need definition before hardware implementation.
- All artifacts remain planning docs. Values, exact footprints, reverse-protection implementation, fuse/connector selection, PCB layout, ERC and hardware validation remain open.

## New handheld remote — TPS63070 component calculation review R1 (2026-11-18)
- Added Remote/New_Handheld_Remote/PCB/TPS63070_COMPONENT_CALCULATIONS_REVIEW_R1.md with feedback calculations, inductor/capacitor proposals, radio current scenarios, combined remote current estimate, and schematic/layout validation gates.
- Feedback proposal: 470 kΩ / 150 kΩ gives 3.307 V nominal and approximately 5.33 µA divider current.
- Inductor candidate: Coilcraft XFL4020-152ME 1.5 µH; current ratings and suffix to be confirmed at procurement.
- Proposed capacitor set follows TI reference input values and VAUX bypass; output bank proposed as 4×22 µF + 10 µF local (98 µF nominal), pending DC-bias effective-capacitance verification.
- At 200 mA radio load, 3.0 V battery and assumed 90% efficiency, estimated radio branch battery current is approximately 245 mA. Preliminary whole-remote estimate (450 mA/5 V display at assumed 85% boost efficiency plus radio) is approximately 1.13 A at 3.0 V; not measured and excludes other loads/transients.
- The calculations are a design baseline, not validated stability or a fabrication BOM. Pin-mode configuration, exact capacitor parts, schematic capture, ERC, layout, thermal and bench testing remain pending.

## Datasheet correction and configuration review update (2026-11-18)
- TPS63070 adjustable single-rail configuration: FB2 grounded; VSEL held LOW (no dynamic voltage scaling); PS/SYNC HIGH to enable power-save PFM/PWM operation; EN pulled high through a 10 kΩ resistor to BAT_PROT for always-enabled rail. These are the design choices to implement, pending symbol/net review. If MCU shutdown control is added, it must safely override EN.
- PG is open-drain; 100 kΩ pull-up to 3V3_RADIO is a candidate for MCU status monitoring, not mandatory. Verify the intended MCU input and PG behavior before capture.
- Capacitor-bank proposal corrected: follow TI's adjustable 3.3 V typical application BOM (3×22 µF X6S 0805 + 10 µF 0603, nominal 76 µF), rather than the earlier 98 µF proposal. TI's filter table supports 1.5 µH with nominal 47/68/100 µF; the 76 µF typical-app bank is within the recommended combination table. Effective capacitance remains to be confirmed using manufacturer DC-bias/temperature data. Datasheet permits effective output capacitance 15–470 µF for nominal 1.5 µH.
- Selected 22 µF candidate: Murata GRM21BC81C226ME44L, 16 V X6S, 0805, ±20%, -55°C to +105°C. Exact DC-bias capacitance at 3.3 V is not stated in the basic part specification and must be verified from Murata's bias curves/approval data. Three units are specified by TI's reference BOM.
- The prior 98 µF nominal output proposal is superseded. See the calculation review and pin/net map for corrected baseline.
Sources: TI TPS63070 Rev. B datasheet, §§8.5.1, 9.2.2.1–9.2.2.3, Tables 2–4; Murata GRM21BC81C226ME44L reference sheet.
