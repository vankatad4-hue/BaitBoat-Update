# R45 manufacturer PDF register — verification and retrieval

The GitHub text connector cannot upload binary PDFs. **This is a verified download register, not a claim that PDFs have been stored in the repository.** Download only from manufacturer sites, verify revision and SHA-256, then archive under `docs/datasheets/PDF/` through a binary-capable Git client.

## Verified direct manufacturer PDFs
1. ESP32-S3-WROOM-2 module datasheet v1.7 — https://documentation.espressif.com/esp32-s3-wroom-2_datasheet_en.pdf — **GPIO47/48 are 1.8V; GPIO35–37 unavailable on WROOM-2**.
2. ESP32-S3 SoC datasheet — https://documentation.espressif.com/esp32-s3_datasheet_en.pdf
3. ESP32-S3-DevKitC-1 v1.1 schematic — https://dl.espressif.com/dl/schematics/SCH_ESP32-S3-DevKitC-1_V1.1_20221130.pdf
4. ESP32-S3-DevKitC-1 v1.1 official guide — https://documentation.espressif.com/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html

## Remaining manufacturer PDFs to locate/validate before wiring
- HGLRC M100 PRO B101 GNSS hardware manual, pin map, UBX NAV-PVT configuration and QMC5883L/P revision-specific register datasheet. Manufacturer page: https://www.hglrc.com/products/hglrc-m100-pro-gps
- EBYTE **exact purchased E22 UART SKU** hardware manual, voltage, AUX/M0/M1 timing, RF mode, antenna and compliance.
- Nordic nRF24L01+ product specification and **exact PA/LNA module** supply/antenna manual.
- Cytron MDD20A official manual, PWM/DIR/enable/current/thermal ratings.
- Electrokit EKM014 manufacturer module manual and verified 3.3V control/5V VL wiring.
- MPU-9250 and AK8963 datasheets, BMP280 Bosch datasheet, ADS1115 datasheet.
- 3S4P BMS and each selected regulator, fuse, DC contactor, physical STOP supervisor, independent watchdog, pulse one-hot and current-zero sensor manuals.
- Waveshare ESP32-S3-Touch-LCD-4.3 board schematic and pinout, touchscreen/SD/LVGL 9 compatibility, remote nRF and E22 manuals.

**Unselected models cannot have a definitive datasheet**. Preserve model numbers and SHA-256 after procurement; no guessed PDF links.
