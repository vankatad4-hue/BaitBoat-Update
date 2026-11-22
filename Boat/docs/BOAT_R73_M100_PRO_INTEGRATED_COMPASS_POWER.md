# R73 — HGLRC M100 PRO onboard QMC5883L and GNSS 5 V supply correction

**Date:** 2026-11-22. **Decision:** OWNER CLARIFICATION ACCEPTED; verified against [official HGLRC M100 PRO product listing](https://www.hglrc.com/products/hglrc-m100-pro-gps), which explicitly identifies **QMC5883L onboard** and **3.6–5.5 V module supply input**. No separate QMC5883L board/module is required in the purchase list.

## BOM correction — no duplicate compass

- **1× HGLRC M100 PRO GPS** assembly includes both the GNSS receiver and onboard **QMC5883L compass**.
- **1× GY-91** (MPU-9250 + BMP280) contains its own magnetometer within the MPU-9250. This provides a *second* magnetometer source, but **NOT a second purchased QMC5883L**.
- **1× Waveshare MCP23017 IO Expansion Board SKU15391**, factory default I²C `0x27` remains unchanged.
- **2× confirmed Pololu D24V10F3** on Boat still selected: (1) 3.3 V E22 radio and (2) **3.3 V for GY-91 + MCP23017**, not the M100/QMC assembly. No independent compass module is to be bought.
- Existing master purchase-register HGLRC entry now says **'HGLRC M100 PRO GPS assembly with onboard QMC5883L'**. Do not add another QMC line.

## Power and bus split

Per manufacturer, M100 PRO **VCC input 3.6–5.5 V**. A 3.3 V D24V10F3 output is **below** this specified minimum and should **not** be used to power the M100 PRO board, even though the internal QMC5883L IC itself is a 3.3 V family device. The onboard compass is powered internally as part of the module, not by a second directly wired sensor rail.

### Current provisional physical power buses (NOT yet commissioned)
```text
3S battery and BMS
  |-- separately protected 3S -> Pololu D24V10F3 #1 3.3V -> EBYTE E22-900T22S only
  |-- separately protected 3S -> Pololu D24V10F3 #2 3.3V -> GY-91 + Waveshare MCP23017
  |-- separately protected 3S -> proposed Olimex fixed 5V -> ESP32-S3 DevKitC-1
  |                                                |-> HGLRC M100 PRO board incl QMC5883L
  |                                                |-> filtered 5V Ardboard nRF24 adapter 1127 (conditional)
  |-- separately protected 3S -> proposed Pololu fixed 5V -> EKM014 VL only
  |-- safety-qualified DC load cutoff -> MDD20A motors / EKM014 VH
```
- The two 5 V upstream functions **remain candidates**, not owner-confirmed. Verify combined Olimex 5 V / 2 A **ESP32 + M100 PRO + Ardboard nRF** startup current, cable drop, RF noise and temperature. If inadequate, choose a dedicated premade 5 V converter for M100 or radio rather than cross-feed the confirmed 3.3 V sensor supply.
- GNSS receiver communicates by **UART** (M100 TX/RX); its integrated QMC5883L compass communicates over **I²C** (SDA/SCL), potentially sharing the controller's provisional **GPIO8 SDA/GPIO21 SCL** bus (physical pin audit pending). Verify all six wires and their fitted harness pin order and actual board revision.
- **Crucial I/O caveat:** Manufacturer's **3.6–5.5 V module supply rating does NOT imply 5 V-tolerant ESP32-S3 digital I/O**. Neither QMC SDA/SCL nor GNSS TX can be connected if they are pulled/driven above 3.3 V. Determine exact built-in pull-ups and UART output amplitude using the powered module with no MCU pin attached; if any are 5 V, use a suitable certified/prebuilt **bidirectional I²C and appropriate UART level-shifting module**, not raw 5 V inputs. Avoid simultaneous power sequencing/backfeed issues between 5 V M100 and 3.3 V sensor bus.
- Manufacturer page: [HGLRC M100 PRO GPS official](https://www.hglrc.com/products/hglrc-m100-pro-gps) (listed QMC5883L, power input 3.6–5.5 V). Device variants/actual shipped harness must still be inspected in person.

## Acceptance gates (all unperformed)
1. Verify purchased M100 PRO label and actual QMC5883L presence, UART and I²C connector pinout, module 3.6–5.5 V rated VIN and real 5 V current.
2. Bench-only readback of M100 GPS UART and onboard QMC I²C address (typically `0x0D` but not blindly presumed); no 12 V, 3.3 V-only VCC feed or connected actuator.
3. Check SCL/SDA and UART TX high-level voltage with the M100 powered at approved 5 V before joining ESP32 3.3 V GPIO; qualified level conversion if required; prevent backfeed while MCU power is off.
4. Verify combined 5 V regulator budget and conducted noise with ESP32, M100 and filtered Ardboard adapter at actual radio power envelope.
5. Confirm separate 3.3 V sensor regulator supplies only GY-91 and MCP23017, with internal MPU magnetometer separate from GPS QMC; check both heading sources after magnetic alignment.
6. Confirm independent STOP/watchdog DC-rated cutoff and solenoid pulse interlocks before any actuator power.

**No new hardware purchase, no 5 V regulator owner lock, no commissioned GPIO/motor/coil driver, no release bump. Boat VER014 / Remote VER028 unchanged.**


## R74 — M100 PRO 5V source now selected

Owner confirmed **Olimex DCDC6-16-TO5** as main 5V converter (one unit initially). The full M100 PRO with integrated QMC5883L is included on this 5V rail alongside ESP32-S3. Second DCDC6 only if measured combined load/EMI warrants a separately conditioned branch. The dedicated EKM014 VL supply is now TRACO TSR 2-2450, not connected to M100 or MCU 5V output. M100 UART/I2C 3.3V logic level proof still pending. See [R74](BOAT_R74_5V_REGULATOR_SELECTION.md).
