# Whole-project modular BOM and shopping links R1 — 2026-11-21
Links are verified supplier/catalogue or manufacturer pages where possible, NOT promises of current stock. Do not substitute electrical parts by appearance. Primary preference: reputable Bulgarian suppliers, Farnell Bulgaria, Cytron authorized distributors, Electrokit; AliExpress only after exact specifications and seller verification. Exclude OLX/eMAG.

| Subsystem | Qty | Preferred | Substitute / decision rule | Shop / sourcing link | Verification |
|---|---:|---|---|---|---|
| Boat controller+GNSS+LoRa | 1 | Heltec Wireless Tracker v1.2 | Same revision only until GPIO/schematic audit; later revision requires re-audit | https://heltec.org/ ; https://bg.farnell.com/ | Exact SKU and header map pending |
| Boat motor driver | 1 | Cytron MDD20A dual brushed H-bridge | Two independent rated brushed H-bridges with reverse, logic compatibility and sufficient stall margin; no automatic swap | https://www.cytron.io/distributor ; https://www.erelement.com/ | Availability and measured motor load pending |
| Boat solenoid driver | 1 | Electrokit EKM014 4ch 50V 10A, 41024394 | Two EKM013 2ch boards only if identical interface and tested at measured coil current | https://www.electrokit.com/en/control-units/ | EKM014 listed in stock; shipping to BG check |
| Boat 5V regulator | 1 | Preassembled DC/DC buck module rated for 3S full charge/transients | Electrokit EKM006 3.3/5V 5A only if input range and thermal ratings qualify | https://docs.electrokit.com/modules/ ; https://bg.farnell.com/ | No selected SKU yet |
| Boat primary radio | 1 | nRF24L01+ PA/LNA 2.4GHz | Equivalent module only after chip, antenna, current, 3.3V and range testing | https://bg.farnell.com/ | SKU and stock pending |
| Boat IMU/compass | 1 | MPU9250 + BMP280 breakout | ICM-20948 breakout plus BMP280 only with firmware and heading calibration changes | https://bg.farnell.com/ | Verify actual silicon and voltage |
| Boat battery pack | 1 | Existing 3S4P 18650 10Ah / 20A BMS (reported) | Rebuild only by qualified battery specialist if failed acceptance | Existing asset | Pack electrical test pending |
| Boat motors | 2 | Existing 550 brushed motors | Matching brushed 550 after stall current and shaft fit tests | Existing asset | Current unknown |
| Boat bait solenoids | 2 | Existing FW0630-009 12V | Same stroke/force/current/duty and mounting | Existing asset | Measure |
| Boat hook solenoids | 2 | Existing FW0630-001 12V | Same stroke/force/current/duty and mounting | Existing asset | Measure |
| Boat fuses, disconnect, terminal distribution, waterproof connectors and wiring | set | Rated off-the-shelf industrial components | Equivalent current/IP/temp ratings | https://bg.farnell.com/ | Ratings TBD from measurements |
| Remote controller/display | 1 | Waveshare ESP32-S3-Touch-LCD-4.3 standard | No drop-in substitute without UI/enclosure/firmware changes | https://www.waveshare.com/ ; https://bg.farnell.com/ | Confirm exact revision |
| Remote primary radio | 1 | nRF24L01+ PA/LNA | Same electrical/RF validation as boat | https://bg.farnell.com/ | Pending |
| Remote backup LoRa | 1 | EBYTE E22-900T22S (legacy remote architecture) | Change to EU868-compatible verified LoRa module only with protocol and RF audit | https://www.ebyte.com/ ; https://bg.farnell.com/ | Remote architecture must be reconciled with boat integrated SX1262 |
| Remote joystick | 2 | Analog two-axis joystick | Equivalent mechanical/ADC range | https://bg.farnell.com/ | Mechanical fit pending |
| Remote buttons | 16 + power | Momentary tactile buttons, 2x4 each side | Rated sealed equivalents | https://bg.farnell.com/ | Exact footprint pending |
| Remote ADC and I/O expansion | 1 each | ADS1115 and MCP23017 module | Compatible module with firmware adjustment | https://bg.farnell.com/ | Existing source pin map audit pending |
| Remote battery | 1 | 1S4P protected 18650 pack, 2500mAh cells | Qualified assembled protected pack with charger matching chemistry | https://bg.farnell.com/ | Pack/charger not selected |
| Remote radio power | 1 | Ready-made regulated 3.3V supply; legacy TPS63070 design is superseded if modular policy applies to remote | Rated buck-boost with measured peak RF current | https://bg.farnell.com/ | Remote power design requires reconciliation |
| Remote USB-C charge/update, connectors, enclosure | set | Compatible ready-made charge/protection module and printed case | Only after battery/board fit verification | https://www.waveshare.com/ ; https://bg.farnell.com/ | Pending |
| Sonar | 1 | TBD CHIRP | No purchase recommendation yet | TBD | Deferred by user |
| Offline maps SD | 1 | Waveshare integrated SD interface and suitable microSD | Industrial-rated microSD | https://www.waveshare.com/ ; https://bg.farnell.com/ | Verify capacity/interface |

## Supplier evidence
- Electrokit EKM014 4-channel board listing: https://www.electrokit.com/en/control-units/ ; EKM013 description and 3.3V control: https://docs.electrokit.com/en/modules/EKM013/
- Cytron distributor page explicitly lists Bulgarian ERELEMENT and Farnell: https://www.cytron.io/distributor
- Farnell Bulgaria catalogue: https://bg.farnell.com/
- No supplier product URL is to be described as in-stock unless directly checked for that exact SKU; all catalogue links are sourcing leads.
