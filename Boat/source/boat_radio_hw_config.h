#pragma once
#include "boat_pin_assignments_r30h32.h"
// R20 candidate mapping for ESP32-S3-DevKitC-1-N32R16V.
// Pin audit flags intentionally remain 0: board revision and harness not physically signed.
#define BOAT_NRF_CE_PIN boat_pins::NRF_CE
#define BOAT_NRF_CSN_PIN boat_pins::NRF_CSN
#define BOAT_NRF_CHANNEL 76
#define BOAT_NRF_SCK_PIN boat_pins::NRF_SCK
#define BOAT_NRF_MISO_PIN boat_pins::NRF_MISO
#define BOAT_NRF_MOSI_PIN boat_pins::NRF_MOSI
#define BOAT_NRF_IRQ_PIN boat_pins::NRF_IRQ
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
#define BOAT_E22_UART_PIN_TX boat_pins::E22_TX
#define BOAT_E22_UART_PIN_RX boat_pins::E22_RX
#define BOAT_E22_UART_PIN_M0 boat_pins::E22_M0
#define BOAT_E22_UART_PIN_M1 boat_pins::E22_M1
#define BOAT_E22_UART_PIN_AUX boat_pins::E22_AUX
#define BOAT_E22_UART_PIN_NRST boat_pins::E22_NRST
#define BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED 0
#define BOAT_E22_UART_LOGIC_LEVELS_VERIFIED 0
#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0
#define BOAT_E22_RF_AIRTIME_AUDIT_SIGNED 0
// R30H-2: optional BOAT-LOCAL, USB-only read-only event trace. Disabled by default.
// May be enabled only in the non-actuating diagnostic target after physical logic-level checks.
#define BOAT_R30H_LOCAL_TRACE_ENABLED 0
