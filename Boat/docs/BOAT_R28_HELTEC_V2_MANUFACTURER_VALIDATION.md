# R28 — Heltec Wireless Tracker V2 manufacturer schematic validation
Date 2026-11-21. Official docs reviewed. **Migration feasibility review, not an approved pinout or replacement.**

## Primary manufacturer sources
- Datasheet rev 1.1, March 2026, board V2: https://resource.heltec.cn/download/Wireless_Tracker_V2/Wireless_Tracker_v2_Datasheet/Wireless%20Tracker%20v2.pdf (pages 8–10 pin map/header)
- Schematic HTIT-Tracker-FEM_V2.2: https://resource.heltec.cn/download/Wireless_Tracker_V2/Schematic/HTIT-Tracker-FEM_V2.2.pdf
- Official product documentation: https://wiki.heltec.org/docs/devices/open-source-hardware/esp32-series/lora-32/wireless-tracker-v2/

## Confirmed functions
ESP32-S3FN8, integrated SX1262 and UC6580 GNSS, TFT, 5V header input and 3.3V logic, CN3165 1S battery/solar charge circuitry. **Never connect 3S boat battery to 1S battery header.** Use separately qualified ready-made 3S-to-5V regulator and ground/reference planning.

| GPIO | Manufacturer documented use | Disposition |
|---|---|---|
| 1,2 | VBAT ADC and ADC enable | Reserved |
| 3 | VEXT control | Reserved |
| 4 | PA_CSD | Reserved RF |
| 5 | PA_CTX | Reserved RF |
| 7 | VFEM control | Reserved RF |
| 8–14 | Integrated SX1262 SPI/controls | Reserved RF |
| 15,16 | 32 kHz XTAL | Reserved |
| 18 | LED write control | Reserved until tested |
| 19,20 | USB | Reserved |
| 21 | TFT backlight | Reserved |
| 33,34 | GNSS UART | Reserved |
| 35,36 | GNSS reset/PPS | Reserved |
| 38–42 | TFT | Reserved |
| 43,44 | UART console | Reserved until boot/debug plan |
| 45,46 | Strapping / boot sensitivity | Reserved |
| 47,48 | XTAL_P/XTAL_N per V2 datasheet | Reserved |
| 0 | USER_SW boot | Reserved |
| 6,17,37 | No onboard net function named in pin map | **Candidate only; verify electrical/boot and external header** |

Caution: datasheet GPIO labels are silicon pad numbers, not evidence of safe use in every power/boot state. RF-front-end GPIO4/5/7 must not be reallocated.

## GPIO feasibility
R26 separate nRF SPI (3) + CE/CSN (2) + I2C (2) + motor PWM (2) = 9 native GPIO. Only 3 straightforward candidate pins identified on V2 from conservative manufacturer pin map (GPIO6/17/37). This is a **minimum shortfall of six** if the candidates are confirmed, before motor DIR, solenoid requests, sensors, feedback. Even a premade I2C-SPI bridge needs 2 I2C + CE + 2 PWM = 5 pins, a **shortfall of two**. A redesigned COTS topology, pin reallocation validated by manufacturer, or explicit user-approved controller change is needed. Do not claim R27 SC18IS602B alone resolves V2.

## Radio and power implications
- SX1262 LoRa and GNSS retained, but V2 high-power front end requires board-specific PA_CSD/PA_CTX/VFEM management and RF region/power review.
- Remote E22 UART-modem interoperability remains unproven. Backup link disabled.
- V2 1S charging/battery connector is **not** a 3S pack power input. Use a separate qualified 5V power converter, preserve actuator STOP isolation.
- nRF24 3.3V independent filtered supply remains necessary.
- MDD20A and EKM014 remain owner-selected, with independent hardware STOP, one-hot nonretrigger pulse, POR and current-zero modules unresolved.
- No custom PCB or second microcontroller.

## Decision
**Do not automatically migrate.** V2 is manufacturer documented but appears to have a tighter usable GPIO budget than v1.2 due to RF PA controls. Mark V2 as *evaluated, not adopted*. Current selected v1.2 retained provisionally; all GPIO output macros disabled. To adopt V2, require proven exact GPIO topology and a commercially premade module strategy with testable control timing and safety.

## Next gates
1. Ask Heltec to confirm whether any additional pins are safely usable on V2 with SX1262 high-power RF/GNSS/TFT enabled.
2. Re-evaluate controller topology without custom PCB/second MCU; quantify external SPI access and safe motor PWM.
3. Obtain exact available premade bridge/expander SKUs and supplier confirmation, then bench test.
4. Independently qualify STOP, contactor, onehot pulse, POR and current-zero safety modules.
5. Target compile and physical acceptance; Remote VER028/Boat VER014 unchanged; sonar deferred.
