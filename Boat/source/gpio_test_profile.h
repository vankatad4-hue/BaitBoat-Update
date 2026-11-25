#pragma once
// Read-only GPIO allocation audit. Never configures GPIO or actuators.
#include "boat_radio_hw_config.h"
namespace flytec_h10_boat {
constexpr int GNSS_TX=4, GNSS_RX=5, SCL=7, SDA=8;
constexpr int NRF_CSN=9, NRF_CE=10, MOSI=11, SCK=12, MISO=13, IRQ=21;
constexpr int E22_NRST=6,E22_AUX=14,E22_M0=15,E22_M1=16,E22_TX=17,E22_RX=18;
constexpr int MDD_PWM1=39,MDD_PWM2=40;
constexpr int SAFETY1=1,SAFETY2=2,SAFETY41=41,SAFETY42=42;
constexpr int PPS=-1;
}
static_assert(flytec_h10_boat::NRF_CE==BOAT_NRF_CE_PIN &&
              flytec_h10_boat::NRF_CSN==BOAT_NRF_CSN_PIN &&
              flytec_h10_boat::SCK==BOAT_NRF_SCK_PIN &&
              flytec_h10_boat::MISO==BOAT_NRF_MISO_PIN &&
              flytec_h10_boat::MOSI==BOAT_NRF_MOSI_PIN &&
              flytec_h10_boat::IRQ==BOAT_NRF_IRQ_PIN,"Boat RF candidate drift");
static_assert(flytec_h10_boat::E22_TX==BOAT_E22_UART_PIN_TX &&
              flytec_h10_boat::E22_RX==BOAT_E22_UART_PIN_RX &&
              flytec_h10_boat::E22_M0==BOAT_E22_UART_PIN_M0 &&
              flytec_h10_boat::E22_M1==BOAT_E22_UART_PIN_M1 &&
              flytec_h10_boat::E22_AUX==BOAT_E22_UART_PIN_AUX &&
              flytec_h10_boat::E22_NRST==BOAT_E22_UART_PIN_NRST,"Boat E22 candidate drift");
static_assert(BOAT_NRF_PIN_AUDIT_SIGNED==0 &&
              BOAT_E22_UART_PIN_AUDIT_SIGNED==0 &&
              flytec_h10_boat::PPS==-1,"Physical GPIO/PPS gates must remain unsigned");
