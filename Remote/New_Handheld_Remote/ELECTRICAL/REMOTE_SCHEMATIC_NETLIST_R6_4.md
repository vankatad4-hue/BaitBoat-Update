# Remote R6.4 — Consolidated Schematic Netlist Review Draft

**Derived from:** Remote ver009 archive, R4 capture/netlist sheets, R5 pin/button assignments and current R6.3 enclosure concept.  
**Status:** logical netlist reconciliation only. Not a KiCad schematic, not ERC/DRC checked, not fabrication-ready.

## 1. Controller / shared buses

| Net | Source | Destination | Electrical | Status |
|---|---|---|---|---|
| I2C_SDA | Waveshare ESP32 GPIO8 | Touch/CH422G/RTC onboard + external peripherals | 3.3V logic | Shared; scan and loading verify |
| I2C_SCL | Waveshare ESP32 GPIO9 | Same shared I2C bus | 3.3V logic | Shared; scan and loading verify |
| SPI_MOSI | GPIO11 | nRF module + TF card bus | 3.3V | Shared SPI, arbitration needed |
| SPI_SCK | GPIO12 | nRF module + TF card bus | 3.3V | Shared SPI |
| SPI_MISO | GPIO13 | nRF module + TF card bus | 3.3V | Shared SPI |
| NRF_CE | GPIO6 | nRF CE | 3.3V | Candidate; Sensor AD path must be isolated/disabled |
| NRF_CSN | GPIO15 | nRF CSN | 3.3V | Candidate; RS485 TX path must be isolated/disabled |
| NRF_IRQ_N | MCP23017 U_RADIO_IO GPA1 | nRF IRQ_N | 3.3V input | Poll first; interrupt timing later |
| SC16_IRQ_N | MCP23017 U_RADIO_IO GPA2 | SC16IS750 IRQ | 3.3V input | Optional/polling candidate |

**GPIO6/GPIO15:** Do not connect until board schematic/physical revision verifies isolation from onboard analog/RS485 circuitry. PORT1 pin-number mapping remains provisional.

## 2. I2C device/address plan

| Device | Address | Connections / functions |
|---|---|---|
| GT911 touchscreen | Board configured (typically 0x5D or 0x14) | Existing board; do not conflict |
| CH422G | Board-specific | Existing board GPIO expander |
| PCF85063 RTC | 0x51 | Existing board |
| MCP23017 U_BUTTONS | 0x20 | 15 scanned physical keys (L1–L8, R1–R7; GPB7 spare) |
| MCP23017 U_RADIO_IO | 0x21 | nRF IRQ, SC16 IRQ, radio enable, OK |
| SC16IS750 U_UART | 0x48 | E22 UART bridge |
| ADS1115 U_AXIS | 0x4A | Four joystick axes, ADDR tied SDA |
| ADS1115 U_MON | 0x4B | Battery/NTC/optional radio rail, ADDR tied SCL |

Addresses are planned, not proven. Perform an I2C scan on the exact Waveshare board and verify driver compatibility before stuffing external modules.

## 3. Button bank

U_BUTTONS MCP23017 @0x20:
- GPA0–GPA7 → left-bank L1–L8, active-low to GND, pullups enabled.
- GPB0–GPB6 → right-bank R1–R7, active-low to GND, pullups enabled.
- GPB7 reserved/spare; pull-up enabled. OK is the right-bank bottom-right key, separately scanned on U_RADIO_IO GPA4.
- Physical arrangement: two columns × four rows per bank. The right-bank bottom-right position is OK/confirm, not an R8 input. The bottom-right right-bank position is OK/confirm, not a normal MCP0x20 button.
- Debounce target 25 ms.
- Button mapping from R5 CSV: L1 STOP; L2 Manual/Autopilot; L3 Return Home; L4 save waypoint; L5 waypoint selection; L6 map full-screen; L7 sonar full-screen; L8 settings. Right R1–R4 guarded independent releases; R5–R7 are UI/status/cancel functions; the physical OK key is separately wired to U_RADIO_IO GPA4
- OK occupies the right-bank bottom-right physical position but uses the dedicated input on U_RADIO_IO GPA4; U_BUTTONS scans only 15 keys.

U_RADIO_IO MCP23017 @0x21:
- GPA0 spare.
- GPA1 nRF IRQ_N input.
- GPA2 SC16 IRQ_N input.
- GPA3 RADIO_EN output (candidate, reset-safe low).
- GPA4 physical OK/confirm key in the right-bank bottom-right position, active-low.
- GPA5–GPA7 and GPB0–GPB7 spare.

Release safety: separate bait-left, bait-right, hook-left, hook-right only. Require hold→release→separate OK confirmation within guard window; authenticated primary link and valid boat interlock telemetry. No release on LoRa backup. Current firmware safety readiness remains false.

## 4. Joysticks / analog

U_AXIS ADS1115 @0x4A:
- AIN0 = LEFT_X
- AIN1 = LEFT_Y
- AIN2 = RIGHT_X
- AIN3 = RIGHT_Y
- 10k potentiometer candidates, 3.3V and GND endpoints; optional 1k series/1µF RC filtering is documented as candidate only.
- Sampling target 50Hz in R4 capture; calibrate endpoints and center, apply deadband and fail-closed behavior.

U_MON ADS1115 @0x4B:
- AIN0 = BAT_SENSE from BAT_PROT divider 220k/68k and 100nF.
- AIN1 = NTC10k/B3950 divider and 100nF.
- AIN2 = optional RADIO_3V3 monitor divider.
- AIN3 = spare.
- Verify input range, ADC reference/PGA, divider accuracy, open/short detection, filtering and calibration.

## 5. nRF24 primary

| Radio pin | Net |
|---|---|
| VCC | RADIO_3V3 (TPS63070) |
| GND | GND |
| MOSI | GPIO11 shared SPI |
| SCK | GPIO12 shared SPI |
| MISO | GPIO13 shared SPI |
| CSN | GPIO15 direct, isolated RS485 path |
| CE | GPIO6 direct, isolated Sensor-AD path |
| IRQ_N | U_RADIO_IO 0x21 GPA1 |

Initial RF settings: channel76, 250kbps, fixed32-byte payload, CRC16, PA_LOW. Antenna LEFT. Separate regulated supply, local 100nF + 10–47µF. SPI start at 1–4MHz, radio priority; serialized TF access and short SD transactions. Validate burst droop, RF link and display/SD coexistence.

## 6. E22 backup radio / SC16IS750

U_SC16IS750 @0x48:
- I2C SDA/SCL = GPIO8/GPIO9.
- VDD=3.3V, local 100nF + 1µF.
- 1.8432MHz crystal candidate; crystal load capacitors calculated from crystal spec.
- UART 9600 8N1.
- Bridge TX→E22 RXD; bridge RX←E22 TXD.
- GPIO0=M0; GPIO1=M1; GPIO2=AUX; GPIO3=NRST.
- IRQ_N→U_RADIO_IO GPA2 (or poll during first bring-up).
- E22 VCC=LORA_5V, dedicated TPS61023 converter; UART/mode logic 3.3V.
- E22 antenna RIGHT 868MHz.

Initial configuration candidate: channel18/868.125MHz, 62.5kbps air rate, 32-byte subpacket, 10dBm, transparent; read back settings and legal duty/EIRP validation required. Backup command policy restricted to STOP/RETURN, heartbeat/low-rate telemetry and limited manual steering; mission/release commands prohibited.

## 7. Power and connector blocks

- 1S4P pack: connector, near-pack fuse, protection, reverse-polarity MOS candidate and BAT_PROT.
- Controller/display: Waveshare single-cell input/onboard CS8501 boost. Verify charge/power-path and thermal.
- nRF: TPS63070 BAT_PROT→RADIO_3V3, candidate 1.5µH and feedback values per reviewed draft; verify reference design/footprints.
- E22: TPS61023 BAT_PROT→LORA_5V, 5V/2A-class candidate. Component values and thermal not frozen.
- No converter outputs tied together.
- Connector pin numbers, keying, footprint and current rating are not assigned here; use connector interface plan and verify with exact modules.

## 8. Review checklist before schematic capture

- [ ] Compare exact Waveshare SKU/revision and PORT1 orientation.
- [ ] Verify GPIO6 sensor circuit and GPIO15 RS485 isolation.
- [ ] Confirm nRF SPI/TF chip-select behavior and shared-bus arbitration.
- [ ] Confirm I2C device addresses and pullups (scan).
- [ ] Validate MCP23017 button/OK mapping and reset states.
- [ ] Validate ADS1115 input range/filter and sampling.
- [ ] Validate SC16 crystal, I2C address, IRQ and FIFO servicing.
- [ ] Validate power converter datasheet/reference, footprints and load/thermal.
- [ ] Confirm pack fuse/BMS/cell/wire/connector ratings.
- [ ] Create native KiCad schematic, annotate, ERC and review connector pinout.


## R6.6 physical input crosswalk update
The approved front arrangement is 2×4 keys on each side. The 16 physical positions are 8 left-bank keys + 7 right-bank scanned keys + OK at right-bank bottom-right. Thus MCP23017 0x20 uses 15 keys (GPB7 spare) and MCP23017 0x21 GPA4 scans OK independently. The scanner source was updated to poll only 15 IDs plus the dedicated OK input. CAD STL/preview exports must be regenerated from the corrected source.


## R6.6 physical crosswalk
There are 16 physical key positions: 8 left-bank keys + 7 right-bank scanner keys + OK. U_BUTTONS 0x20 scans 15 inputs; GPB7 is spare. U_RADIO_IO 0x21 GPA4 scans OK. The front layout is 2 columns × 4 rows per bank, one Power key above the display, no duplicate bottom Power.
