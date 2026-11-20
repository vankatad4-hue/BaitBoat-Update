# Boat controller pin/interface allocation R1

**Controller candidate:** Heltec Wireless Tracker V1.2. This is a pin-plan only until the exact purchased board/revision and its schematic/pin map are confirmed. It integrates ESP32-S3, SX1262 LoRa, and UC6580 GNSS; therefore backup LoRa may use the native radio.

## External carrier requirements
### nRF24 primary
- SPI MOSI/SCK/MISO from MCU pins chosen after board-pin audit.
- CSN and CE must be direct MCU GPIOs; avoid I2C expander for timing.
- IRQ is a direct GPIO/interrupt if available; otherwise polling with measured latency.
- 3V3 rail sized for PA/LNA burst current, local bulk + 100nF.
- 2.4GHz 50Ω antenna connector.

### Integrated LoRa
- SX1262 uses board-integrated radio and dedicated RF path; use Heltec library or exact compatible driver for that hardware revision.
- LoRa antenna via board's LoRa IPEX/U.FL RF connector, 868MHz regional antenna.
- Verify frequency support and output power config. GNSS RF connector is not the LoRa connector.

### GNSS and safety
- GNSS UART/antenna and power GPIO use native board resources; don't remap before pin-map audit.
- GNSS/heading health and battery voltage are required before return-home failsafe.
- Boat-local failsafe controls motor outputs; remote link loss must not be required to invoke it.

## Pin freeze checklist
1. Confirm exact Heltec board revision and frequency.
2. Read its schematic/pin map.
3. Check integrated SX1262 pins are reserved.
4. Select nRF SPI and direct CE/CSN/IRQ pins that do not collide with GNSS, display, USB, or strapping.
5. Confirm carrier connector pin numbers against schematic/continuity.
6. Document actual pin numbers, then freeze.

