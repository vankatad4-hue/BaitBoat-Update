> **Priced full-project BOM VER001 — 2026-11-23:** [63-line purchase register](../../docs/procurement/Flytec_V080_Full_BOM_Prices_2026-11-23.csv) and [budget/pricing gates](../../docs/PROJECT_PRICED_BOM_VER001.md). **Boat €346.47 + Remote €389.71 = €736.18** provisional base, excluding seven **unpriced mandatory safety workstreams**, any selected CHIRP sonar, taxes/delivery differences and hardware tests. Retained Boat shell/motors/solenoids/3S4P pack counted at €0 new purchase; candidates and estimates are not confirmed orders. Official Boat **VER014** / Remote **VER028** remain frozen. Active PlatformIO Boat architecture uses ESP32-S3-DevKitC + HGLRC M100 PRO, **not** older Heltec Tracker draft.

# R58 — verified core procurement shortlist (2026-11-22)

User confirms all modules still to be purchased. No custom PCB. No Boat SD; Remote Waveshare integrated SD. Sonar deferred. **Do not order unverified safety parts.**

| Part | Exact item | Supplier evidence | Listed unit price | Status |
|---|---|---|---|---|
| Boat MCU | Espressif ESP32-S3-DevKitC-1-N32R16V (32MB flash,16MB PSRAM) | Mouser Bulgaria https://www.mouser.bg/en/ProductDetail/Espressif-Systems/ESP32-S3-DevKitC-1-N32R16V?qs=sqEgtWRSLJ3pTWIVKUeIew%3D%3D ; DigiKey Bulgaria https://www.digikey.bg/en/products/detail/espressif-systems/ESP32-S3-DEVKITC-1-N32R16V/26658348 | Mouser €13.67 displayed | Exact model listed; recheck cart, VAT and delivery |
| Dual motor driver | Cytron MDD20A, two channels, 6–30V 20A/channel | Partco https://partco.shop/brand/cytron SKU MOT-MDD20A | €42.11 displayed | Exact dual-channel item listed in stock; Bulgaria shipping unconfirmed |
| Solenoid low-side driver | Electrokit EKM014, 4-channel 50V 10A advertised | https://www.electrokit.com/en/motor-control-1/ ; manufacturer manual https://www.electrokit.com/upload/quick/ea/ed/1094_manual.pdf | 169 SEK displayed | In stock on listing; 4.5–12V logic supply, 3.3V logic inputs, VH >= VL; Bulgarian delivery unconfirmed |

## Procurement hold / not verified
HGLRC M100 PRO GNSS exact SKU and compass type; GY-91 MPU9250/BMP280 authenticity; nRF24L01+ PA/LNA and 3.3V regulator; exact E22 UART module and matching remote LoRa PHY; 5V buck, independent watchdog, physical STOP, **DC-rated** contactors, fuses, 6A solenoid current sensing, coil one-hot limiter, harness/connectors, enclosure and 3S4P battery. All require exact rated supplier listings and design review before ordering.

## Engineering rules
- The selected Cytron board is **MDD20A dual**, not MD20A single-channel.
- EKM014 is not a safety controller. Validate safe boot, one-hot hardware and flyback behavior, supply levels and physical power disconnect.
- Do not energize motors/coils until physical STOP, independent watchdog, DC-rated cutoff, feedback and fault tests pass.
- Listed prices are snapshots, not quotes; VAT, shipping, availability may change.
- Existing firmware remains non-actuating; no prototype or release claim.
