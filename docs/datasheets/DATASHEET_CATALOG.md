# Flytec V080 Retrofit — Datasheet Library

**Folder:** `docs/datasheets/` (downloaded files will be placed under `PDF/`)

This library tracks the project hardware as selected, candidate, or unresolved. PDF acquisition is automated through GitHub Actions because the ChatGPT runtime cannot directly download arbitrary binary files and upload them to GitHub. The workflow downloads the listed sources from GitHub-hosted runners, validates that each result is a PDF, and commits the successful files. Failed/missing source links are reported in the generated acquisition report; do not treat a product as finalized merely because its datasheet is present.

## Current project parts

| System | Element | Status | PDF target / source |
|---|---|---|---|
| Remote | Waveshare ESP32-S3-Touch-LCD-4.3 Standard (board schematic/user guide) | Selected, not purchased | Waveshare wiki downloads; schematic PDF is model-specific |
| Remote | ESP32-S3 / board modules, touch GT911, IO expander CH422G | Integrated on display board | Board documentation and chip datasheets |
| Remote/Boat | Nordic nRF24L01+ RF IC (module is PA/LNA variant) | Selected candidate; module vendor/revision TBD | Nordic nRF24L01P datasheet plus module-specific docs TBD |
| Remote/Boat | EBYTE E22-900T22S UART LoRa | Selected, model/revision verify | Official Ebyte manual |
| Remote | SC16IS750 I2C-UART bridge | Candidate | NXP datasheet |
| Remote | 1S4P 18650 pack, 2500mAh cells proposed | Cell model unresolved | Cell datasheets TBD; no pack PDF available yet |
| Remote | 5V boost converter | Selection unresolved | Datasheet TBD |
| Remote | Protection/fuse/thermal parts | Design not frozen | Datasheets TBD |
| Boat | Heltec Wireless Tracker ESP32-S3 + UC6580 + SX1262 | Candidate, board revision unknown and not purchased | Heltec revision-specific schematic/datasheet; use caution, product phaseout |
| Boat | GNSS UC6580 (integrated) | Integrated/candidate | UC6580 datasheet if available; module documentation |
| Boat | MPU-9250 IMU | Candidate | InvenSense datasheet |
| Boat | BMP280 pressure sensor | Candidate | Bosch datasheet |
| Boat | Cytron MDD20A dual motor driver | Candidate; too large for original carrier footprint | Official MDD20A datasheet |
| Boat | 550 brushed 12V motors | Existing motors; exact manufacturer/model unknown | Manufacturer datasheet unavailable until marked motor identified |
| Boat | FW0630 009 DC12V bait solenoid | Selected model, electrical specs not verified | Vendor datasheet TBD |
| Boat | FW0630 001 DC12V hook solenoid | Selected model, electrical specs not verified | Vendor datasheet TBD |
| Boat | DMTH3002LK3-13 MOSFET | Prior repair/possible component reference; driver selection not frozen | Diodes Inc datasheet |
| Boat | 3S4P Li-ion pack, BMS 20A (reported) | Existing pack, details need confirmation | Cell/BMS datasheets TBD |
| Boat | Sonar | CHIRP-style ready module concept, exact unit not selected | No datasheet yet |
| Both | Connectors, wire, antenna, fuse, regulators, DC/DC | To be finalized after schematic/BOM | Individual part datasheets TBD |

## Source links (initial)
- EBYTE E22-900T22S manual (official source link): https://www.cdebyte.com/pdf-down.aspx?id=4217
- Waveshare ESP32-S3-Touch-LCD-4.3B schematic (B variant reference only; board variant must match): https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B/ESP32-S3-Touch-LCD-4.3B-Sch.pdf
- Cytron MDD20A datasheet: https://download.kamami.pl/p587223-MDD20A%20Datasheet.pdf
- Nordic nRF24L01+ datasheet: https://infocenter.nordicsemi.com/pdf/nRF24L01P_PS_v1.0.pdf
- NXP SC16IS750 datasheet: https://www.nxp.com/docs/en/data-sheet/SC16IS750.pdf
- Bosch BMP280 datasheet: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp280-ds001.pdf
- Diodes Inc. DMTH3002LK3 datasheet: https://www.diodes.com/assets/Datasheets/DMTH3002LK3.pdf
- Heltec Wireless Tracker product resource page (links revision docs): https://heltec.org/project/wireless-tracker/
- Heltec Tracker documentation portal: https://docs.heltec.org/

## Important
1. Datasheet sources may refer to similar variants, not necessarily the exact revision purchased. Verify model/PCB revision before using electrical limits or pinouts.
2. Keep vendor manuals, schematic PDFs, IC datasheets, certifications, and application notes in separate filenames.
3. No datasheet can substitute for measurements, design review, bench testing, or regulatory assessment.
4. We will expand the catalog as the BOM is frozen and add PDFs in groups, avoiding guessed or mismatched documents.


## Acquisition state (checked 2026-11-20)
The initial automated download report is available at [DOWNLOAD_REPORT.md](DOWNLOAD_REPORT.md). Four PDFs were acquired and two direct-source downloads failed (Nordic nRF24L01P and NXP SC16IS750); treat the failed entries as outstanding, not as missing specifications.

### Priority queue for next acquisition batch
1. **Exact Waveshare Standard board documentation** — board revision, schematic, pin map and official user guide. The existing B-variant schematic is explicitly reference-only.
2. **nRF24L01+ PA/LNA module** — IC datasheet plus exact module documentation and antenna/PA supply requirements. The IC datasheet does not cover the PA/LNA module.
3. **SC16IS750** — official NXP datasheet and address/pin/package references; verify whether this bridge is actually needed after UART/pin audit.
4. **Remote power parts** — TPS63070 and TPS61023 datasheets; CS8501 board documentation; battery cell model, pack wiring, fuse, protection and connector datasheets.
5. **Boat controller** — exact Heltec Wireless Tracker revision; board schematic/pin map and UC6580 GNSS documentation. Confirm the integrated LoRa radio is not used in the selected architecture.
6. **Boat motor and release drivers** — selected motor-driver datasheet and current limits; solenoid resistance/current/duty-cycle documentation for FW0630 009 and FW0630 001; MOSFET/driver, gate resistor, flyback clamp, fuse and thermal datasheets.
7. **Navigation sensors and sonar** — MPU-9250 source/documentation and exact selected CHIRP sonar module/interface, once selected.
8. **Power/BOM finalization** — regulators, TVS, reverse protection, battery/BMS, wiring, keyed connectors, fuse holders and antenna coax/connectors.

Do not mark any unresolved item as approved based only on a datasheet download. Exact model/revision matching and engineering review are still required before schematics are frozen.
