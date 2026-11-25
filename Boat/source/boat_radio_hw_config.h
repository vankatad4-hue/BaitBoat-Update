#pragma once
#include "boat_pin_assignments.h"
// R20 candidate mapping for ESP32-S3-DevKitC-1-N32R16V.
// Pin audit flags intentionally remain 0: board revision and harness not physically signed.
#define BOAT_NRF_CE_PIN 10
#define BOAT_NRF_CSN_PIN 9
#define BOAT_NRF_CHANNEL 76
#define BOAT_NRF_SCK_PIN 12
#define BOAT_NRF_MISO_PIN 13
#define BOAT_NRF_MOSI_PIN 11
#define BOAT_NRF_IRQ_PIN 21
#define BOAT_NRF_PIN_AUDIT_SIGNED 0
#define BOAT_SX1262_BOARD_AUDIT_SIGNED 0
#define BOAT_SX1262_PEER_PHY_VERIFIED 0
#define BOAT_SX1262_EU868_APPROVED 0
#define BOAT_SX1262_NSS (-1)
#define BOAT_SX1262_SCK (-1)
#define BOAT_SX1262_MISO (-1)
#define BOAT_SX1262_MOSI (-1)
#define BOAT_SX1262_RST (-1)
#define BOAT_SX1262_BUSY (-1)
#define BOAT_SX1262_DIO1 (-1)
#define BOAT_E22_UART_SELECTED 1
#define BOAT_E22_UART_PIN_TX 17
#define BOAT_E22_UART_PIN_RX 18
#define BOAT_E22_UART_PIN_M0 15
#define BOAT_E22_UART_PIN_M1 16
#define BOAT_E22_UART_PIN_AUX 14
#define BOAT_E22_UART_PIN_NRST 6
#define BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED 0
#define BOAT_E22_UART_LOGIC_LEVELS_VERIFIED 0
#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0
#define BOAT_E22_RF_AIRTIME_AUDIT_SIGNED 0
// R30H-2: optional BOAT-LOCAL, USB-only read-only event trace. Disabled by default.
// May be enabled only in the non-actuating diagnostic target after physical logic-level checks.
#define BOAT_R30H_LOCAL_TRACE_ENABLED 0

// Preprocessor #if expressions require numeric macros; constexpr namespace values
// are NOT valid #if operands. Keep these assertions to prevent pin-map drift.
static_assert(BOAT_NRF_CE_PIN == boat_pins::NRF_CE, "R30H-33 pin-map mismatch: BOAT_NRF_CE_PIN");
static_assert(BOAT_NRF_CSN_PIN == boat_pins::NRF_CSN, "R30H-33 pin-map mismatch: BOAT_NRF_CSN_PIN");
static_assert(BOAT_NRF_SCK_PIN == boat_pins::NRF_SCK, "R30H-33 pin-map mismatch: BOAT_NRF_SCK_PIN");
static_assert(BOAT_NRF_MISO_PIN == boat_pins::NRF_MISO, "R30H-33 pin-map mismatch: BOAT_NRF_MISO_PIN");
static_assert(BOAT_NRF_MOSI_PIN == boat_pins::NRF_MOSI, "R30H-33 pin-map mismatch: BOAT_NRF_MOSI_PIN");
static_assert(BOAT_NRF_IRQ_PIN == boat_pins::NRF_IRQ, "R30H-33 pin-map mismatch: BOAT_NRF_IRQ_PIN");
static_assert(BOAT_E22_UART_PIN_TX == boat_pins::E22_TX, "R30H-33 pin-map mismatch: BOAT_E22_UART_PIN_TX");
static_assert(BOAT_E22_UART_PIN_RX == boat_pins::E22_RX, "R30H-33 pin-map mismatch: BOAT_E22_UART_PIN_RX");
static_assert(BOAT_E22_UART_PIN_M0 == boat_pins::E22_M0, "R30H-33 pin-map mismatch: BOAT_E22_UART_PIN_M0");
static_assert(BOAT_E22_UART_PIN_M1 == boat_pins::E22_M1, "R30H-33 pin-map mismatch: BOAT_E22_UART_PIN_M1");
static_assert(BOAT_E22_UART_PIN_AUX == boat_pins::E22_AUX, "R30H-33 pin-map mismatch: BOAT_E22_UART_PIN_AUX");
static_assert(BOAT_E22_UART_PIN_NRST == boat_pins::E22_NRST, "R30H-33 pin-map mismatch: BOAT_E22_UART_PIN_NRST");
