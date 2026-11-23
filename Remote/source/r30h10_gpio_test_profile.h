#pragma once
// Waveshare 4.3 Standard SKU25948: read-only pin/resource constraints.
#include "radio_hw_config.h"
namespace flytec_h10_remote {
constexpr int SDA=8,SCL=9,MOSI=11,SCK=12,MISO=13;
constexpr int SD_DUMMY_CS=6,RS485_TX=15,RS485_RX=16;
constexpr int NRF_CE=-1,NRF_CSN=-1,NRF_IRQ=-1,PPS=-1;
constexpr int BUTTONS_MCP=0x20,OK_MCP=0x21;
constexpr int ADS1115=0x4A,SC16IS750=0x48;
constexpr int NRF_BRIDGE_I2C=0x28,NRF_BRIDGE_CE_SS2=2,NRF_BRIDGE_CSN_SS0=0;
}
static_assert(REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED==0 &&
              REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED==0 &&
              REMOTE_NRF_BRIDGE_I2C_ADDRESS==0x28,"Bridge hardware unsigned");
static_assert(REMOTE_NRF_CE_PIN==-1 && REMOTE_NRF_CSN_PIN==-1 &&
              REMOTE_NRF_PIN_AUDIT_SIGNED==0,"Remote RF GPIO must stay unassigned");
static_assert(REMOTE_SD_DUMMY_CSN_PIN==6 &&
              REMOTE_R30H10_REJECTED_CE_GPIO==6 &&
              REMOTE_R30H10_REJECTED_CSN_GPIO==15,"Known GPIO6/15 collisions");
static_assert(flytec_h10_remote::MOSI==REMOTE_TF_SPI_MOSI_PIN &&
              flytec_h10_remote::SCK==REMOTE_TF_SPI_SCK_PIN &&
              flytec_h10_remote::MISO==REMOTE_TF_SPI_MISO_PIN &&
              REMOTE_NRF_MOSI_PIN==-1 && REMOTE_NRF_SCK_PIN==-1 &&
              REMOTE_NRF_MISO_PIN==-1 && flytec_h10_remote::PPS==-1,
              "TF-owned ESP SPI and isolated nRF bridge pin drift");
