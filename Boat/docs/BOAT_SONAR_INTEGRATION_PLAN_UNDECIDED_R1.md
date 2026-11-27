# Sonar integration plan — preferred candidate and software input path

**Owner constraint:** candidate selection is not purchase, wiring, or electrical approval. Software-only NMEA DBT parsing, compact depth telemetry, and Remote depth display integration are authorized; no GPIO assignment, transducer power, hardware activation, or full echo transport is approved.

**Sonar choice — 2026-11-27:** Owner-selected preferred candidate: [Open Echo TUSS4470](https://github.com/Neumi/open_echo) + Lucky D+T wired 200 kHz / 45° transducer, including live water temperature measurement. Electrical compatibility, probe pinout, temperature sensor characteristics, shallow-water performance and CHIRP capability remain unverified.

**Software integration — 2026-11-27:** The [Open Echo repository](https://github.com/Neumi/open_echo) provides an [NMEA DBT output sketch](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/arduino/NMEA_DBT_OUT/NMEA_DBT_OUT.ino) that emits depth-only serial sentences. Boat now has a bounded checksum-validating DBT decoder and normalized sensor ingress; Remote receives the compact depth summary. The DBT path does not carry water temperature, range configuration, or raw echo amplitudes; the current sketch calculates depth with a fixed 1450 m/s sound speed. A verified dedicated UART adapter, depth accuracy evaluation and target validation are still required before live readings.

## Required end-user behaviour
- Full fishfinder-style echogram with real *fish arches* on the existing single **4.3-inch 800×480 Waveshare Remote display**. No separate sonar screen.
- Visible while manually driving and during autopilot; 50/50 map/sonar split view with selectable full-screen echogram or full-screen map and a return control.
- Prefer a real ready-made wired CHIRP sonar/sounder with usable echo intensity or raw profile data; do not equate a sonar's proprietary app picture with an accessible integration API.
- Support the intended shallow fishing range approximately **0.5–10 m**, with bottom tracking, depth scale, fish targets and usable arch depiction. Real arches depend on beam geometry, movement and display history; a narrow beam alone is not proof of reliable arches.

## Unselected electronics boundaries
The Boat central carrier will preserve **unpopulated optional J_SONAR space/physical routing** outside the committed motor, coil, GPS, radio and safety-critical nets. Reserved only as an envelope and wire-routing corridor; no assigned UART/SPI/USB/Ethernet pins, bus address, voltage or current are frozen. A selected module may require an isolated supply, a dedicated controller or different data transport; assess after the model is chosen. Keep any transducer and sounder power/noise return separate from PWM motor current, avoid false GNSS/compass readings, and verify enclosure clearance, waterproof routing and bottom-facing mounting geometry.

The Remote continues to have one display. Radio telemetry architecture must distinguish sonar bandwidth from compact command and GNSS telemetry: **nRF24/Lora may be unsuitable for transmitting full-resolution echo waterfalls**. Candidate architecture options (not implemented) include a specifically budgeted high-throughput auxiliary link or local preprocessed compression, subject to radio coexistence, power and license verification. Do not claim a full echo image is available through current telemetry messages.

## Candidate qualification checklist
1. Manufacturer and exact sounder/transducer SKUs, a documented interface or a licensed SDK for timestamped amplitude-vs-depth echo samples or rendered real echograms.
2. Real example captures (not marketing mockups) with fish arches at 0.5, 1, 2, 5 and 10 m; bottom and suspended fish, shallow bottom clutter, boat movement, target distinction.
3. CHIRP sweep frequencies/bandwidth, beam cone angle, transducer size/mounting, environmental protection, interference and permitted acoustic power.
4. Data volume: ping rate, samples/ping, bit depth, timestamps, compression, transport and latency; sustained capacity across the chosen link and Remote decode/PSRAM/UI update path.
5. Supply voltage, peak/average current, acoustic/electrical interference, thermal and waterproof requirements; fit against the Boat 3S/logic power system without disturbing independent safety cutoff.
6. Host protocol and legal rights to parse, store and display echo data; third-party fishfinder ecosystem pairing/DRM restrictions.
7. Simulator or recorded sample data for deterministic UI integration tests.
8. Transparent pricing, purchase availability and support, weighed **only after** the data/protocol gates pass.

## Development sequencing after model selection
A. Freeze the device/API/power/transducer and data transport. B. Capture real sample records under a permitted interface. C. Budget RF/data and implement isolated receive/decoder (separate from Boat motor and watchdog logic). D. Render calibrated echogram, fish-arch history and bottom trace with LVGL on the existing screen. E. Test against recorded captures and measured on-water echoes with manual/autopilot view. F. Check impact on battery, radio and watchdog deadlines. 

**Current:** software-only DBT decoder and compact depth telemetry are prepared. No physical reader, UART pin assignment, module wiring, transducer drive, or echogram decoder is active.
