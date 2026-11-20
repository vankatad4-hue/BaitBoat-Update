# Remote Hardware Test Build R4

**Purpose:** Safe radio/electronics bench build specification. Not fabrication-ready or field-operational. Motors/solenoids are not part of the remote test.

## Selected hardware
- Waveshare ESP32-S3-Touch-LCD-4.3 Standard; exact revision/pin crosswalk verify.
- 1S4P matched 18650 pack; protected/fused; BMS and cell assembly validated.
- Primary nRF24L01+ PA/LNA 2.4GHz module.
- Backup EBYTE E22-900T22S-V2 UART LoRa module; matching module on boat.
- SC16IS750 I2C-UART bridge @0x48 for E22 UART.
- ADS1115 axes @0x4A and monitor @0x4B; MCP23017 buttons @0x20 and radio I/O @0x21.
- Dedicated power: controller/display CS8501 onboard 5V, nRF TPS63070 3.3V, E22 TPS61023 BAT_PROT→5V boost. Separate converter outputs; never parallel.

## Remote GPIO / buses (provisional test allocation)
- I2C SDA GPIO8, SCL GPIO9. Verify bus devices, pullups and voltage.
- SPI MOSI GPIO11, SCK GPIO12, MISO GPIO13 shared with TF. Initialize explicitly; test SD/radio arbitration.
- nRF CE GPIO6 (Sensor AD terminal repurposed; no sensor connected); CSN GPIO15 (RS485 TX path repurposed; disconnect RS485 cable and ensure transceiver inactive); IRQ_N to MCP23017 0x21 GPA1 for polling.
- SC16IS750 @0x48: I2C, 1.8432MHz crystal candidate, UART 9600 8N1, GPIO0=M0, GPIO1=M1, GPIO2=AUX input, GPIO3=NRST; bridge IRQ to GPA2.
- MCP23017 buttons @0x20: GPA left-bank 8, GPB right-bank 8; active-low to GND, pullups.
- MCP23017 radio I/O @0x21: GPA0 spare, GPA1 nRF IRQ, GPA2 SC16 IRQ, GPA3 RADIO_EN; remaining pins reserved.
- ADS1115 @0x4A axes: AIN0 LX, AIN1 LY, AIN2 RX, AIN3 RY; ADDR SDA.
- ADS1115 @0x4B monitor: AIN0 battery divider, AIN1 NTC, AIN2 optional RADIO_3V3 monitor, AIN3 spare; ADDR SCL.
- All header pin numbers, connector orientation, GPIO loading and continuity remain mandatory verification before wiring.

## Power tree
1S4P pack → F_MAIN candidate4A → reverse-polarity P-MOS candidate → BAT_PROT.
- BAT_PROT → Waveshare PH2.0 battery input; verify polarity physically. CS8501 5V is controller/display only; test board load and USB/backfeed.
- BAT_PROT → TPS63070 buck-boost → RADIO_3V3 for nRF.
- BAT_PROT → TPS61023 dedicated 5V boost → LORA_5V → E22 VCC. Reserve 150mA TX peak until measured. Local input/output capacitors and module bypass per reference design; verify stability and thermal. Never parallel CS8501 and TPS61023 outputs.
- E22 UART and M0/M1/AUX are 3.3V logic, not 5V TTL.
- Battery divider and NTC are ADC monitored; divider input limits, calibration, open/short behavior must be tested.
- Estimate with assumed loads: 5V controller/display 450mA, 5V E22 150mA peak, 3.3V nRF 300mA burst, converters90%: approximately 1.48A pack at3.0V. This is an estimate; measure worst-case total.

## Primary nRF24 profile
- Channel76 (2476MHz), 250kbps, fixed32-byte payload, CRC16, auto-ack/retries, PA_LOW initial.
- Pair addresses: Remote→Boat “BOAT1”; Boat→Remote “RMTE1”.
- Remote source CE6/CSN15/SPI11/12/13 are provisional. Test RF and SD coexistence; do not connect RS485 bus.

## Backup E22 profile
Identical configuration at remote and boat:
- Address/network0.
- UART9600 8N1; air rate62.5kbps.
- Subpacket32 bytes.
- Output10dBm.
- Channel18=868.125MHz.
- Transparent mode, RSSI append off, LBT off initially.
- Configuration write/readback: `C0 00 09 00 00 00 67 C3 12 00 00 00`.
- M1=1/M0=0 configuration; M1=M0=0 normal; both high=sleep.
- 868.125MHz is in the 1% duty-cycle sub-band. E22 channel step1MHz cannot select 869.525MHz center of 10% band.
- Conservative estimated airtime25ms/frame; one frame/3s=0.83%. The limiter is RAM-only; no field operation until actual airtime measured and duty accounting persists across reset. LoRa is low-rate safety/telemetry, not continuous steering/sonar.

## Firmware implementation status
- nRF24 transport class, E22 transport, SC16IS750 bridge, frame CRC/HMAC codec, command decoder, link manager, UI link state and remote heartbeat scheduler are present in source.
- Remote E22 module config/readback integrated at boot. nRF CE/CSN settings provisional.
- Key store returns false; no authenticated link, commands or actuators enabled. Session negotiation/replay across reboot and application ACK/state confirmation incomplete.
- Boat dual-radio receive/command queue/dedupe/failsafe scaffold exists, but not integrated into a main boat control loop.
- No compilation, bench measurements or RF range test recorded.

## Bench test order
1. Inspect exact Waveshare revision, PORT1 pin crosswalk, GPIO6 Sensor AD circuit, GPIO15 RS485 path, connector polarity.
2. Current-limited supply: controller/display only; confirm 5V load/ripple/thermal and USB behavior.
3. Verify I2C scan, addresses, pullups and GPIO expanders.
4. nRF radio test with remote sketch; no SD access first, then shared SPI coexistence.
5. E22 config/readback and remote bridge UART loopback.
6. Pair E22 test sketches at low power, antennas attached, close range; verify CRC probe RX/TX and module AUX.
7. Test controller, nRF, E22, Wi-Fi and SD simultaneous current/rail droop/thermal.
8. Record current, rail ripple, packet error, latency and configuration. Do not connect boat motors/solenoids.

## Hardware freeze blockers
Native KiCad schematics, ERC/DRC, PCB layout, exact physical pin crosswalk, module footprints, pack/BMS/fuse coordination, converter design/thermal, persistent duty-cycle counter, secure key/session provisioning, application ACK, boat actuator/failsafe integration and RF regulatory verification remain incomplete.
