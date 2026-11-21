# R62 — Selected-endpoint code audit, fixes and remaining integration gates

**Review date:** 2026-11-22. **Branch:** `hardware-ready-dual-radio-r1`.
**Hardware endpoints confirmed by owner as selected:** Boat Espressif **ESP32-S3-DevKitC-1-N32R16V** (32MB octal flash, 16MB octal PSRAM) and Remote **Waveshare ESP32-S3-Touch-LCD-4.3 Standard**. This is a model decision, **not** proof modules are purchased, electrically connected or commissioned. Prior user report: modules still to be purchased.

## R62 source improvements (reviewed actual files)
| Area | File(s) | Change |
|---|---|---|
| Remote backup link | `Remote/source/dual_radio_manager.cpp` | Ignore invalid link identifiers; reject sending after authenticated link freshness expires even if `tick()` has not run; consume the 3-second backup steering slot only when the physical transport send succeeds. Preserve safety restrictions on backup releases. |
| Remote tests | `Remote/tests/dual_radio_manager_r62_test.cpp` | Invalid LINK_NONE spoof, failed-send retry and successful-send pacing, 25% backup steering cap, stale link and authenticated primary recovery. Added to `remote-host-tests.yml`. Fixed required `<cstring>` include surfaced by CI. |
| Boat calibration | `Boat/source/figure8_calibration_record.h` | Compute the version-1 CRC over exact 48-byte little-endian binary32 fields; no compiler padding or CRC-member dependence. Refuse incompatible struct sizes; retain 48-byte record layout. Golden CRC regression in `Boat/tests/figure8_calibration_record_test.cpp`. |
| Boat NVS diagnosis | `Boat/source/firmware/Flytec_Boat.ino` | Read and CRC-validate previous NVS calibration slots at boot, report status. **Never auto-apply calibration** or enable outputs. |
| Boat endpoint identity | `Boat/source/boat_endpoint_memory_gate.h`, diagnostic sketch, host test | Report detected flash and PSRAM and compare with 32MB/16MB selection. Not a physical pin audit or actuator authorization. |
| Boat E22 UART | `Boat/source/e22_uart_link.cpp` | Bound each cooperative UART parser pass to 32 input bytes, preserving primary radio and independent watchdog service opportunities. |
| Boat target | `.github/workflows/boat-target-build.yml` | Select `FlashSize=32M,FlashMode=opi,PSRAM=opi`, then explicit diagnostic-only `PartitionScheme=huge_app` because pinned esp32 core 3.0.2 is missing its default `app5M_fat24M_32MB.csv` partition file. **huge_app has no OTA**; do not use it as a production OTA plan. |

## Verification — GitHub Actions confirmed
- **PASS — Remote portable host tests**, run [44956981643](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44956981643), source SHA `bd176630`; verifies Remote radio regression and existing map/lake/telemetry/mission modules.
- **PASS — Remote Waveshare full target compile**, run [44956981581](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44956981581), source SHA `bd176630`; compilation is not hardware integration.
- **PASS — Boat portable host tests**, run [44957202055](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44957202055), source SHA `ec760833`; covers R62 memory gate, radio guard and CRC regression. **PASS — Boat safety/interface tests**, run [44956893866](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44956893866).
- **PASS — Boat selected N32R16V diagnostic target compile**, run [44957322196](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44957322196), source/config SHA `8d50c208`; explicit `PartitionScheme=huge_app`. This compiles a **NON-ACTUATING diagnostic binary**, not a physical driver firmware.
- Earlier build `44957190165` failed because Arduino ESP32 core 3.0.2 attempted to copy missing `app5M_fat24M_32MB.csv`. Fixed at `8d50c208`; retain the failure as historical, not current.
- Physical tests: **NOT PERFORMED**; no hardware purchased or commissioned.

## Still missing — ordered by enabling dependency
1. Complete safety BOM and wiring using **verified preassembled modules only**: independent physical STOP, 3S-compatible external watchdog/brownout, DC-rated branch isolation, one-hot non-retriggerable pulse cutoff and current/contactor feedback. Measure motor stall, solenoid pulse and supply transients before energizing. No custom PCB.
2. Confirm GPIO **physical header pins** and actual sensor/radio supply levels for selected Boat module and Waveshare. Freeze compliant, purchase-ready E22 pair: existing software chooses matching **E22-900T22S SMD** units, but ready-made carrier/adapter availability is unresolved; R60 D-V2 is **not** an approved substitute.
3. Implement and attach a real `BoatPhysicalDriver` to the Boat application only after independent safety hardware is commissioned, including motor PWM/DIR, four EKM014 outputs, verified contact/current sensor acquisition, and safe startup.
4. Finish verified radio key provisioning and session state persistence, genuine Remote/Boat RF interoperability, legal persistent EU868 airtime enforcement. A 3-second LoRa backup control period is not equivalent to a continuous short-leased manual-motor control link; fail-safe behavior must be tested before enabling backup steering.
5. Complete GPS/compass calibration quality gating and application of stored NVS candidate, authenticated Remote Start/Abort and GPS-guided figure-eight trajectory with geofence/STOP. Current boot operation is read-only diagnostics; no autonomy is commissioned.
6. Verify Remote physical input GPIO/SC16IS750 I2C bridge, nRF radio, LVGL/UI and **integrated Remote SD** map/waypoint persistence against real hardware.
7. Final complete target builds, end-to-end simulations, fault-injected hardware-in-the-loop, RF and on-water acceptance. Then and only then publish an operational Boat/Remote release or change official version markers.

**Release state:** Boat VER014 / Remote VER028 unchanged. R62 is an engineering branch revision, not a water-ready release. Sonar design deferred until model selection. No Boat SD module.
