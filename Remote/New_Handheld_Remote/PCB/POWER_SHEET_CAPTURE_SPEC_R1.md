# Remote Power Schematic Capture Specification R1

**Purpose:** A complete capture guide for native KiCad schematic sheets. This document is not a KiCad schematic, ERC report or fabrication output.

## Sheet 01 — Battery input/protection
- J_BAT: pack+, pack−, NTC.
- Pack/BMS protected positive → F_MAIN Littelfuse 0458004.DR (4A candidate) → Q_REV FDS6575 P-MOS source; Q_REV drain→BAT_PROT.
- Q_REV gate→100kΩ→GND. Confirm pin numbering and body diode direction in CAD.
- PACK−→GND.
- ADC divider: 220k BAT_PROT→ADC_BAT; 68k ADC_BAT→GND; 100nF ADC_BAT→GND.
- Temperature: 10k pullup from 3V3_CTRL to NTC_SENSE; NTC 10k B3950 candidate from sense→GND; 100nF sense→GND. Open/short detection required.
- Testpoints BAT_PROT, PACK+, GND, ADC_BAT, NTC_SENSE.
- Pack BMS/charger board are off-sheet modules; confirm wiring and ratings.

## Sheet 02 — 5V boost (TPS61023)
- U_DISPLAY TPS61023DRLR; pin map from TPS61023_PIN_NET_MAP_R1.md.
- BAT_PROT→VIN; GND→GND; L=1µH from BAT_PROT to SW; VOUT→+5V_MAIN.
- CIN 10µF from BAT_PROT to GND; supplemental 100nF local.
- COUT 2×22µF from +5V_MAIN to GND.
- RFB 732kΩ +5V_MAIN→FB; 100kΩ FB→GND.
- EN default ON via BAT_PROT pullup 100k candidate; verify datasheet.
- J_DISPLAY_POWER +5V_MAIN/GND; testpoints +5V_MAIN/GND.
- Load target 450mA continuous, 1A transient; thermal validation pending.

## Sheet 03 — Radio buck-boost
- U_RADIO TPS63070; net/pin details from TPS63070_PIN_NET_MAP_R2.md.
- BAT_PROT→VIN; RADIO_3V3 output to radio connector.
- L=1.5µH between L1/L2.
- Divider 470k VOUT→FB, 150k FB→GND.
- Input caps per documented draft; output bank per draft; VAUX 100nF.
- EN: MCU-controlled after boot, 1k series candidate and 100k pulldown; converter/radio OFF during reset. Verify EN threshold and GPIO. PG optional.
- J_RADIO: 3V3, GND, SPI SCK/MOSI/MISO, CSN, CE, IRQ; pin order TBD.
- Radio connector local 100nF + 10–47µF.

## Sheet 04 — Power interconnect/controller
- +5V_MAIN feeds Waveshare ESP32-S3-Touch-LCD-4.3 Standard documented power input.
- Controller 3V3 logic rail is not to be assumed equal to RADIO_3V3 converter output; document board's 3.3V output/source and ground.
- BOOST_5V_EN defaults enabled independent of MCU.
- RADIO_EN is defined and reset-safe.
- Testpoints on all rails and enables.

## Net classes and layout constraints
- BAT_PROT and +5V_MAIN power traces: width determined by current/temperature/PCB copper; transient 2A+ input path considered.
- RADIO_3V3 and RF control: avoid switching-node adjacency.
- Switching nets SW_DISPLAY and SW_RADIO: short, small copper; no testpoint stub unless needed for measurement.
- FB and ADC sense: quiet, filtered, away from switching nodes.
- Ground: continuous reference plane with power loop placement optimized; don't split into isolated islands without layout justification.
- Place testpoints where probe access is practical.

## Capture gates
- [ ] Library symbols match datasheets and packages.
- [ ] Pin numbers verified for every IC/FET.
- [ ] All power pins and unused pins treated correctly.
- [ ] Connector keying/pinout frozen.
- [ ] Electrical rules checker (ERC) clean or exceptions documented.
- [ ] Independent schematic review.
- [ ] PCB placement/routing review and DFM.
- [ ] Bench validation before release.
