#pragma once
// R30H-32 canonical Remote wiring map: Waveshare Standard 4.3, no stolen SD GPIO.
// No dedicated hardware failsafe devices.
#include <stdint.h>
namespace remote_pins {
constexpr int I2C_SDA=8, I2C_SCL=9;
constexpr int TF_MOSI=11, TF_SCK=12, TF_MISO=13, SD_DUMMY_CS=6;
constexpr int RS485_TX=15, RS485_RX=16;
constexpr int NRF_DIRECT_CE=-1, NRF_DIRECT_CSN=-1, NRF_DIRECT_IRQ=-1;
constexpr uint8_t BUTTONS_MCP=0x20, RADIO_IO_MCP=0x21, NRF_CE_GPA=0, OK_GPA=4;
constexpr uint8_t ADS1115_AXES=0x4A, SC16IS750_E22=0x48, SC18IS606_NRF=0x28;
constexpr uint8_t E22_M0_GPIO=0, E22_M1_GPIO=1, E22_AUX_GPIO=2, E22_NRST_GPIO=3;
constexpr uint8_t BUTTON_BANK_COUNT=15, PHYSICAL_BUTTON_COUNT=16;
constexpr int GPS_PPS=-1; // UTC telemetry, not physical PPS
static_assert(NRF_DIRECT_CE<0 && NRF_DIRECT_CSN<0,"Remote nRF must use isolated bridge");
static_assert(BUTTONS_MCP!=RADIO_IO_MCP && NRF_CE_GPA!=OK_GPA,"Remote MCP conflict");
}
