#pragma once
// R30H-10: no verified direct nRF CE/CSN pin on Waveshare Standard SKU25948.
// Historical CE=GPIO6 collides with SD dummy CS/Sensor AD; historical CSN=GPIO15
// collides with the onboard RS485 transmitter. Never wire these as nRF pins.
// Existing RF24 firmware needs DIRECT GPIOs; I2C expander control is unimplemented.
// This test-time map is FAIL-CLOSED until a separately reviewed interface exists.
#define REMOTE_NRF_CE_PIN   (-1)
#define REMOTE_NRF_CSN_PIN  (-1)
#define REMOTE_R30H10_REJECTED_CE_GPIO 6
#define REMOTE_R30H10_REJECTED_CSN_GPIO 15
#define REMOTE_NRF_CHANNEL  76
#define REMOTE_NRF_MOSI_PIN 11
#define REMOTE_NRF_SCK_PIN  12
#define REMOTE_NRF_MISO_PIN 13
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
// Current display driver holds EXIO4 asserted during SD mount, preventing
// simultaneous nRF SPI operation until per-transaction CS ownership is solved.
#define REMOTE_SD_DUMMY_CSN_PIN 6
#if REMOTE_NRF_PIN_AUDIT_SIGNED
#error "R30H-10: no approved direct nRF CE/CSN, SPI/SD arbitration or RF evidence"
#endif
