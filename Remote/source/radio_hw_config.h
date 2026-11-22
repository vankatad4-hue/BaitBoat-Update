#pragma once
// Provisional Waveshare GPIO assignment for test harness only.
// GPIO6 is Waveshare Sensor AD and is ALREADY used as the SD library's dummy CS.
// Proposed NRF CE GPIO6 therefore CONFLICTS even without an external ADC sensor.
// Keep nRF physically unwired until a verified, revised pin/power map exists.
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
// R30H-3 optional diagnostic-only Remote STOP timing trace; default disabled.
#define REMOTE_R30H3_STOP_TRACE_ENABLED 0

// The SD library drives a dummy GPIO CS, while the real TF CS is CH422G EXIO4.
// This is a current real pin collision with the provisional nRF CE signal.
// No approved replacement GPIO exists on the current Waveshare Standard PCB.
#define REMOTE_SD_DUMMY_CSN_PIN 6
#if REMOTE_NRF_PIN_AUDIT_SIGNED && (REMOTE_NRF_CE_PIN == REMOTE_SD_DUMMY_CSN_PIN)
#error "REMOTE nRF CE conflicts with SD dummy CS on GPIO6; physical nRF authorization forbidden"
#endif
