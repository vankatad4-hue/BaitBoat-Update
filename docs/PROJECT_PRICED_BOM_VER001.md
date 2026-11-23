> **ACTIVE DESIGN DECISION — 2026-11-23 (supersedes earlier hardware-safety requirements):** The owner explicitly excludes dedicated independent hardware failsafe electronics and hardware-safety acceptance/signoff tests from the Flytec V080 project scope. Do not propose, require, purchase, or block software development/release planning on external safety cutoff relays/contactors, external watchdog/power-cut modules, independent current-zero sensors, hardwired STOP safety monitor, or independent coil limiter. Implement STOP, output-off, radio-loss RTH/STOP, low-battery response, watchdog recovery, one-coil-at-a-time and pulse limits in software with the existing ESP32, MCP23017, MDD20A and EKM014. Retain functional firmware unit/integration tests, PlatformIO builds, RF interoperability and ordinary electrical bring-up checks; these are not dedicated hardware failsafe tests. Do not describe software STOP/ACK, GPIO readback or watchdog reset as proof that physical current is zero. Previous R30H-10/R30H-12 hardware-safety gates and R30H-27 independent-feedback signoff are **RETIRED / NOT APPLICABLE BY OWNER DECISION**, not passed. Historical files remain archived as historical context and do not override this decision. No physical hardware tests have been run. Official Boat VER014 / Remote VER028 unchanged.

# Flytec V080 — priced modular procurement BOM (VER001)

**Snapshot:** 2026-11-23 | **Development:** `platformio-unified-r30h20` | **Currency:** EUR, provisional gross buyer-facing values | **Release version unchanged:** Boat VER014 / Remote VER028.

## Summary
| Scope | Included line items | New purchase subtotal |
|---|---:|---:|
| Boat components / packaging / wiring, excluding owned parts | 27 (including retained/option rows) | **€346.47** |
| Remote electronics / controls / pack / case | 25 (including option) | **€389.71** |
| **Dedicated hardware failsafe items (removed by owner decision)** | **0** | **OUT OF SCOPE** |
| Sonar / future rebuilt Boat pack options | 4 | **EXCLUDED** |
| **Base subtotal of currently priced selected/candidate/budget rows** | **56 total rows in register** | **€736.18** |
| 15% planning reserve (not a vendor quotation) | | €110.43 |
| Base plus 15% reserve | | €846.61 |
| 25% planning reserve | | €184.05 |
| Base plus 25% reserve | | €920.23 |

[**Current full 56-row BOM with supplier URLs, exact/candidate part references, unit and extended pricing, assumptions, owned items and missing quotes**](procurement/Flytec_V080_Full_BOM_Prices_2026-11-23.csv).

**Pricing limitation:** A mix of previously recorded shop listings, rough estimates and illustrative EUR/VAT conversions. This is not a fresh checkout or stock reservation. Confirm exact SKU, Bulgarian delivery/VAT, shipping, availability, RF legality, power/fit and pack protection before any order. `BUDGET` is not `SELECTED`; `CANDIDATE` is not approved procurement; `TBD` means **no cost estimate at all**. The €736.18 base is **not a full safe build price**.

## Hardware baseline and precedence

Use **the latest R20+ purchase/hardware register** and `platformio-unified-r30h20` wiring revisions, not the earlier Heltec Wireless Tracker concept: current Boat **ESP32-S3-DevKitC-1-N32R16V + HGLRC M100 PRO/QMC5883L**; GY-91; Cytron MDD20A; Electrokit EKM014; nRF24L01+ PA/LNA primary; EBYTE E22-900T22S EU868 UART backup in *both* Boat/Remote with qualified factory carrier; Olimex DCDC6-16-TO5; TRACO TSR 2-2450 (EKM VL only); two Pololu D24V10F3 3V3 rails; INA231 BUS voltage only; MCP23017 I/O. Remote is the selected Waveshare ESP32-S3 Touch LCD 4.3 Standard with 16 face buttons, an independent confirmation switch candidate, 2 dual-axis joysticks, 1S4P battery and microSD. 3D case, regulators and remote expanders retain candidate/fit gates.

**Do not duplicate** retained Boat V080 shell, two brushed 550 motors, two hopper solenoids, two hook solenoids or existing 3S4P battery/BMS in purchase cost unless inspection fails. Keep their quantities for build planning.

**Dedicated independent hardware failsafe devices and hardware-safety acceptance tests are OUT OF SCOPE by explicit owner decision.** Reference [R30H-27 feedback provenance and physical measurement procedure](../Boat/docs/BOAT_R30H_27_STOP_CURRENT_ZERO_BENCH_PROCEDURE.md). Status remains R30H-10 12 NOT_RUN / 6 BLOCKED and R30H-12 7 NOT_RUN / 5 BLOCKED. Software STOP/ACK is not independently measured current-zero.

**Sonar:** Cerulean S500 appears as **optional, not selected**, price €1,116.95 for comparison. No sonar hardware or interfacing is in base subtotal.

## Register/version discipline

- `docs/procurement/Flytec_V080_Full_BOM_Prices_2026-11-23.csv` is the **current priced procurement snapshot**, not an order receipt.
- `docs/PROJECT_PURCHASE_REGISTER_R20.csv` remains historical **supplier/selection evidence**; do not overwrite its observations or prior physical gates.
- `docs/PROJECT_BOM_SHOPPING_R1.md`, older remote BOMs and earlier Heltec candidate notes remain archival/historical design references.
- Any fresh vendor price, part substitution or new purchase status must update the same canonical CSV plus this summary, project checkpoints and changelog, with the new date/revision. The design releases **Boat VER014/Remote VER028 remain frozen** until physical acceptance.

**Next work:** obtain priced, compatible off-the-shelf independent safety hardware (RFQ), select exact remote charger/protection and radio carriers, then build a truly **complete** total including taxes, delivery and calibrated current-zero tests.
