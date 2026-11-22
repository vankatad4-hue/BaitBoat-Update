# Flytec V080 Remote — active hardware and firmware

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
