# Boat R22 — ADS1115 isolated battery-bench firmware and evidence capture

**Date:** 2026-11-22. **Status:** code/CI implementation, **no physical hardware test**.
**Current versions:** Boat VER014 / Remote VER028 (unchanged).

This is the next actionable step after [R21 ADC electrical commissioning](BOAT_R21_BATTERY_ADC_COMMISSIONING.md). It adds a **separate diagnostic sketch**, independent of the Boat mission firmware. It does **not** commission the actual ADC, GNSS, controller, battery, safety cutoff, motor driver or any solenoid.

## Files

- [ADS1115_Battery_Bench.ino](../hardware_test/ADS1115_Battery_Bench/ADS1115_Battery_Bench.ino): Arduino ESP32-S3 sensor-only serial CSV output.
- [boat_adc_bench_local.example.h](../hardware_test/ADS1115_Battery_Bench/boat_adc_bench_local.example.h): **disabled by default** private/local GPIO-audit example.
- [boat_ads1115_bench_math.h](../source/boat_ads1115_bench_math.h): portable uncommissioned ADS1115 raw reading decoder.
- [boat_ads1115_bench_math_test.cpp](../tests/boat_ads1115_bench_math_test.cpp): healthy full-charge, red threshold, invalid raw, invalid range regression.
- [verify_ads1115_bench_isolation_r22.py](../tools/verify_ads1115_bench_isolation_r22.py): checks the sketch is GPIO-audit locked, with no actuator/radio APIs.
- [R21 measurement worksheet](BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv): enter actual DMM, oscilloscope, raw code and pack-voltage readings, never generated or guessed.

## Strict power boundary

- **Disconnect MDD20A motor supply and BOTH brushed motors.**
- **Disconnect EKM014 high-current supply and all four solenoids.**
- No radio module, GPS or safety-control activation is required for this test. This bench sketch imports **no** Boat radio, mission, motor or release driver.
- Supply only the ESP32-S3 and 3.3V ADS1115 measurement circuit using a current-limited supply. No USB/external-regulator backfeeding.
- Use the R21 **150kΩ / 33kΩ / 100nF** switched measurement-divider candidate. The 100kΩ / 33kΩ R20 suggestion is superseded.
- **Before connecting the pack simulator sense feed**, measure VDD/AIN0 supply rise, decay and power-off behavior; the input must remain within the TI device's VDD-relative rating. The firmware cannot verify this.
- The ESP32-S3 GPIO8(SDA)/GPIO7(SCL) are **candidates**, not a production-approved pin map. Check exact DevKitC-1-N32R16V manufacturer revision/schematic and physical wiring before enabling I²C.

## Why it is deliberately locked

No `boat_adc_bench_local.h` is committed. The repository sketch defaults to:
```cpp
#define BOAT_ADC_BENCH_PIN_AUDIT_SIGNED 0
#define BOAT_ADC_BENCH_SDA_GPIO -1
#define BOAT_ADC_BENCH_SCL_GPIO -1
```
With those defaults it prints a lockout notice through USB serial but calls **no** `Wire.begin()` and does not configure the I²C pins.

After the exact physical board/pin audit and regulated-supply checks have been signed, make a **private, local** copy of `boat_adc_bench_local.example.h` as `boat_adc_bench_local.h`, set `BOAT_ADC_BENCH_PIN_AUDIT_SIGNED 1` and keep SDA8/SCL7 **only if they match the physical board**. Any other pair is rejected pending a revised GPIO map. Do not check the local signed file into Git.

This sketch still does **not** authorize motors, solenoids, Home/RTH or real mission hardware.

## Build / serial capture

The GitHub Actions [Boat target workflow](../../.github/workflows/boat-target-build.yml) stages the locked sketch separately and compiles it for the selected ESP32-S3 octal-flash/PSRAM target. Successful compilation only proves build compatibility.

For a **local Windows PowerShell** build after the physical checks:
```powershell
$root = (Get-Location).Path
$dst = Join-Path $env:TEMP 'BoatAdcBench'
New-Item -Type Directory -Force $dst | Out-Null
Copy-Item 'Boat/hardware_test/ADS1115_Battery_Bench/ADS1115_Battery_Bench.ino' "$dst/BoatAdcBench.ino"
Copy-Item 'Boat/source/boat_battery_divider.h','Boat/source/boat_ads1115_bench_math.h' $dst
# Only after the physical pin/supply audit and local sign-off:
# Copy-Item 'Boat/hardware_test/ADS1115_Battery_Bench/boat_adc_bench_local.h' $dst
arduino-cli compile --fqbn 'esp32:esp32:esp32s3:FlashSize=32M,FlashMode=opi,PSRAM=opi,PartitionScheme=huge_app' $dst
```
The local `arduino-cli` ESP32 core version must be `esp32:esp32@3.0.2` to match the current CI. Firmware upload remains a separate operator action. **Never flash a pin-enabled sketch until physical power checks pass.**

Serial baud **115200**. The first lines state the lock status and CSV header:
```text
ms,state,raw_code,ain_mV,estimated_pack_mV,reason,safety_authority
```
On an audited, electrically valid bench only, the readout initiates an ADS1115 AIN0 single-shot conversion every second at ±4.096V PGA, 128SPS, and checks the converter's ready/config bits before reading. Every line explicitly says `UNCOMMISSIONED`; `READING` denotes a **plausible raw observation only**, not calibrated voltage or battery safety approval. Error and rejected samples are reported as such; no old sample is silently reused.

**ADS1115-vs-ADS1015 limitation:** Both can answer at 0x48 and share a register layout. An ACK or an apparently valid sample alone **cannot** identify the silicon. Verify the chip/breakout identity from trusted part marking/datasheet, and check raw-code behavior against a calibrated varying input (ADS1015's conversion output is 12-bit left-justified). This readout does not attempt to claim chip identification.

## Physical acceptance record

1. Record the part markings, ADC address strap, GPIO pin audit, 3.3V measured supply, I²C pullups, DMM resistor values, and isolation of actuator power.
2. Measure pack simulator and ADC AIN0 at 9.0 / 9.9 / 10.2 / 10.8 / 11.4 / 12.0 / 12.6V; save the serial CSV unchanged.
3. Scope both ADC VDD and AIN0 during startup/power down; check VDD-relative limits and input back-power, **not** just steady-state values.
4. Enter actual raw ADC codes, DMM readings, gain/offset calibration residual and evidence filenames in [R21 worksheet](BOAT_R21_ADC_CALIBRATION_WORKSHEET.csv).
5. Test I²C disconnect/no-ACK, open sense line, implausible ADC codes and invalid conversions without applying dangerous input voltages.
6. Only after the data is reviewed may a separate **production** sensor profile be commissioned. That **still does not enable motor/solenoid outputs**: physical independent STOP/watchdog/release-idle feedback and motor/solenoid dummy-load acceptance are separate remaining gates.

**Known limitation:** A sensor diagnostic that prints plausible values does not certify battery remaining capacity or enough return-to-home energy under load. Loaded-cell sag, temperature, endurance and RTH reserve still require real testing.
