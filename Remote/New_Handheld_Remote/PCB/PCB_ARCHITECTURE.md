# Modular PCB Architecture (Preliminary)

The PCB design is planned as replaceable modules with keyed connectors. Avoid a monolithic board where it complicates service or RF isolation. This is an architecture proposal, not a finished electrical schematic.

## Proposed modules
1. **MAIN / ESP32-S3 board** — MCU, display interface, USB data/programming, button inputs, joystick ADC interface, radio-control signals.
2. **POWER board** — USB-C input, 1S charging, battery protection/fuel-gauge sensing, power switch interface, regulated rails.
3. **INPUT board(s)** — joystick ADC conditioning and button matrix/scan circuits; front-mounted, removable.
4. **RADIO board** — nRF24L01+ PA/LNA with dedicated low-noise 3.3V supply, SPI and control lines; RF keepout.
5. **ESP antenna/Wi-Fi path** — external-antenna ESP32-S3 module/board variant; do not assume a PCB antenna can be externally connected without a supported RF connector/selection network.
6. **DISPLAY interface** — dedicated connector/adapter to selected 4.3-inch display; verify interface type and pin count before design.
7. **Optional expansion** — camera/light controls, vibration feedback, future accessory interface.

## Block schematic (logical)
```
USB-C
  |-- 5V input --> POWER/CHARGER --> 1S4P BATTERY (4.2V max)
  |                              |--> protected battery rail
  |                              |--> 5V boost (display/peripherals as required)
  |                              |--> 3V3 low-noise rail (ESP32 + nRF as designed)
  |
MAIN ESP32-S3 <--> DISPLAY connector
       |  |-- Button/joystick input connector(s)
       |  |-- nRF24L01+ PA/LNA: SPI + CE/CSN/IRQ
       |  |-- USB-C data/programming (native USB or validated interface)
       |  |-- Wi-Fi/Bluetooth external antenna via supported ESP32 RF design
       |  |-- battery measurement / status
       |  |-- optional vibration/LED
```

## Power/charging cautions
- USB-C charging current must be chosen based on cell datasheet, pack temperature and protection design.
- For a 1S4P pack, all cells must be compatible, matched and connected safely. A 1S pack does not need cell balancing but does require proper protection.
- The USB-C port must have correct CC resistors/charger behavior and ESD protection. USB data lines must be routed for programming if intended.
- Charging and powering the controller simultaneously requires a power-path design; do not assume a basic charger IC safely handles load sharing.
- Fuse/protection, reverse-current, thermal, regulator transient and brownout behavior require engineering review.

## RF cautions
- nRF24 and ESP32 Wi-Fi/Bluetooth both use 2.4GHz. Antenna spacing alone is not a complete coexistence solution.
- Maintain antenna feed/ground layout and RF keepouts per selected modules. Avoid battery, display cables, boost converter and switching nodes near antenna feed.
- Consider radio time-sharing/coexistence policy; simultaneous transmissions can desensitize each other.
- EIRP and antenna gain must comply with local regulations.

## Design deliverables still needed
- Actual component choices and datasheets.
- Full circuit schematics (KiCad).
- PCB layout and board stack-up.
- Connector pinouts, voltage/current limits, cable definitions.
- BOM and assembly drawings.
- Test plan, firmware bring-up and safety review.
