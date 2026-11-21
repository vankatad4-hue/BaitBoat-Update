# R61 — matched backup radio and fail-closed output validation (2026-11-22)

## Correction to R60 radio candidate
The existing Remote specification `Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md` **already selects identical EBYTE E22-900T22S UART LoRa modules at both endpoints** (initial 868.125 MHz, 9600 8N1, 62.5kbps air, 32B subpackets, 10dBm power). E22-900T22D-V2.0 discussed in R60 is **not** a drop-in equivalent. Do not purchase mixed E22 variants and assume RF interoperability.

The E22-900T22S is an SMD 26x16mm UART module with an I-PEX antenna connection, **not a connectorized DIP breakout**. Supplier listing (not a confirmed Bulgaria checkout) https://www.micros.com.pl/en/product/rf-e22-900t22s,171727.html lists manufacturer EBYTE, part E22-900T22S, RF E22-900T22S, 4 pieces and EUR 10.6255 net at time consulted. Its small stock quantity, delivery, antenna matching and whether a genuine preassembled SMD-to-connector adapter can be sourced must be rechecked. Manufacturer https://www.ebyte.com/product/1587.html. Do **not** commission with improvised bare SMD wiring. If no off-the-shelf adapter exists, formally evaluate **a matched pair** of E22-900T22D DIP modules, but require version-specific config/readback comparison and physical bench interoperability *before changing both designs*.

Remote uses the SC16IS750 I2C UART bridge; Boat uses direct UART. Configuration command bytes, frame header and HMAC/CRC wire codec are identical in both source trees at R61. Static drift regression: `Boat/tests/test_r61_e22_pair_contract.py`. **Not yet physical RF verification.** Initial RAM-only duty-cycle limiter, missing persistent accounting and provisioning of session secrets block outdoor LoRa reliance. A STOP command cannot bypass legal RF duty limits; independent hardware STOP is obligatory.

## Physical output safety fix
Updated `Boat/source/ekm014_pulse_guard.h` to deny a new solenoid command until *measured* coil current is zero and expose guard validity. Updated `Boat/source/boat_physical_driver_adapter.h` to:
- reject disagreement between independent coil-current feedback and physical contacts;
- reject motor demands on stale coil feedback, missing independent pulse limiter readiness, unfinished zero-current acknowledgement or invalidated guard after a fault;
- fail closed if guard times out, loses feedback or faults during a tick;
- maintain independent power-cutoff callback and STOP requirement.

New `Boat/tests/boat_physical_driver_safety_r61_test.cpp` covers normal coil-off/post-pulse current-zero transition, disagreement, stale feedback and STOP. CI gains cross-device LoRa and physical fail-closed tests. Existing R53 calibration-store test had a `-Werror=misleading-indentation` compiler failure and was corrected at R61.

## Safety hardware purchase screening
- **Albright SW80-358** contactor is a real DC-rated candidate listed through Comet/Farnell Bulgaria https://store.comet.bg/en/CatalogueFarnell/Product/109936/ . Listing shows 12V coil, 125A maximum DC contact current and approximately 13W coil operating power; **currently reports 0 stock / awaiting delivery**. This is a large contactor (approximately 91x50x52mm), may be inefficient for a small bait boat, and is not an approved fit. Validate coil continuous duty, contact-current breaking rating for actual motor/solenoid DC load, auxiliary contact availability, 3S battery voltage, coil suppression, and mechanical/waterproof installation before selection.
- **ZIEHL WD100V** independent watchdog relay https://archiv.ziehl.com/en/AllProducts/detail/WD100V-103/ is **rejected for direct 3S battery power**: it requires AC/DC 24-240V (DC minimum 20.4V). Do not specify it as a drop-in 12V watchdog.
- Need a genuine independent watchdog directly supplied from 3S or verified regulated supply, DC-rated branch power cutoff, hardware one-hot solenoid pulse limiting, fused feeds and local independent contact/current readback. None has passed purchase approval.

## Status
Hardware modules not purchased; no physical assembly, watchdog/contactor selection, target enabled-output compilation, RF field tests or on-water acceptance. **Boat output firmware remains non-actuating and official VER014 unchanged**. Sonar postponed; Remote integrated SD is the only map/waypoint SD.
