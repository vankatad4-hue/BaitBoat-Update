/*
  Flytec V080 radio configuration — nRF24L01+ PA/LNA selected on boat and remote.
  Archive: Flytec_Boat_ver002.zip

  This file deliberately contains no GPIO assignments. Confirm the exact ESP32
  module, carrier schematic, SPI bus availability and all peripheral conflicts
  before assigning pins. Do not power a PA/LNA module from an unverified ESP32
  3.3 V rail. See docs/RADIO_LINK_DESIGN.md.
*/
#pragma once

// Initial radio test configuration (not yet applied by an operational driver).
#define FLYTEC_RADIO_NRF24L01_PA_LNA 1
#define FLYTEC_RADIO_INITIAL_DATA_RATE_KBPS 250
#define FLYTEC_RADIO_MAX_RF_PAYLOAD_BYTES 32
#define FLYTEC_RADIO_TARGET_JOYSTICK_HZ 20
#define FLYTEC_RADIO_TARGET_RANGE_METRES 500

// TODO: Assign SPI SCK/MOSI/MISO, CE, CSN and optional IRQ after pin audit.
// TODO: Select a compliant 2.4 GHz channel and verify antenna/EIRP.
// TODO: Implement shared packet format, acknowledgements, security and watchdog.
