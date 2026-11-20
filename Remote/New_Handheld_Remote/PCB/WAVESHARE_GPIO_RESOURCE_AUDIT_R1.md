# Waveshare ESP32-S3-Touch-LCD-4.3 Standard — GPIO Resource Audit R2

**Status:** GPIO/peripheral mapping checked against official online documentation; PORT1 pin-number crosswalk is provisional due source-document discrepancies. Confirm exact board revision and connector orientation. No hardware validation.

## Reserved MCU GPIOs
- LCD RGB: GPIO0,1,2,3,5,7,10,14,17,18,21,38,39,40,41,42,45,46,47,48.
- Touch IRQ GPIO4; I2C GPIO8/9; CH422G EXIO1 touch reset.
- GPIO6 Sensor AD circuit: reserved pending review.
- GPIO8/9 shared I2C for touch, CH422G, RTC and external connector.
- TF card SPI GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO; SD_CS via CH422G EXIO4. Share SPI with radio candidate; separate CS and serialized transactions.
- GPIO15/16 RS485/CAN; GPIO19/20 USB/CAN mux; GPIO43/44 UART0; avoid.
- GPIO35–37 left unused pending memory-routing confirmation for exact WROOM variant.
- PORT1 map in WAVESHARE_HEADER_MAPPING_AND_PIN_ASSIGNMENTS_R2.md.

## I2C expansion
- ADS1115 @0x48: four joystick axes AIN0 L-X, AIN1 L-Y, AIN2 R-X, AIN3 R-Y; scan timing to test.
- MCP23017 @0x20: 16 buttons; GPA left, GPB right; active-low, pullups, poll/debounce.
- MCP23017 @0x21 radio control candidate: GPA0 CE, GPA1 CSN, GPA2 IRQ_N. Expander timing not validated; replace if inadequate.
- Confirm addresses, bus pullups and capacitance; don't add pullups blindly.

## Shared nRF24 SPI
GPIO11 MOSI, GPIO12 SCK, GPIO13 MISO. Separate TF/radio CS, serialized transactions; radio priority and short SD bursts. Start 1–4MHz and bench characterize. Control pins via MCP23017@0x21 candidate.

## Battery sensing
GPIO6/Sensor AD is not free; battery telemetry is unassigned until circuit review or ADC allocation change.

## Remaining risks
Exact board revision; GPIO35–37 memory use; I2C scan and timing; SPI/SD coexistence; expander radio control timing; radio-rail transients/thermal; battery sense; native KiCad/ERC and hardware validation.
