# Remote Schematic Netlist Blocks R1

**Net-level design reference for capture.** Not a generated netlist or native KiCad schematic. Sheet design details in SCHEMATIC_CAPTURE_SHEETS_R1.md.

## Global power nets
- PACK_PLUS, PACK_MINUS
- PACK_FUSED (after F_MAIN)
- BAT_PROT (after reverse MOS)
- GND (common pack negative)
- WAVE_BAT_POS/WAVE_BAT_NEG
- RADIO_3V3
- 3V3_CONTROLLER (from Waveshare board header only after header pin verified)
- RADIO_EN
- RADIO_PGOOD optional

## Battery protection nets
J_PACK.1 PACK_PLUS; J_PACK.2 PACK_MINUS.
F_MAIN: 1 PACK_PLUS; 2 PACK_FUSED.
Q_REV: D PACK_FUSED; S BAT_PROT; G GATE_REV (verify symbol/package pin numbers).
R_GATE: GATE_REV–GND 100k.
C_BULK: BAT_PROT–GND 100µF candidate.
C_BYPASS1: BAT_PROT–GND 10µF.
C_BYPASS2: BAT_PROT–GND 100nF.
J_WAVE_BAT: 1 BAT_PROT; 2 GND.
U_RADIO VIN input from BAT_PROT; converter output RADIO_3V3.

## Battery/thermal monitor
U_ADC2 ADS1115 @0x49: ADDR=3V3; I2C_SDA/SCL; VDD=3V3; GND=GND; AIN0=BAT_SENSE; AIN1=NTC_SENSE; AIN2=RADIO_SENSE optional via divider; AIN3=SPARE_ADC.
BAT divider: R_BAT_TOP 220k BAT_PROT–BAT_SENSE; R_BAT_BOTTOM 68k BAT_SENSE–GND; C_BAT_SENSE 100nF BAT_SENSE–GND.
NTC divider: R_NTC_PULLUP 10k 3V3–NTC_SENSE; J_NTC.1 NTC_SENSE; J_NTC.2 GND; NTC 10k B3950 NTC_SENSE–GND; C_NTC 100nF NTC_SENSE–GND.
ADC2 channels all to ground-referenced single-ended signals; configure PGA appropriate; do not allow analog inputs above VDD or below GND.

## I2C
I2C_SDA=ESP GPIO8; I2C_SCL=ESP GPIO9; external I2C connector shares. No additional pullups initially.
U_ADC1 ADS1115 address 0x48: AIN0=JOY_LX; AIN1=JOY_LY; AIN2=JOY_RX; AIN3=JOY_RY.
U_ADC2 ADS1115 address 0x49: AIN0=BAT_SENSE; AIN1=NTC_SENSE; AIN2=RADIO_SENSE (optional divider); AIN3=SPARE_ADC.
U_BTN MCP23017 address 0x20: GPA0–7=BTN_L0–7; GPB0–7=BTN_R0–7; RESET pull-up.
U_RADIO_IO MCP23017 address 0x21: GPA0=RADIO_CE; GPA1=RADIO_CSN; GPA2=RADIO_IRQ_N; RESET pull-up. Confirm address straps. I2C reset interaction to be defined.

## SPI
SPI_MOSI=GPIO11, SPI_SCK=GPIO12, SPI_MISO=GPIO13. Radio CSN from U_RADIO_IO; TF-card CS controlled onboard CH422G EXIO4. Shared transactions serialized.

## Connector drafts
J_JOY_L/R: 1 3V3; 2 GND; 3 axis X; 4 axis Y; 5 NC.
J_RADIO: 1 GND; 2 RADIO_3V3; 3 SPI_SCK; 4 SPI_MOSI; 5 SPI_MISO; 6 RADIO_CSN; 7 RADIO_CE; 8 RADIO_IRQ_N.
J_NTC: 1 NTC_SENSE; 2 GND.
J_WAVE_BAT: 1 BAT_PROT; 2 GND; connector orientation/polarity verified physically before connection.
