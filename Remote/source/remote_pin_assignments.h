#pragma once
// Canonical Remote map: Waveshare ESP32-S3 Touch LCD 4.3 Standard, SKU 25948.
// Board-owned display/storage/USB/RS485 pins stay reserved for those devices.
// External modules use the documented I2C/SPI/UART bridges and expanders below;
// no direct ESP32 GPIO is available for the sonar (depth arrives over radio).
#include <stdint.h>
namespace remote_pins {
constexpr int I2C_SDA=8, I2C_SCL=9;
constexpr int TF_MOSI=11, TF_SCK=12, TF_MISO=13, SD_DUMMY_CS=6;
constexpr int RS485_TX=15, RS485_RX=16;
constexpr int UART0_TX=43, UART0_RX=44; // USB-UART bridge; reserve for console
constexpr int USB_D_MINUS=19, USB_D_PLUS=20; // native USB; do not repurpose
constexpr int TOUCH_IRQ=4;
// Waveshare RGB panel bus. These are board-owned output pins.
constexpr int LCD_DE=5, LCD_VSYNC=3, LCD_HSYNC=46, LCD_PCLK=7;
constexpr int LCD_R0=1, LCD_R1=2, LCD_R2=42, LCD_R3=41, LCD_R4=40;
constexpr int LCD_G0=39, LCD_G1=0, LCD_G2=45, LCD_G3=48, LCD_G4=47, LCD_G5=21;
constexpr int LCD_B0=14, LCD_B1=38, LCD_B2=18, LCD_B3=17, LCD_B4=10;
constexpr int BOARD_RESERVED_GPIO35=35, BOARD_RESERVED_GPIO36=36, BOARD_RESERVED_GPIO37=37;
constexpr int NRF_DIRECT_CE=-1, NRF_DIRECT_CSN=-1, NRF_DIRECT_IRQ=-1;
constexpr uint8_t BUTTONS_MCP=0x20, RADIO_IO_MCP=0x21, NRF_CE_GPA=0, OK_GPA=4;
constexpr uint8_t ADS1115_AXES=0x4A, SC16IS750_E22=0x48, SC18IS606_NRF=0x28;
constexpr uint8_t CH422G_TF_CS_EXIO=4, CH422G_TOUCH_RESET_EXIO=1;
constexpr uint8_t CH422G_BACKLIGHT_EXIO=2, CH422G_USB_CAN_SELECT_EXIO=5;
constexpr uint8_t E22_M0_GPIO=0, E22_M1_GPIO=1, E22_AUX_GPIO=2, E22_NRST_GPIO=3;
constexpr uint8_t BUTTON_BANK_COUNT=15, PHYSICAL_BUTTON_COUNT=16;
constexpr int GPS_PPS=-1; // UTC telemetry, not physical PPS
static_assert(NRF_DIRECT_CE<0 && NRF_DIRECT_CSN<0,"Remote nRF must use isolated bridge");
static_assert(BUTTONS_MCP!=RADIO_IO_MCP && NRF_CE_GPA!=OK_GPA,"Remote MCP conflict");
static_assert(TF_MOSI==11 && TF_SCK==12 && TF_MISO==13 && TOUCH_IRQ==4,
              "Remote onboard peripheral ownership changed");
}
