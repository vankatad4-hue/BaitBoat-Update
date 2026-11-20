# Flytec Remote R6.3 — Preliminary Power Architecture

**Status:** concept only. Do not select converter/charger/BMS parts or fabricate until current and thermal measurements are done.

## Known / reference loads

| Load | Supply | Known specification / estimate |
|---|---|---|
| Waveshare ESP32-S3-Touch-LCD-4.3 | 5V USB input, or its 1-cell battery interface | Manufacturer lists 5V, 450mA power consumption; board includes a lithium charge/boost manager (CS8501) and a 1-cell battery interface |
| E22-900T22S backup radio | 2.7–5.5V; 5V recommended for rated output | TX peak 150mA at 22dBm; RX 17mA; 3.3V UART logic |
| nRF24 PA/LNA primary radio | Exact module-specific rail TBD | PA/LNA breakout current and supply decoupling depend on the exact purchased board; do not assume a bare nRF24 current value |
| Joysticks/buttons | 3.3V logic | GPIO/ADC input only; verify pull-ups and analog limits |
| USB-C / battery charge | 1S pack | Board charger capability and charging-path behavior need verification before using it for a 1S4P pack |

## Proposed rail separation

1. **1S4P protected battery pack** → central battery input with pack fuse and keyed connector.
2. **Display rail:** initially evaluate the Waveshare board's own 1-cell battery input/boost/charge system. Verify its charging and power-path behavior in the official schematic before relying on it.
3. **5V radio rail:** dedicated boost regulator for E22, sized for 150mA transmit peak with headroom, low ripple, and transient response. Preliminary target is **5V / 1A continuous, 1.5A peak** pending measurement—not a selected part.
4. **3.3V radio/logic rail:** separate high-current, low-noise regulator for nRF PA/LNA and external logic. Exact output-current target depends on the selected radio breakout and load tests.
5. **Power distribution:** separate switched branches and local decoupling; radio bursts should not brown out display or ESP32. Add test points for battery, 5V, 3.3V, and radio current.

## Charge and protection decision

- Pack concept: 1S4P, four matched 18650 cells in parallel. If 2.5Ah cells are ultimately selected, nominal pack capacity would be 10Ah; this is not yet a confirmed pack.
- Use a pack protection system appropriate to 1S parallel cells (over-charge, over-discharge, over-current/short-circuit, and temperature protection); verify the pack's actual protection board and ratings.
- Do not blindly combine the Waveshare board's onboard charger with a separate charger. First decide which charger is used and confirm power-path behavior, charge current, thermal limits, and whether charging while operating is supported.
- USB-C input should have a defined charging-only/operating policy, connector rating, fuse/protection, and cable strain relief.

## Preliminary worst-case budgeting

The display's 450mA at 5V is about 2.25W. E22 TX at 150mA and 5V is 0.75W. Together, these two documented loads alone are approximately 3W at 5V, before nRF PA/LNA, conversion losses, or transient margin. A 1S pack will draw more than 1A under this combined load at low battery voltage; final design must include converter efficiency and thermal testing.

## Required next steps
1. Confirm exact Waveshare revision and review its schematic/charge-manager circuit.
2. Select and measure the exact nRF24 PA/LNA board.
3. Verify E22 version and antenna connector; use 868MHz configuration for the European band and comply with local limits.
4. Calculate rail loads at low battery voltage, including radio transmit duty cycle and screen brightness.
5. Choose converter(s), fuse, pack protection, charge current and thermal solution.
6. Test brownout, radio TX, full-brightness display, USB charging and operating-while-charging behavior on bench before integrating.

## Manufacturer references
- Waveshare official product documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3
- Ebyte E22-xxxT22S series user manual: https://www.ebyte.com/Uploadfiles/Files/2024-4-12/2024412165254103.pdf
