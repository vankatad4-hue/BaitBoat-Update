# Remote Electrical Netlist Blocks R4

## Pack
J_PACK.P1=PACK+;J_PACK.P2=PACK−.
PACK+→F_MAIN4A candidate→PACK_FUSED→Q_REV.D;Q_REV.S→BAT_PROT.
Q_REV.G→100k→GND; verify body diode/pins.
PACK−→GND.
BAT_PROT→C_BAT 100µF+10µF+100nF to GND.

## Controller
BAT_PROT/GND→Waveshare PH2.0 battery input (polarity verify).
CS8501 onboard 5V rail powers controller/display only.

## nRF power
BAT_PROT→TPS63070 VIN; TPS63070 VOUT=RADIO_3V3.
RADIO_3V3→nRF VCC; local bypass to GND.
TPS63070 candidate L1.5µH, FB top470k/bottom150k; validate exact reference design.

## E22 power
BAT_PROT→TPS61023 dedicated boost→LORA_5V→E22 VCC.
No connection between LORA_5V and CS8501 5V output.
LORA_5V local100µF+10µF+100nF to GND.
E22 UART/mode/status logic=3V3.

## nRF
J_NRF.1=GND;2=RADIO_3V3;3=MOSI GPIO11;4=SCK GPIO12;5=MISO GPIO13;6=CSN GPIO15;7=CE GPIO6;8=IRQ_N MCP23017@0x21 GPA1.
GPIO6 Sensor AD unused;GPIO15 RS485 path, cable disconnected.
 
## E22 bridge
SC16IS750@0x48 I2C SDA GPIO8/SCL GPIO9.
Bridge UART TX→E22 RX; bridge RX←E22 TX.
GPIO0=M0;GPIO1=M1;GPIO2=AUX;GPIO3=NRST.
E22 RF→RIGHT 868MHz coax/antenna.
 
## I2C
MCP23017 buttons@0x20,radioIO@0x21;ADS1115 axes@0x4A,monitor@0x4B;SC16@0x48. Verify onboard addresses/pullups.
