# R30H-124W — sonar and environmental telemetry contract

**Branch:** `platformio-unified-r30h20`

**Scope:** Boat/Remote source preparation only. Map integration is postponed. No firmware release or hardware commissioning.

## Sonar decision

The preferred candidate is **Open Echo TUSS4470 + Lucky D+T 200 kHz / 45° wired transducer**, with a temperature output. It is not an approved final design: electrical compatibility, transducer pinout/characteristics, shallow-water behaviour, waterproof mounting, and the temperature interface remain unverified. Open Echo's shield guide describes an Arduino UNO interface (SPI, four GPIOs and analog A0) and says it is not currently pin/software-compatible with other Arduino boards. It also has a high-voltage transducer supply path. The ESP32-S3 adapter, analog level range, safe drive voltage and transducer match must be resolved before a driver or pin map is committed.

Boat accepts a normalized, compact depth summary through `TrackerSensorPort::publishSonarSummary`; that method does not read hardware or assume a vendor protocol. The Remote receives depth, range, health, alarm flags, and sample ID. It shows the depth only while fresh and plots up to 64 distinct fresh samples as a scrolling depth history. The vertical scale starts at 15 m and expands in 15 m increments when needed; loss of fresh data clears the trace. It draws no synthetic arches, fish, or bottom trace, and the plot is explicitly identified as a depth history rather than an echogram.

The Open Echo repository offers an Arduino NMEA DBT output sketch. `OpenEchoDbtDecoder` parses its checksummed serial `$??DBT` sentences, uses the meter field, and rejects malformed, out-of-range, or stale partial records. `TrackerSensorPort::feedOpenEchoDbtByte` publishes a depth-only summary with unknown range and degraded/unknown echo quality. `BoatOpenEchoUartPort` now connects the isolated Boat UART0 RX candidate to that bounded parser; it remains inactive in the default profile while the hardware pin audit is unsigned. The coprocessor has no DBT transmitter yet because it has no validated depth acquisition. The current upstream sketch computes depth with a fixed 1450 m/s sound-speed value and its DBT sentence carries neither measured water temperature nor raw echo amplitudes; shallow-water accuracy must be measured before trusting the displayed value.

Directly wiring the TUSS4470 shield to Boat is feasible in principle using a harness, without stacking it on the ESP32 or keeping an Uno as the production host. The shield exposes SPI, four control/status GPIOs, and VOUT to an ADC, but the Uno pin labels are not Boat GPIO assignments. The current Uno sketch requests 1 MHz SPI with a 16 MHz clock divider, above the TI datasheet's 500 kHz SPI maximum; its 5,224-sample capture also occupies about 69 ms at the stated sample interval. The shield's exact revision, digital/analog output levels, Boat pin conflicts, transducer power, and real-time capture behavior must be measured before a direct driver or pin map is added. See the [direct-wire feasibility gate](../Boat/docs/BOAT_OPEN_ECHO_DIRECT_WIRING_GATE_R1.md).

The current radio is suitable only for compact measurements. Do not send raw CHIRP echo profiles over the control/telemetry links. A future sonar selection must provide lawful documented data access and recorded samples before any driver or echogram decoder is added.

## Water temperature

The Lucky D+T candidate advertises water temperature, but its electrical output, calibration and access through the transducer cable are not confirmed. Boat accepts already-converted Celsius through `TrackerSensorPort::publishWaterTemperature`, with freshness and a broad -5.00..50.00 °C range check. This is an acquisition boundary, not a transducer driver. Radio subtype 10 carries the signed centi-degree value and sample ID; Remote displays it separately from BMP280 sensor temperature and blanks it when stale.

## GY-91 / BMP280 pressure

The existing Boat BMP280 driver probes addresses 0x76/0x77, requires chip ID 0x58, decodes factory calibration, applies Bosch compensation, and rejects implausible pressure/temperature values. This change copies its fresh pressure and sensor temperature into `BoatHardwareSample` and the existing subtype-7 weather telemetry, so Remote continues to receive pressure in Pa and show hPa. The BMP280 temperature is the board's local sensor temperature; it is not water temperature, and pressure is not sea-level corrected.

## Dual-core ownership

- Keep the physical GY-91/BMP280 I²C reads, future water-probe reads, and future sonar interface reads on the existing Boat application owner until bus timing and target ownership are measured.
- The current Boat loop now copies timestamped optional sensor summaries into `BoatApplicationPort`; if a commissioned driver is attached, that snapshot is merged into telemetry without entering actuator decisions. A future radio worker must receive only copied summaries through its bounded transport queue. Do not let a second task access `Wire`, LVGL, motor state, or actuator outputs.
- Keep Remote UI mutation on its existing application task. The worker may receive/transfer authenticated bounded packets; it must not render sensor state.
- Sonar depth, water temperature, and BMP pressure can use the bounded telemetry queue. Full echo images remain local to a future Boat renderer or a separately designed data link.
- Production worker activation, RF gates, and hardware locks remain unchanged. This source work does not claim live sensor readings without the selected modules and wiring.

## Remaining dependencies

1. Bench-check the preferred Open Echo shield with Lucky D+T at reduced/controlled drive, verify ESP32-S3-safe logic/analog levels, interface timing, transducer loading and target depth range; obtain recorded real echo samples.
2. Determine whether the D+T water-temperature output is accessible and document its pinout, voltage, calibration and waterproof connector behaviour.
3. Validate the existing DBT receiver/Remote summary path with exact-commit protocol regressions and no-flash Boat/Remote builds when verification is requested. The default firmware still has no commissioned `BoatPhysicalDriver`, so this path is prepared but does not claim production RF telemetry.
4. After boards are available, verify GY-91 silicon/address, pressure against a reference, probe calibration, sonar depth against known distances, bus timing, and radio freshness on the actual units.

## Open Echo references

- [Open Echo repository](https://github.com/Neumi/open_echo) — project overview, RAW Data and NMEA DBT firmware choices, and desktop echogram tools.
- [TUSS4470 shield guide](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/README.md) — UNO-specific interface and power notes.
- [NMEA DBT output sketch](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/arduino/NMEA_DBT_OUT/NMEA_DBT_OUT.ino) — serial depth output; this mode does not include temperature or raw echo intensity.
