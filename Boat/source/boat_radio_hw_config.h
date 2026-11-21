#pragma once
// R8: Boat LoRa is the internal SPI SX1262; not an external E22 UART.
// These defaults deliberately prohibit radio hardware initialization.
#define BOAT_NRF_CE_PIN (-1)
#define BOAT_NRF_CSN_PIN (-1)
#define BOAT_NRF_CHANNEL 76
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
// V1.1 reference ONLY (not audited on V1.2): NSS8 SCK9 MOSI10
// MISO11 RST12 BUSY13 DIO1=14. Never copy as final pin assignments.
