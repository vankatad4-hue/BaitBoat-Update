# Remote Power Net Draft R7

**Status:** Capture-ready net definition candidate; no native schematic/ERC/PCB/bench validation. Sheet definitions are in SCHEMATIC_CAPTURE_SHEETS_R1.md.

## Battery/protection
J_PACK PACK+ → F_MAIN (4A fast fuse candidate) → PACK_FUSED → Q_REV Drain; Q_REV Source → BAT_PROT; Gate → R_GATE 100k→GND. PACK−→GND. Verify MOSFET pin/body-diode topology against current datasheet. BAT_PROT bulk candidates 100µF+10µF+100nF to GND. BAT_PROT branches to J_WAVE_BAT and TPS63070 VIN. Fuse close to pack.

## Controller power
BAT_PROT/GND → Waveshare PH2.0 battery interface, polarity to verify. Onboard CS8501 boost; external TPS61023 is fallback only. Do not parallel boosts.

## Radio converter
TPS63070 BAT_PROT→VIN; 1.5µH candidate between L1/L2; 470k RADIO_3V3→FB; 150k FB→GND; CIN/COUT candidate values per capture sheets; VAUX 100nF; VSEL/FB2/PS-SYNC treatment datasheet verification pending; EN reset-safe default OFF. Output RADIO_3V3.

## I2C controls
I2C_SDA=GPIO8; I2C_SCL=GPIO9. Shared onboard bus; no new pullups initially.
- ADS1115 U_AXIS @0x48: AIN0 JOY_LX; AIN1 JOY_LY; AIN2 JOY_RX; AIN3 JOY_RY.
- MCP23017 U_BUTTON @0x20: GPA0–7 BTN_L0–7; GPB0–7 BTN_R0–7.
- ADS1115 U_MON @0x49: AIN0 BAT_SENSE; AIN1 NTC_SENSE; AIN2 RADIO_SENSE optional; AIN3 SPARE.
- MCP23017 U_RADIO_IO @0x21: GPA0 RADIO_CE; GPA1 RADIO_CSN; GPA2 RADIO_IRQ_N; others spare. Timing validation is a blocker for final radio firmware.

## Battery sense
R_TOP 220k BAT_PROT→BAT_SENSE; R_BOTTOM 68k BAT_SENSE→GND; 100nF BAT_SENSE→GND. AIN0 of ADS1115@0x49. GPIO6 Sensor AD not used pending board circuit tracing.

## NTC
Controller 3V3→10k pullup→NTC_SENSE; pack NTC 10k B3950 candidate NTC_SENSE→GND; 100nF→GND; ADS1115@0x49 AIN1.

## Optional radio rail monitor
RADIO_3V3→100k→RADIO_SENSE; 100k RADIO_SENSE→GND; 100nF→GND; ADS1115@0x49 AIN2.

## SPI
SPI_MOSI GPIO11; SPI_SCK GPIO12; SPI_MISO GPIO13; shared TF/radio bus. Radio CSN via MCP23017@0x21 GPA1; CE GPA0; IRQ_N GPA2. TF-card CS remains onboard CH422G EXIO4. Software serialization and radio priority required.

## Connector drafts
J_JOY_L/R: 1 3V3; 2 GND; 3 X; 4 Y; 5 NC.
J_RADIO: 1 GND; 2 RADIO_3V3; 3 SCK; 4 MOSI; 5 MISO; 6 CSN; 7 CE; 8 IRQ_N.
J_NTC: 1 NTC_SENSE; 2 GND.
J_WAVE_BAT: 1 BAT_PROT; 2 GND; PH2.0 mating polarity must be physically checked.


## LoRa backup link power addition (R1)
- LoRa candidate module is UART EBYTE E22-900T22S-V2 (SX1262 family), which may require a 5V supply for full configured output power. Add branch LORA_5V from the Waveshare board's 5V boost rail only after simultaneous-load test confirms margin.
- At the module, candidate 100µF bulk + 10µF + 100nF bypass; validate capacitor type/ESR, surge, transient and layout.
- Module TX current estimate ~110mA. Do not assume board CS8501 can handle display/backlight + nRF + LoRa simultaneously. If fails, separate boost design from BAT_PROT is required; do not parallel boost outputs.
- LoRa's RF power must be configured to applicable EU/Bulgaria band limit including antenna gain and cable loss; airtime/duty-cycle constraints are release blockers.


## E22 supply correction R4
The E22-900T22S-V2 accepts 2.7–5.5V, but the datasheet says ≥5V guarantees output power. For the prototype, feed E22 VCC from the Waveshare CS8501 5V rail, not BAT_PROT or RADIO_3V3. Keep UART/M0/M1/AUX logic at 3.3V. E22 is configured to 10dBm; peak current budget must still allow the module's worst-case TX demand (use 150mA until exact 10dBm current is measured). Test CS8501 with display/backlight + E22 TX. If 5V droops/overheats, add a separate BAT_PROT→5V buck/boost branch; never parallel converter outputs.
