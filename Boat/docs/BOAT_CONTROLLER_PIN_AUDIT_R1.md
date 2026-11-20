# Boat Controller Pin Audit R1

Updated 2026-11-20. This is a resource-conflict audit, **not a pin freeze**.

## Candidate board
Heltec Wireless Tracker ESP32-S3 + UC6580 GNSS. Hardware revision is unknown; this project must identify whether the board is V1.0, V1.1, or another revision and confirm the exact schematic/pin map before wiring.

## Board-integrated resources to reserve
The Wireless Tracker documentation describes an onboard SX1262, UC6580 GNSS, TFT, USB, power-control, and antenna resources. The following GPIO functions are shown in the V1.1/V2 documentation family; exact revisions differ and must be cross-checked against the physical board:
- GPIO 9/10/11: integrated SX1262 SPI clock/MOSI/MISO (reserve as a shared SPI bus only after confirming wiring and chip-select behavior).
- GPIO 8: SX1262 NSS/CS (reserve).
- GPIO 12: SX1262 reset (reserve).
- GPIO 13: SX1262 BUSY (reserve).
- GPIO 14: SX1262 DIO1 (reserve).
- GPIO 33/34: GNSS UART (reserve).
- GPIO 3: GNSS power enable on V1.1; do not repurpose.
- GPIO 38–42: onboard TFT resources (reserve).
- GPIO 21: TFT backlight (reserve).
- GPIO 0: user button / boot-strapping concern (reserve).
- GPIO 45/46: boot/strapping-related caution; do not allocate without schematic review.
- GPIO 43/44: USB serial/UART0 console; reserve for debug unless console disabled and verified.
- GPIO 17/18: UART1 candidates, but check board routing and any use.
- GPIO 19/20: UART flow-control signals in some board mappings; avoid treating as free without checking.

## nRF24 primary radio
- Requires SPI MOSI/SCK/MISO, CSN, CE, optional IRQ.
- Shared SPI with the integrated SX1262 is theoretically possible only if both chip-selects are electrically independent, bus pin mapping confirmed, each radio driver uses the same SPI peripheral, and bus arbitration is tested. Since the Tracker's onboard SX1262 is intentionally unused, do not assume its pins can be reassigned.
- Candidate physical header pins remain **unassigned**. No GPIO macros should be enabled yet.

## E22 backup radio
- UART TX/RX 9600 8N1, M0/M1 control, AUX input, optional reset, 3.3V logic.
- GPIO17/18 are preliminary UART candidates only; TX/RX direction and revision pin map must be checked. Mode/AUX pins remain TBD; do not use GPIO4–7 or other RF-front-end-related pins without schematic review.

## Stop / audit procedure
1. Photograph board front/back and read silkscreen revision/model.
2. Download matching schematic and pin map from the manufacturer.
3. Compare every candidate pin against GNSS, SX1262, TFT, USB, boot/strapping, LED, battery/ADC, RF controls and any board firmware.
4. Continuity-test header-to-MCU signals with board unpowered where possible.
5. Verify idle voltage and logic levels; ensure external radio CE/CSN/M0/M1/AUX have safe reset states.
6. Update the pin CSV and `boat_radio_hw_config.h` together only after audit sign-off.
7. Compile, then current-limited radio-only bench test; motors and solenoids remain disconnected.

## Official references
- Heltec Wireless Tracker documentation/resource page: https://docs.heltec.org/en/node/esp32/wireless_tracker/index.html
- Manufacturer V1.1 schematic and pin map are linked from that page; use the document matching the physical revision.
- V1.1 revision notes identify GPIO3 as GNSS power enable: https://docs.heltec.org/en/node/esp32/hardware_update_log.html

## Result
The current boat nRF/E22 bench sketches remain deliberately disabled by `-1` pin macros. This is the correct safe state until board revision and pin crosswalk are confirmed.
