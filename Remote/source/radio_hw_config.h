#pragma once
// R30H-10: no verified direct nRF CE/CSN pin on Waveshare Standard SKU25948.
// Historical CE=GPIO6 collides with SD dummy CS/Sensor AD; historical CSN=GPIO15
// collides with the onboard RS485 transmitter. Never wire these as nRF pins.
// R30H-11 software selection: manufactured SC18IS606-EVB I2C->SPI bridge,
// no MCU GPIO required: bridge SS0=nRF CSN and existing MCP23017 GPA0=nRF CE.
// SS1 remains the EVB EEPROM and SS2 stays UNCONNECTED. NXP SS2 resets as
// active-low slave select (idle high), so a CE pull-down alone is not
// sufficient protection if SS2 were wired directly to active-high CE.
// This *candidate* requires physical module, power, timing and RF acceptance.
// Direct-pin nRF RF24 library stays archival; the active adapter uses a
// separate SPI controller so the TF card on ESP SPI11/12/13 is untouched.
// This test-time map is FAIL-CLOSED until a separately reviewed interface exists.
#define REMOTE_NRF_CE_PIN   (-1)
#define REMOTE_NRF_CSN_PIN  (-1)
#define REMOTE_R30H10_REJECTED_CE_GPIO 6
#define REMOTE_R30H10_REJECTED_CSN_GPIO 15
// EVB JP3 all jumpers ON -> wire address 0x50, Arduino 7-bit I2C 0x28.
// Actual jumper states and 3.3V levels must be measured.
#define REMOTE_NRF_BRIDGE_I2C_ADDRESS 0x28
#define REMOTE_NRF_CE_EXPANDER_ADDRESS 0x21
#define REMOTE_NRF_CE_EXPANDER_GPA 0
#define REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED 0
#define REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED 0
#define REMOTE_NRF_CHANNEL  76
// Direct nRF SPI on ESP32 is retired. The only ESP32 SPI11/12/13 is TF.
#define REMOTE_TF_SPI_MOSI_PIN 11
#define REMOTE_TF_SPI_SCK_PIN  12
#define REMOTE_TF_SPI_MISO_PIN 13
#define REMOTE_NRF_MOSI_PIN (-1)
#define REMOTE_NRF_SCK_PIN  (-1)
#define REMOTE_NRF_MISO_PIN (-1)
// R30H-1: actual Waveshare board, RF carrier, logic and airtime have NOT
// been purchased/inspected/signed. Keep RADIO INITIALIZATION locked by default.
// Do not set to 1 based on host tests, a schematic or a successful target build.
#define REMOTE_NRF_PIN_AUDIT_SIGNED 0
#define REMOTE_E22_BRIDGE_AUDIT_SIGNED 0
#define REMOTE_E22_RF_INTEROPERABILITY_VERIFIED 0
#define REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED 0
// R30H-3 optional diagnostic-only Remote STOP timing trace; default disabled.
#define REMOTE_R30H3_STOP_TRACE_ENABLED 0

// SD GPIO6 is a dummy select; real TF CS comes from CH422G EXIO4.
// Current display driver holds EXIO4 asserted during SD mount. Its SD path
// remains unchanged; the selected SC18IS606 has INDEPENDENT SPI lines.
// Never wire nRF to MCU SPI11/12/13 (they remain SD-owned).
#define REMOTE_SD_DUMMY_CSN_PIN 6
#if REMOTE_NRF_PIN_AUDIT_SIGNED
#error "R30H-11: no approved nRF hardware or I2C-SPI bridge; radio commissioning forbidden"
#endif
