# Remote Schematic Capture Sheets R4

## Sheet 01 — pack input/protection
J_PACK keyed2pin; PACK+→F_MAIN4A candidate→PACK_FUSED→Q_REV P-MOS candidate Drain; Source→BAT_PROT; Gate100k→GND. PACK−→GND. Verify body diode/pin orientation. Bulk100µF+10µF+100nF. Fuse near pack; BMS, wire, connector and fuse coordination.

## Sheet 02 — controller power
BAT_PROT→Waveshare PH2.0 single-cell input; physically verify polarity. CS8501 onboard boost powers controller/display only. USB charging/backfeed and max-load/thermal test. No parallel external boost.

## Sheet 03 — nRF radio rail
BAT_PROT→TPS63070→RADIO_3V3. Candidate L1.5µH; FB divider470k/150k; CIN/COUT per datasheet effective capacitance/stability review; EN default OFF. RADIO_3V3 only to nRF and optional monitored node. Local bypass100nF+10–47µF.

## Sheet 04 — nRF primary
J_NRF: GND, RADIO_3V3, MOSI GPIO11, SCK GPIO12, MISO GPIO13, CSN GPIO15 direct, CE GPIO6 direct, IRQ_N→MCP23017 0x21 GPA1. GPIO6 Sensor AD unused; GPIO15 RS485 TX path repurposed, RS485 bus disconnected and transceiver inactive. SPI shared TF; lock/prioritize and bench-test. Channel76,250kbps,fixed32,PA_LOW initial.

## Sheet 05 — E22 backup and UART bridge
E22 VCC=LORA_5V from dedicated TPS61023 BAT_PROT→5V boost; CS8501 controller rail not shared. Reserve150mA TX peak until measured; converter layout/reference/thermal validate.
SC16IS750 @0x48 on I2C GPIO8/9; 1.8432MHz crystal candidate; 9600 8N1. Bridge GPIO0=M0, GPIO1=M1, GPIO2=AUX input, GPIO3=NRST. E22 UART logic3.3V only. Local bypass100µF+10µF+100nF.
E22 config: network/address0; REG0=0x67, REG1=0xC3, REG2=0x12, REG3–5=0. Channel18=868.125MHz, air62.5kbps, packet32, output10dBm, transparent. Config command/readback exact.

## Sheet 06 — I2C/expanders
Shared I2C GPIO8/9: touch/RTC/CH422G onboard plus MCP23017 0x20 buttons, MCP23017 0x21 radio IO, ADS1115 0x4A axes, ADS1115 0x4B monitor, SC16 0x48. Verify address scan and pullups; don't add pullups blindly.

## Sheet 07 — joystick ADC
ADS1115 0x4A ADDR=SDA; AIN0 LX/AIN1 LY/AIN2 RX/AIN3 RY. 10k axis pots3V3/GND; 1k series+1µF filters candidate. 50Hz scan/calibration/deadband.

## Sheet 08 — battery and temperature ADC
ADS1115 0x4B ADDR=SCL; AIN0 divider220k/68k from BAT_PROT +100nF; AIN1 10k pullup+10k B3950 NTC+100nF; AIN2 optional RADIO_3V3 divider100k/100k; AIN3 spare. Input limits/calibration/open-short detect verify.

## Sheet 09 — buttons/radio IO
MCP23017 0x20 buttons active-low, GPA0–7 left, GPB0–7 right; internal pullups and debounce.
MCP23017 0x21 GPA0 spare,GPA1 nRF IRQ,GPA2 SC16 IRQ,GPA3 RADIO_EN; reset pullup,100nF. nRF CE/CSN direct, not expander.

## Sheet 10 — connectors/test points
Keyed controller, display, pack, button banks, joysticks, nRF, E22/antenna and service. Test pads BAT_PROT,5V_CONTROLLER,LORA_5V,RADIO_3V3,GND,I2C,SPI,UART,CE/CSN,ADC. RF antenna LEFT2.4GHz,RIGHT868MHz. No antenna swap.

## Layout rules
4-layer PCB candidate100×75mm,1.6mm; 2oz external power copper candidate; L2 solid GND; keep RF, converter switching, battery and ADC physically separated; nRF/E22 modules at respective antenna feeds; decoupling near pins; test/thermal/clearance. Board dimensions are provisional; no native KiCad/ERC/DRC yet.
