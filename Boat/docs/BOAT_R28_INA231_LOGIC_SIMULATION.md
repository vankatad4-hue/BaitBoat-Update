# R28 — INA231 Boat sensor integration and simulated faults (logic-only)

**Date: 2026-11-22 · Prototype development only; no live hardware or actuator access.**

## Scope and what changed

R27 selected **Ardboard INA231 #1491** instead of the Boat ADS1115. R28 builds its first complete **transport-neutral acquisition + Arduino TwoWire bridge** into `TrackerSensorPort`, but keeps it behind `INA231_PHYSICAL_COMMISSIONED=false` and two distinct unsigned local board/calibration gates. The Boat cannot physically use INA231 yet and does not fabricate a battery reading. The selected module, wiring design, PCB-free architecture and R79 battery thresholds remain unchanged.

1. `Boat/source/ina231_acquisition_r28.h`: C++11 portable state machine with injected `RegisterBus`, explicit BUS-only **triggered** conversion config `0x4122` (MODE=010), 200ms sampling interval, new-conversion flag `CVRF` bit 3 at 0x06, arithmetic-overflow rejection, range/signed-bit checks, 40ms conversion timeout and 1000ms sample age. A cached BUS register or an incomplete conversion can NEVER refresh the sample timestamp. On any transport failure, bad config echo, math overflow or invalid value, validity immediately goes false; the next poll can retry without copying the stale value.
2. `Boat/source/ina231_arduino_wire_r28.h`: 16-bit big-endian I²C register operations using the already-selected Boat `TwoWire`. Constructing this adapter does **not** initialize GPIO or contact a chip. Caller must first pass the signed hardware gates.
3. `Boat/source/tracker_sensor_port.h/.cpp`: integrates the new sampler into the existing Boat battery voltage field and its real acquisition timestamp. `TrackerSensorPort::begin` requires `INA231_PHYSICAL_COMMISSIONED && cfg.pinAuditSigned && cfg.ina231BoardAndWiringSigned && cfg.ina231DmmCalibrationSigned` for INA231 enable. Since the compile-time physical commissioning constant is **false**, runtime returns invalid 3S battery telemetry; no voltage is invented. The historic ADS1115 fallback is unreachable when the R27 selected part is active.
4. `Boat/source/tracker_sensor_profile.example.h`: explicitly initializes board, DMM and pin signoffs `false`, leaves 0x40 only as an audited-after-purchase **candidate** I²C address. `Boat/source/boat_physical_driver_adapter.h` describes the INA231 sensor's local origin rather than obsolete ADS1115.
5. `Boat/tests/ina231_acquisition_faults_r28_test.cpp`: simulated chip bus: no commissioning/wrong address, bus-only 0x4122 framing, new conversion, unchanged stale-data timestamp, stalled CVRF, short/failed I²C, write NACK, readback mismatch, arithmetic overflow, MSB out-of-range, open/short-like 0 V, 9.9V R79 immediate critical STOP, invalid sample, 1-second expiration, 32-bit clock wrap. **No RF, electrical loads or water used.**
6. `Boat/tests/ina231_wire_r28_test.cpp` + `r28_stubs/Wire.h`: synthetic TwoWire calls verify big-endian 0x4122 register writes, two-byte reads, correct address and fault-handling without Arduino hardware.
7. `Boat/tools/verify_r28_prototype_gate.py`: static check of full software lock chain, 21 unsigned R27 physical check rows and existing independent output cutoff gate. Boat CI updated to compile/execute the two portable fault suites and run static assertions.

## Manufacturer-specific decision

TI INA231 Rev D §8.6.7: mask register `0x06` has **CVRF at bit 3**; reading it in triggered mode clears the conversion-ready event. TI §8.6.1 config MODE=010 is **BUS-only triggered**. TI §8.6.3 BUS register `0x02` is 1.25 mV/bit. R28 chooses triggered (`0x4122`) rather than R27's candidate continuous (`0x4126`) so each accepted reading is associated with a fresh conversion request and CVRF confirmation. R27 codec/host fixtures remain historical; the selection of INA231 itself is unchanged.

See [INA231 manufacturer datasheet](https://www.ti.com/lit/ds/symlink/ina231.pdf) and [R27 physical BUS-only pinout/acceptance](BOAT_R27_INA231_SELECTED_VOLTAGE_ONLY.md).

## Hard boundaries

- `INA231_PHYSICAL_COMMISSIONED=false`, board/wiring and DMM flags all false, and the physical output driver's independent safety commissioning flag still false. Neither the I²C adapter nor any host test grants an actuator permission.
- The factory INA231 module must be inspected first. Its **0.1Ω R100 shunt must never be inserted into the 20A pack/motor/coil circuit**. Only protected low-current BUS sense is proposed. Exact pad order, pull-up voltages and BUS-to-IC continuity are unverified.
- The **21 R27 physical checks, 28 R25 checks and 32 R23 checks** remain `NOT_RUN`. Independent DC STOP/watchdog/current-zero and one-hot hardware pulse-limiter remain separate physical blockers.
- Real Remote battery indication must remain **unavailable** until physical INA231 acquisition is qualified; no active Home return is promised on stale sensor data.
- These local host tests do not prove the current Arduino target compiles. New branch GitHub Actions runs require separate observation; no successful Actions result is claimed until verified.
- **Boat VER014 / Remote VER028 remain frozen**; no sonar module integrated.

## Next

Audit the exact INA231 PCB and assembled Boat ESP32-S3 revision on a logic-only/current-limited bench, verify address and 3.3V levels, then calibrate BUS voltage against a DMM. Record commissioning evidence, separately complete independent physical STOP/current-zero interfaces, and only afterward consider enabling the existing dormant sensor path or power to actuators.
