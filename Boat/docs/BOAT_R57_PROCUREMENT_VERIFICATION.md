# R57 — procurement verification (2026-11-22)

User confirms all Boat modules are still to be purchased. No custom PCB. Remote owns integrated SD. Sonar remains deferred.

## Verified item
- Electrokit EKM014, SKU 41024394, four-channel MOSFET driver, 5–50V DC load rail (VH), 4.5–12V logic rail (VL), specified 3–12V logic levels, 10A/channel stated by manufacturer. Product: https://www.electrokit.com/mosfet-drivkort-4-kan-50v-10a ; manufacturer docs index: https://docs.electrokit.com/modules/
- Manufacturer listing checked 2026-11-22 displayed 169 SEK and stock; Bulgarian shipping not verified. Terminal blocks and header supplied but must be soldered. Do not assume plug-and-play.
- Critical: independently verify safe boot state, input polarity, separate EN controls, flyback suppression and measured 6A solenoid pulses. Hardware one-hot pulse limiting and independent coil supply disconnect remain mandatory.

## Unverified procurement / hold
- Exact ESP32-S3-DevKitC-1-N32R16V SKU and Bulgarian/Farnell stock not confirmed; Farnell ESP32-S3 category search did not establish an exact SKU match.
- MDD20A, HGLRC M100 PRO, GY-91, nRF24 PA/LNA, E22 exact model, 5V and 3.3V converters, DC-rated contactors, independent watchdog, fuse, wiring, sealed enclosures, and coil current sensing: exact shop listing, stock and electrical ratings pending.
- Do not substitute a different ESP32 board or E22 module without pin/protocol review.
- Purchase sequence: low-power controller, sensors, RF and power regulators first; motor/coil power and independent safety hardware only after measured current budget and approved module specifications.

No physical assembly, motor enabling, or water acceptance. This is procurement research, not a verified shopping cart or working prototype.
