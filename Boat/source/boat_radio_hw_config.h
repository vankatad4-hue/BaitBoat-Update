#pragma once
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
