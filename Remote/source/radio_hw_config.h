#pragma once
// Provisional Waveshare GPIO assignment for test harness only.
// GPIO6 is the Sensor AD signal (repurposed, no sensor connected).
// GPIO15 is RS485 TXD (repurposed as CSN; onboard RS485 driver must be disabled).
// Confirm exact board revision, electrical loading and connector crosswalk before wiring.
#define REMOTE_NRF_CE_PIN   6
#define REMOTE_NRF_CSN_PIN  15
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
