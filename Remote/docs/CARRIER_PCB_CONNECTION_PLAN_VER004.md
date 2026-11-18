# Flytec V080 Remote — Carrier PCB Connection Plan (ver004)

**Status:** approved modular architecture; logical connector plan, not a released PCB pinout.

## 1. Architecture

Retain the original Flytec V080 remote enclosure and replace its electronics. Use a small central carrier/interconnect PCB with removable cables for serviceable modules:

- Main display/controller: Waveshare ESP32-S3-Touch-LCD-4.3 Standard (SKU 25948), 800 × 480, ST7262 RGB, GT911 touch, CH422G expander.
- Command radio: separate nRF24L01+ PA/LNA module, matching the boat-side module.
- Joystick(s): separate replaceable analog joystick module(s).
- Physical buttons: separate button/E-stop switch harness.
- Power module: battery pack, USB-C charging/protection and regulated supply, kept as a replaceable module.
- Optional expansion connector reserved for future low-voltage accessories.

The Waveshare board itself remains the processor/display module; do not design a second processor carrier around it. The remote carrier PCB distributes external controls, radio and power connections.

## 2. Existing Waveshare resource constraints

The following resources are already occupied or multiplexed on the selected standard board and must not be reassigned to external controls:

- RGB LCD: GPIO 5 DE, 3 VSYNC, 46 HSYNC, 7 PCLK; RGB data GPIO 1/2/42/41/40, 39/0/45/48/47/21, 14/38/18/17/10.
- GT911: GPIO4 IRQ, GPIO8 SDA, GPIO9 SCL; CH422G EXIO1 touch reset.
- CH422G EXIO2 backlight; EXIO4 TF-card chip select; EXIO5 USB/CAN selection.
- TF SPI: GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO.
- GPIO8/9 shared I²C; GPIO15/16 multiplexed RS485/CAN; GPIO19/20 USB D-/D+.

These are known board resources, not an exhaustive declaration that every other GPIO is available. The actual Waveshare schematic, Arduino board definition, boot/strapping behavior and firmware use must be checked before any external GPIO allocation.

## 3. Power architecture

| Rail | Intended load | Rule |
|---|---|---|
| BAT_1S | Protected 1S2P 18650 pack (proposed) | Pack, cells, protection and charger must be designed as one matched system. |
| USB_C_IN | Charging input | Use a dedicated USB-C charge/protection design appropriate to the exact cells and board; do not parallel charger outputs. |
| REMOTE_SYS | Regulated supply accepted by Waveshare board | Confirm the exact board input path and voltage/current limits before selecting regulator. |
| RADIO_3V3 | nRF24L01+ PA/LNA | Dedicated low-noise 3.3 V regulator with local bulk/high-frequency decoupling; do not assume the Waveshare 3.3 V rail can supply PA/LNA peaks. |
| JOY_3V3 | Joystick potentiometers and button logic | Clean 3.3 V reference; joystick outputs must remain within ESP32 ADC input limits. |

The proposed two 18650 1200 mAh cells in parallel form approximately 1S2P / 2400 mAh. This exceeds the previously noted Waveshare recommendation of below 2000 mAh for its battery connector. Therefore do **not** connect this pack to the onboard battery connector unless Waveshare confirms the charger/connector is suitable. Prefer a separately engineered, protected battery/charger/power module feeding a verified board power input. Confirm cell matching, protection, charge current, thermal management and enclosure ventilation.

## 4. Proposed remote carrier connector schedule

Connector families and pin order are logical proposals. Final connector series, pitch, current rating, pin numbers and wire gauge follow the mechanical design and board schematic review. Use keyed/locking connectors and unique families for different voltage domains.

| Ref | Module / function | Logical pin order (pin 1 onward) | Preliminary connector |
|---|---|---|---|
| R-J1 | Main power module to remote carrier / Waveshare input | 1: REMOTE_SYS_POS (verified voltage); 2: GND | Locking 2-pin, current-rated |
| R-J2 | nRF24L01+ PA/LNA radio | 1: RADIO_3V3; 2: GND_RF; 3: SCK; 4: MOSI; 5: MISO; 6: CSN; 7: CE; 8: IRQ (optional) | Locking 8-pin; short cable |
| R-J3 | Joystick module A | 1: JOY_3V3; 2: GND; 3: X_OUT; 4: Y_OUT; 5: SW (optional) | Locking 5-pin |
| R-J4 | Joystick module B / second control | 1: JOY_3V3; 2: GND; 3: X_OUT; 4: Y_OUT; 5: SW (optional) | Different keying from R-J3 if electrical role differs |
| R-J5 | Physical buttons | 1: GND_BUTTON; 2: STOP_NC (normally closed loop); 3: MODE; 4: HOME; 5: BAIT; 6: HOOK; 7: SPARE | Locking 7-pin; exact button set subject to UI/control design |
| R-J6 | Optional buzzer / status indicator | 1: GND; 2: BUZZER_CTRL; 3: STATUS_LED_CTRL | Locking 3-pin; driver transistor if required |
| R-J7 | Expansion | Reserved low-voltage GPIO/UART/I²C only after pin audit | Keyed, no power pins until assigned |
| R-J8 | Service / debug | Reserved GND, UART_TX, UART_RX, RESET only after pin audit | Separate keyed 4-pin; avoid conflict with USB |

**No ESP32 GPIO numbers are assigned by this table.** The logical connector pin order is not a Waveshare board pin map.

## 5. Signal and safety notes

### nRF24L01+ PA/LNA
- Use SPI plus CE/CSN; IRQ optional.
- Supply from dedicated RADIO_3V3. Confirm exact PA/LNA board peak current and decoupling requirements.
- Keep antenna clear of the operator's hand, battery, display metalwork and switching regulators.
- Use same channel, data rate (initially 250 kbps), address, CRC, auto-ack and retry profile as boat.
- Verify legal EIRP for the exact antenna/module combination in Bulgaria/EU.

### Joysticks
- Use 3.3 V analog reference and ensure X/Y outputs never exceed the ESP32 ADC input range.
- Add input filtering and suitable series/protection components at the carrier.
- Joystick connector pinout must include a clear orientation/key and pin 1 marking.
- Calibration currently held in firmware RAM; persistent calibration storage and actual ADC GPIO assignment remain future work.

### Physical STOP and buttons
- STOP should be a normally closed physical circuit where practical, so a broken wire is detected as an open circuit.
- Remote STOP is a request and status latch; the boat must independently enforce its own watchdog and actuator inhibit. The remote button alone is not the boat's safety system.
- Buttons should use defined pull-ups/pull-downs and debounce; do not leave inputs floating.

### Power module
- Keep charging, cell protection and system regulation on a replaceable module with documented cell polarity and test points.
- Use matched cells of the same type, age and state of health; do not parallel mismatched cells.
- Provide fuse/protection close to the battery and strain relief for USB-C and battery wiring.
- Verify actual Waveshare input voltage, startup current and brownout behavior before choosing the regulator.

## 6. GPIO allocation policy

No external GPIO numbers are approved in ver004. Before assigning them:
1. confirm the exact Waveshare standard-board revision and schematic;
2. check every GPIO already used by RGB LCD, GT911, CH422G, TF, USB, RS485/CAN and current firmware;
3. check ADC-capable pins, boot strapping, input-only limitations and startup levels;
4. check the chosen joystick count, physical buttons, radio SPI and any future peripherals;
5. confirm no external connector can drive a pin while the Waveshare board is unpowered.

## 7. Harness and mechanical rules

- Keep the Waveshare display/controller removable without desoldering the carrier PCB.
- Use locking connectors, cable strain relief and pin-1 labels at both ends.
- Use separate connector families/keying for battery, radio, joystick and button harnesses.
- Keep nRF antenna and radio supply away from display flex/ribbon, USB and switching regulator noise.
- Prevent joystick harnesses from crossing or loading the display/touch connections.
- All battery/module replacement is performed with USB disconnected and system power off.
- Reserve space for connector access and service loop; verify enclosure closure and original LCD aperture before fabrication.

## 8. Measurements / gates before PCB release

- Exact Waveshare board revision/schematic and available GPIO/ADC pins.
- Verified input voltage/current range and battery/charger recommendation for the chosen 1S2P pack.
- Exact nRF24 PA/LNA variant and measured peak supply current.
- Number/type of joysticks and physical buttons; ADC range and mechanical dimensions.
- STOP switch contact arrangement and safety semantics.
- Carrier PCB outline, mounting points, connector height and original enclosure clearance.

## 9. Implementation status

This is a hardware connection plan only. It does not add radio drivers, GPIO assignments, joystick ADC implementation, physical button handling, battery charging circuitry or new operational firmware. Existing remote software still contains a non-operational radio stub and joystick ADC placeholder. Do not treat the remote as a working boat controller until physical hardware, firmware and safety tests are complete.
