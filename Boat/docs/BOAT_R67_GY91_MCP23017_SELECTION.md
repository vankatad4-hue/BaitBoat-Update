# R67 — Owner-confirmed Boat GY-91 and Waveshare MCP23017 expansion board

**Date:** 2026-11-22. **Branch:** `hardware-ready-dual-radio-r1`.
**Decision status:** **OWNER_SELECTED_MODEL, NOT PURCHASED / NOT PHYSICALLY VERIFIED**. Part-selection approval is **not** permission to connect actuators.

## Frozen choices

1. **GY-91**, genuine **MPU-9250 + BMP280**, **quantity 1** on **Boat**. This is the selected inertial/magnetometer/barometric sensor board, not an open candidate. Manufacturer of the specific GY-91 assembly and seller SKU remain unconfirmed. **MPU-9250 has a historical end-of-life notice**; genuine MPU-9250 identity and BMP280 identity/readings must be confirmed from a trusted seller and read-only bench inspection before commissioning. Do not silently replace with MPU-6500-only or BMP180/BME280, or a generic "GY-91" listing without proving both actual devices. Reference: [GY-91 assembly example](https://artofcircuits.com/product/10dof-gy-91-4-in-1-mpu-9250-and-bmp280-multi-sensor-module); [TDK MPU-9250 EOL](https://cdn.soselectronic.com/productdata/a0/a1/3079c447/mpu-9250.pdf).
2. **Waveshare MCP23017 IO Expansion Board**, **SKU 15391**, **quantity 1** on **Boat**. Owner's exact requested commercial product name matches [official manufacturer listing](https://www.waveshare.com/mcp23017-io-expansion-board.htm) and [official wiki](https://www.waveshare.com/wiki/MCP23017_IO_Expansion_Board). It is a **factory-made 38×23mm 16-GPIO I²C board**, not a bare MCP23017 chip, Adafruit board, custom carrier or new MCU. Manufacturer specifies 3.3V/5V compatible interface/board power and configurable A0/A1/A2 address bridges. **Default factory I²C address is 0x27** (all open/high on this board); the canonical software proposal will use **0x27** until verified otherwise. 0x20 requires intentionally shorting all 3 A0/A1/A2 on the physical Waveshare board; do not apply generic MCP23017 assumptions. Manufacturer lists a supplied six-pin PH2.0 cable; verify actual fitted connectors, I²C pull-ups and reset state.

## Integration proposal (not commissioned)

- Tentative Boat low-voltage I²C bus remains **ESP32-S3 GPIO8 SDA, GPIO21 SCL** from [R65 pin-budget crosswalk](BOAT_R65_ESP32S3_PIN_RESOURCE_CROSSWALK.csv); both are **proposal only**. Final J1/J3 physical pin revisions, 3.3V rail, pull-ups, bus wiring/harness and level compatibility require actual purchased modules.
- Sensor proposal at 3.3V: GY-91 MPU-9250 at I²C `0x68/0x69` (strap selection dependent), BMP280 at `0x76/0x77` (strap selection dependent), plus separate HGLRC M100 compass `QMC5883L` (often `0x0D`, actual variant to scan), and Waveshare expansion board at **`0x27`**. Do **not** mistake sensor I²C addresses for proof of identity.
- MCU sensor firmware already includes `Boat/source/tracker_sensor_port.h/.cpp` logic for IMU/BMP280 and M100/QMC compass. Module purchase does **not** imply successful firmware bring-up; read-only 7-bit I²C scan, MPU chip-ID/magnetometer identification, BMP280 ID/calibration, dual compass quality and performance checks are needed.
- **Intended use of four MCP23017 output requests:** assign provisional `GPA0` left hopper, `GPA1` right hopper, `GPA2` left hook, `GPA3` right hook, **only as low-voltage logical requests to a separately qualified factory-made one-hot, non-retriggerable timed safety controller**. The remaining twelve pins remain unassigned. No GPIO or expander output is connected directly to live EKM014 IN1..IN4, its VL/VH rails, motors or contactor.
- MCP23017 outputs are not independently fail-safe: on power-up/reset and lost I²C/MCU control, the receiving **separate safety hardware** must be OFF without relying on expander pull state, reset direction or firmware default. Verify physical pull-downs/input filtering through approved factory interface modules; fail-closed manual reset, max pulse, coil current-zero and STOP remain unresolved.
- If a 24V industrial safety-controller variant is used, both `3.3V → 24V` requests and `24V → EKM014 logic` must use verified **preassembled** interface modules. There is no authorized 24V signal wired directly to MCU/expander/EKM014.
- **Boat-only selection** of Waveshare 15391 does not change the Remote's separate MCP23017 breakout choice; the Remote button-expander SKU is still a separate selection/compatibility item. Separate buses can reuse same I²C address.

## Purchasing and acceptance checklist

1. Locate authentic **GY-91 MPU-9250 + BMP280**, board pinout, stable 3.3V, real sensor IDs, official seller/delivery to Bulgaria and no counterfeit substitution.
2. Procure exact **Waveshare 15391**, verify PH2.0 harness pin order, I²C `0x27`, 3.3V level on external expansion pins, physical RESET and pull-up behavior. Prefer manufacturer/authorized BG or reputable EU electronics sellers. Seller stock/shipping to Bulgaria **not yet confirmed**.
3. Read-only sensor scan; read WHO_AM_I and BMP chip ID; verify magnetometer and BMP calibration, bus arbitration and timing, possible address collisions, power-up state, sensor freshness and failure detection.
4. Keep **MDD20A/EKM014 physically powerless** pending independent emergency STOP, contactors, watchdog, current-zero and one-shot interlock verification, plus all R66 dummy-load acceptance tests.

**Release hold:** firmware and GPIO commissioning flags unchanged; diagnostic Boat target remains non-actuating. Official **Boat VER014 / Remote VER028** unchanged. No CHIRP sonar purchase or implementation.

## R73 correction: QMC5883L is on M100 PRO GPS module

HGLRC confirms that the **M100 PRO assembly includes onboard QMC5883L**, and its complete module needs **3.6–5.5 V**, therefore use **proposed 5 V GNSS supply**. Above historical R67 phrasing `plus separate HGLRC M100 compass` means a physically separate **GPS assembly**, not a separate QMC5883L PCB. No extra compass module quantity or 3.3 V sensor-rail current. GY-91 plus Waveshare MCP23017 remain on their own owner-confirmed D24V10F3 3.3 V rail. The I²C QMC remains in bus topology, but check SDA/SCL pull-up voltage and GPS UART output before joining 3.3 V GPIO. See [R73](BOAT_R73_M100_PRO_INTEGRATED_COMPASS_POWER.md).
