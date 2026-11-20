# Modular PCB Architecture — Preliminary R2

**Status:** Architecture document; native schematic, ERC, PCB, BOM and validation pending.

## Blocks
1. Waveshare ESP32-S3-Touch-LCD-4.3 Standard controller/display module.
2. Pack protection: BMS-protected 1S4P input, fuse candidate, reverse-protection MOSFET, voltage/NTC sense.
3. Controller/display power: use the Waveshare board's onboard CS8501 single-cell battery boost via its battery connector; external TPS61023 is fallback only after tests. Do not parallel boost outputs.
4. Radio power: TPS63070 buck-boost, dedicated RADIO_3V3 rail for nRF PA/LNA.
5. Analog input: ADS1115 I2C ADC for four joystick axes.
6. Buttons: MCP23017 I2C 16-bit GPIO expander for 16 contacts.
7. nRF24 radio: SPI and control pins; share SPI with TF card only with arbitration and radio priority.
8. Connector harnesses: battery, radio, joysticks, buttons, I2C and testpoints.

## Power
Protected pack→BAT_PROT. BAT_PROT supplies Waveshare battery header and TPS63070. Radio rail calculated 3.3067V nominal from 470k/150k divider; 1.5µH candidate and capacitor bank. Effective capacitance/current/thermal tests pending.
Controller/display is powered by board's onboard battery boost; manufacturer's board load figure 5V/450mA. Validate low battery, backlight, Wi-Fi and radio TX; external boost fallback only if necessary.

## Interface candidates
- I2C: GPIO8 SDA/GPIO9 SCL (existing board bus); ADS1115 address 0x48, MCP23017 address 0x20.
- Joysticks: 4 single-ended axis signals; ADS1115 scan.
- Buttons: 16 active-low inputs to MCP23017; 8 per bank.
- Battery ADC: GPIO6/Sensor AD candidate with 220k/68k divider and 100nF.
- nRF SPI: GPIO11/12/13 shared TF card bus; candidate CSN GPIO15, CE GPIO43; verify mux/exposure; IRQ polling.
- All candidate GPIO assignments require schematic/PORT1 and physical board verification.

## RF and layout
Keep switching loops away from RF module/antenna, short SPI, keyed connectors, adequate power/ground, testpoints. Shared 2.4GHz ESP32 Wi-Fi/Bluetooth and nRF24 link must be tested for coexistence.

## Outstanding
Native KiCad capture and ERC; exact symbols/footprints/pins/connectors; protection part selection; GPIO verification; PCB placement/routing/DFM; test/bench bring-up.
