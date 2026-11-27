# R30H-124W — sonar and environmental telemetry contract

**Branch:** `platformio-unified-r30h20`

**Scope:** Boat/Remote source preparation only. Map integration is postponed. No firmware release or hardware commissioning.

## Sonar decision

The preferred candidate is **Open Echo TUSS4470 + Lucky D+T 200 kHz / 45° wired transducer**, with a temperature output. It is not an approved final design: electrical compatibility, transducer pinout/characteristics, shallow-water behaviour, waterproof mounting, and the temperature interface remain unverified. Open Echo's shield guide describes an Arduino UNO interface (SPI, four GPIOs and analog A0) and says it is not currently pin/software-compatible with other Arduino boards. It also has a high-voltage transducer supply path. The ESP32-S3 adapter, analog level range, safe drive voltage and transducer match must be resolved before a driver or pin map is committed.

Boat accepts a normalized, compact depth summary through `TrackerSensorPort::publishSonarSummary`; that method does not read hardware or assume a vendor protocol. The Remote receives depth, range, health, alarm flags, and sample ID. It shows the depth only while fresh. The plot is an empty scale and explicitly says the module/protocol is unselected. It draws no synthetic arches or bottom trace.

The current radio is suitable only for compact measurements. Do not send raw CHIRP echo profiles over the control/telemetry links. A future sonar selection must provide lawful documented data access and recorded samples before any driver or echogram decoder is added.

## Water temperature

The Lucky D+T candidate advertises water temperature, but its electrical output, calibration and access through the transducer cable are not confirmed. Boat accepts already-converted Celsius through `TrackerSensorPort::publishWaterTemperature`, with freshness and a broad -5.00..50.00 °C range check. This is an acquisition boundary, not a transducer driver. Radio subtype 10 carries the signed centi-degree value and sample ID; Remote displays it separately from BMP280 sensor temperature and blanks it when stale.

## GY-91 / BMP280 pressure

The existing Boat BMP280 driver probes addresses 0x76/0x77, requires chip ID 0x58, decodes factory calibration, applies Bosch compensation, and rejects implausible pressure/temperature values. This change copies its fresh pressure and sensor temperature into `BoatHardwareSample` and the existing subtype-7 weather telemetry, so Remote continues to receive pressure in Pa and show hPa. The BMP280 temperature is the board's local sensor temperature; it is not water temperature, and pressure is not sea-level corrected.

## Dual-core ownership

- Keep the physical GY-91/BMP280 I²C reads, future water-probe reads, and future sonar interface reads on the existing Boat application owner until bus timing and target ownership are measured.
- Publish only copied, timestamped summaries to the future radio worker/transport queue. Do not let a second task access `Wire`, LVGL, motor state, or actuator outputs.
- Keep Remote UI mutation on its existing application task. The worker may receive/transfer authenticated bounded packets; it must not render sensor state.
- Sonar depth, water temperature, and BMP pressure can use the bounded telemetry queue. Full echo images remain local to a future Boat renderer or a separately designed data link.
- Production worker activation, RF gates, and hardware locks remain unchanged. This source work does not claim live sensor readings without the selected modules and wiring.

## Remaining dependencies

1. Bench-check the preferred Open Echo shield with Lucky D+T at reduced/controlled drive, verify ESP32-S3-safe logic/analog levels, interface timing, transducer loading and target depth range; obtain recorded real echo samples.
2. Determine whether the D+T water-temperature output is accessible and document its pinout, voltage, calibration and waterproof connector behaviour.
3. Connect the normalized sensor snapshots to the commissioned Boat sample owner, then run protocol/host regressions and both target builds.
4. After boards are available, verify GY-91 silicon/address, pressure against a reference, probe calibration, sonar depth against known distances, bus timing, and radio freshness on the actual units.
