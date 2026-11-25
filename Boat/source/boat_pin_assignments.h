#pragma once
// R30H-32 canonical Boat module-only wiring map (2026-11-23).
// These are design assignments, NOT hardware signoff or output enable.
#include <stdint.h>
namespace boat_pins {
constexpr int GNSS_TX=4, GNSS_RX=5, I2C_SCL=7, I2C_SDA=8;
constexpr int NRF_CSN=9, NRF_CE=10, NRF_MOSI=11, NRF_SCK=12, NRF_MISO=13, NRF_IRQ=21;
constexpr int E22_NRST=6, E22_AUX=14, E22_M0=15, E22_M1=16, E22_TX=17, E22_RX=18;
constexpr int MDD_PWM1=39, MDD_PWM2=40;
constexpr uint8_t MCP_ADDR=0x27, MCP_DIR1_GPB=0, MCP_DIR2_GPB=1;
constexpr uint8_t MCP_HOPPER_LEFT_GPA=0, MCP_HOPPER_RIGHT_GPA=1;
constexpr uint8_t MCP_HOOK_LEFT_GPA=2, MCP_HOOK_RIGHT_GPA=3;
constexpr uint8_t INA231_ADDR=0x40, QMC5883L_ADDR=0x0D, MPU9250_ADDR=0x68;
constexpr uint8_t BMP280_ADDR=0x76;
constexpr int GNSS_PPS=-1; // UART UTC only
constexpr int SPARE1=1, SPARE2=2, SPARE41=41, SPARE42=42;
static_assert(NRF_CSN!=NRF_CE && MDD_PWM1!=MDD_PWM2,"Boat GPIO conflict");
static_assert(MCP_HOOK_RIGHT_GPA<8 && MCP_DIR2_GPB<8,"MCP bit invalid");
}
