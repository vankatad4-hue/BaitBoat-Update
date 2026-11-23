#pragma once
#include <stdint.h>
#include "boat_pin_assignments_r30h32.h"
// R23 MODULE-ONLY BOAT PIN CONTRACT — candidate header mapping verified
// against published Espressif DevKitC-1 v1.1 J1/J3 headers.
// Physical PCB revision, connector orientation, signal levels and actual
// safety feedback are still UNSIGNED. This file must NEVER enable GPIOs.
namespace boat_r23 {
struct Pin {const char *net; uint8_t gpio; const char *header;};
static constexpr Pin assigned[]={
 {"M100_UART_TX",4,"J1-4"}, {"M100_UART_RX",5,"J1-5"},
 {"SHARED_I2C_SCL",7,"J1-7"}, {"SHARED_I2C_SDA",8,"J1-12"},
 {"NRF_CSN",9,"J1-15"}, {"NRF_CE",10,"J1-16"},
 {"NRF_MOSI",11,"J1-17"}, {"NRF_SCK",12,"J1-18"},
 {"NRF_MISO",13,"J1-19"}, {"NRF_IRQ",21,"J3-18"},
 {"E22_NRST",6,"J1-6"}, {"E22_M0",15,"J1-8"},
 {"E22_M1",16,"J1-9"}, {"E22_TX",17,"J1-10"},
 {"E22_RX",18,"J1-11"}, {"E22_AUX",14,"J1-20"},
 {"MDD_PWM1",39,"J3-9"}, {"MDD_PWM2",40,"J3-8"}
};
static constexpr uint8_t reserved[]={
  0,3,19,20,35,36,37,38,43,44,45,46,47,48
};
// GPIO1/2/41/42 are available spares; no safety interface assigned;
// GPIO47/48 are 1.8V on the selected ESP32-S3-WROOM-2 'V' module.
// 39/40 are repurposed JTAG and cannot simultaneously provide JTAG debug.
static constexpr uint8_t I2C_MCP23017=boat_pins::MCP_ADDR;
static constexpr uint8_t I2C_INA231=boat_pins::INA231_ADDR;
static constexpr uint8_t I2C_QMC5883L=0x0D;
static constexpr uint8_t I2C_MPU9250=0x68;
static constexpr uint8_t I2C_BMP280_PRIMARY=0x76;
static constexpr uint8_t I2C_BMP280_ALTERNATE=0x77;
static constexpr uint8_t MCP_EKM_GPA_MASK=0x0f;
static constexpr uint8_t MCP_MDD_GPB_MASK=0x03;
static constexpr bool PHYSICAL_GPIO_AUDIT_SIGNED=false;
static constexpr bool ACTUATOR_OUTPUTS_COMMISSIONED=false;
static constexpr bool SENSOR_POWER_SEQUENCE_ACCEPTED=false;
}
