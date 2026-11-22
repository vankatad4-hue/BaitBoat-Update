> **CURRENT R30H-10 REMOTE GPIO TEST HANDOFF — 2026-11-22:** [Waveshare Standard SKU25948 resource map](docs/REMOTE_R30H_10_PIN_MAP.csv) is the current source-only reference. The old nRF **CE=GPIO6 / CSN=GPIO15** proposal is **rejected**: the live RF config uses **CE=-1 and CSN=-1** (unassigned), because GPIO6 belongs to SD dummy-CS/Sensor AD and GPIO15 to onboard RS485. TFT/SD share SPI GPIO11–13 and CH422G EXIO4 currently asserts actual TF CS; per-transaction bus arbitration and a reviewed direct nRF control interface remain prerequisites. I²C GPIO8/9 with MCP23017 buttons 0x20/OK 0x21, ADS1115 0x4A and SC16IS750 E22 UART 0x48 remain candidate read-only peripherals. [Remote host CI #45066383423](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066383423) passed; Remote PlatformIO job in [#45066383457](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/45066383457) PASSED for the GPIO revision. Upload/RF/STOP still disabled, VERSION 028. [Joint review](../docs/R30H_10_GPIO_RESOURCE_EVALUATION.md). This supersedes the older proposed nRF assignments in the next banner.

# Flytec V080 Remote — active hardware and firmware

> **R30H-10 development authority (2026-11-22):** **VS Code + PlatformIO is now PRIMARY** for Remote: `pio run -d Remote -e remote_waveshare_43_diagnostic` (from repository root). Arduino CLI remains a secondary regression. [Workflow](../docs/PLATFORMIO_PRIMARY_WORKFLOW_R30H10.md) and [physical signoff plan](../Boat/docs/BOAT_R30H_10_HARDWARE_ACCEPTANCE.md). **Upload remains denied.** The proposed nRF CE **GPIO6** conflicts with SD dummy-CS, and nRF CSN **GPIO15** conflicts with onboard RS485 TXD. RF/STOP/PPS remain unsigned; no change to VER028.


**Selected controller:** Waveshare ESP32-S3-Touch-LCD-4.3 Standard, 800×480.  
**UI:** LVGL 9.6, Bulgarian labels.  
**Boat hardware baseline:** R20, documented in [Boat Hardware Selection R20](../Boat/docs/BOAT_HARDWARE_SELECTION_R20.md).  
**Project status:** [Remote master status](docs/PROJECT_MASTER_STATUS.md).

## Active hardware

- Waveshare ESP32-S3 touchscreen board.
- nRF24L01+ PA/LNA primary Boat-control link.
- E22 LoRa backup link.
- Two joysticks, 16 physical button positions, dedicated front power key, USB-C.
- 1S4P 18650 battery pack.
- SD card for offline maps and up to 16 waypoints.

The Remote's Boat-battery indicator is independent of the Remote's own battery. It displays Boat voltage and estimated percentage:
- Green: above 50%.
- Yellow: 21–50%, with `СРЕДНА БАТ. — ЗАРЕДИ`.
- Red: 20% or lower or the Boat's low-battery status flag, with `НИСКА БАТ. — ЗАРЕДИ/СМЕНИ` (RTH is added only when the Boat reports Home return); critical voltage displays `КРИТ. БАТ. — STOP`.
- Stale or missing Boat telemetry: `ЛОДКА: --`, muted grey.

Mission preparation is blocked if Boat battery telemetry is stale or estimated charge is 20% or lower. The Boat independently enforces its own low-battery gate and RTH; the Remote is not the safety authority.

Offline maps remain SD-backed; Google online tiles remain optional. Sonar selection remains deferred.
