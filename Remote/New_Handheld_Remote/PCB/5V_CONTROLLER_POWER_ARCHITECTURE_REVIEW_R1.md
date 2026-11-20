# 5 V Controller Power Architecture Review R1

**Recommendation:** Use the Waveshare ESP32-S3-Touch-LCD-4.3 Standard board's onboard CS8501 single-cell battery boost as the primary 5 V supply; do not populate the external TPS61023 in the baseline unless bench testing shows the onboard supply is inadequate.

## Why this changes the earlier decision
The selected Waveshare board already provides a PH2.0 single-cell battery connector and CS8501 charge/discharge management circuit that boosts a single Li-ion cell to 5 V. The manufacturer lists the board's supply consumption as 5 V / 450 mA. This matches our 1S4P pack architecture and removes a redundant external boost converter, inductor and capacitor bank.

**Important limitation:** the CS8501's detailed output-current/thermal datasheet was not verified. The manufacturer's 450mA board consumption is a board-level planning figure, not a guarantee of transient capacity. Validate the onboard converter at maximum display brightness, ESP32 Wi-Fi activity, radio activity and low battery voltage. If it fails, TPS61023 remains a fallback alternative—not to be connected in parallel with the onboard boost output.

## Revised baseline power tree
- Protected 1S4P battery pack → main fuse and reverse-protection stage → board's battery input connector (PH2.0) and TPS63070 radio buck-boost input.
- Waveshare board's onboard CS8501 boosts battery to its 5V system rail.
- TPS63070 independently generates RADIO_3V3 for the nRF24 PA/LNA module.
- Pack BMS remains required. Charger use is outside this design phase; do not connect competing charge sources or assume the board's USB charger settings are suitable without review.

## Bench verification gates
1. Confirm board revision and battery connector polarity/rail from schematic.
2. Measure 5V board current and battery input current.
3. Test boot/startup and display backlight at maximum brightness.
4. Stress Wi-Fi/Bluetooth and radio TX while display active.
5. Test 4.2V, nominal pack, and low-voltage loaded condition.
6. Log 5V droop/ripple, reset/brownout, converter/board temperature and runtime.
7. If the onboard boost proves inadequate, re-open external TPS61023 fallback with a safe 5V injection path confirmed from board schematic.

## Earlier external boost design
TPS61023 calculation/pin-map files are retained as a fallback engineering study. They are not the baseline power tree unless the bench test warrants the extra converter.
