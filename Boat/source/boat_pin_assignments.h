#pragma once
// Canonical Boat module-only GPIO map for the active R20 hardware baseline.
// These are unsigned design assignments, not hardware signoff or output enable.
// The pin budget is committed: GPIO1/2/41/42 remain reserved, not free spares.
// Direct Open Echo sonar pins remain unassigned until the exact board/shield,
// ADC range, timing, and GPIO conflicts are physically checked.
#include <stdint.h>
namespace boat_pins {
constexpr int GNSS_TX=4, GNSS_RX=5, I2C_SCL=7, I2C_SDA=8;
constexpr int NRF_CSN=9, NRF_CE=10, NRF_MOSI=11, NRF_SCK=12, NRF_MISO=13, NRF_IRQ=-1;
// UART0 RX candidate for the separate sonar coprocessor; still unsigned.
constexpr int OPEN_ECHO_UART_RX=21;
constexpr int E22_NRST=6, E22_AUX=14, E22_M0=15, E22_M1=16, E22_TX=17, E22_RX=18;
constexpr int MDD_PWM1=39, MDD_PWM2=40;
constexpr uint8_t MCP_ADDR=0x27, MCP_DIR1_GPB=0, MCP_DIR2_GPB=1;
constexpr uint8_t MCP_HOPPER_LEFT_GPA=0, MCP_HOPPER_RIGHT_GPA=1;
constexpr uint8_t MCP_HOOK_LEFT_GPA=2, MCP_HOOK_RIGHT_GPA=3;
constexpr uint8_t INA231_ADDR=0x40, QMC5883L_ADDR=0x0D, MPU9250_ADDR=0x68;
constexpr uint8_t BMP280_ADDR=0x76;
constexpr int GNSS_PPS=-1; // UART UTC only
// Unassigned general-purpose pins are reserved for project safety design;
// they are not available for optional sensors or debug wiring.
constexpr int RESERVED_SAFETY_1=1, RESERVED_SAFETY_2=2;
constexpr int RESERVED_SAFETY_41=41, RESERVED_SAFETY_42=42;
// Keep legacy names for source compatibility; each aliases a reserved pin.
constexpr int SPARE1=RESERVED_SAFETY_1, SPARE2=RESERVED_SAFETY_2;
constexpr int SPARE41=RESERVED_SAFETY_41, SPARE42=RESERVED_SAFETY_42;
// Open Echo needs a dedicated CS, IO1/IO2, OUT3/OUT4 and analog VOUT input.
// No conflict-free, electrically reviewed Boat allocation is available yet.
constexpr bool OPEN_ECHO_GPIO_MAP_ASSIGNED=false;
constexpr int OPEN_ECHO_CS=-1, OPEN_ECHO_IO1=-1, OPEN_ECHO_IO2=-1;
constexpr int OPEN_ECHO_OUT3=-1, OPEN_ECHO_OUT4=-1, OPEN_ECHO_VOUT_ADC=-1;
static_assert(NRF_CSN!=NRF_CE && MDD_PWM1!=MDD_PWM2,"Boat GPIO conflict");
static_assert(MCP_HOOK_RIGHT_GPA<8 && MCP_DIR2_GPB<8,"MCP bit invalid");
static_assert(!OPEN_ECHO_GPIO_MAP_ASSIGNED && OPEN_ECHO_CS<0 &&
              OPEN_ECHO_VOUT_ADC<0,"Open Echo GPIO map requires physical review");
static_assert(OPEN_ECHO_UART_RX==21 && NRF_IRQ<0,
              "GPIO21 belongs to sonar UART RX; nRF IRQ is intentionally unconnected");
}
