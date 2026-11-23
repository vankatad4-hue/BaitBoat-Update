#include "../source/boat_r23_gpio_connection_contract.h"
#include "../source/boat_radio_hw_config.h"
#include <cassert>
#include <cstring>
#include <iostream>

static bool contains(const char* net,uint8_t gpio,const char* header){
 for(const auto& p:boat_r23::assigned)
  if(std::strcmp(p.net,net)==0&&p.gpio==gpio&&std::strcmp(p.header,header)==0)return true;
 return false;
}
int main(){
 const auto& c=boat_r23::assigned;
 assert(sizeof(c)/sizeof(c[0])==18);
 bool used[49]={};
 bool reserved[49]={};
 for(uint8_t p:boat_r23::reserved){assert(p<=48);reserved[p]=true;}
 for(const auto& p:c){
  assert(p.gpio<=48&&!used[p.gpio]&&!reserved[p.gpio]);
  used[p.gpio]=true;
  assert(p.header[0]=='J'&&p.header[1]!='\0');
 }
 assert(contains("M100_UART_TX",4,"J1-4"));
 assert(contains("M100_UART_RX",5,"J1-5"));
 assert(contains("SHARED_I2C_SCL",7,"J1-7"));
 assert(contains("SHARED_I2C_SDA",8,"J1-12"));
 assert(contains("NRF_CSN",BOAT_NRF_CSN_PIN,"J1-15"));
 assert(contains("NRF_CE",BOAT_NRF_CE_PIN,"J1-16"));
 assert(contains("NRF_SCK",BOAT_NRF_SCK_PIN,"J1-18"));
 assert(contains("NRF_MOSI",BOAT_NRF_MOSI_PIN,"J1-17"));
 assert(contains("NRF_MISO",BOAT_NRF_MISO_PIN,"J1-19"));
 assert(contains("NRF_IRQ",BOAT_NRF_IRQ_PIN,"J3-18"));
 assert(contains("E22_TX",BOAT_E22_UART_PIN_TX,"J1-10"));
 assert(contains("E22_RX",BOAT_E22_UART_PIN_RX,"J1-11"));
 assert(contains("E22_NRST",BOAT_E22_UART_PIN_NRST,"J1-6"));
 assert(contains("E22_M0",BOAT_E22_UART_PIN_M0,"J1-8"));
 assert(contains("E22_M1",BOAT_E22_UART_PIN_M1,"J1-9"));
 assert(contains("E22_AUX",BOAT_E22_UART_PIN_AUX,"J1-20"));
 assert(contains("MDD_PWM1",39,"J3-9"));
 assert(contains("MDD_PWM2",40,"J3-8"));
 assert(!boat_r23::PHYSICAL_GPIO_AUDIT_SIGNED);
 assert(!boat_r23::ACTUATOR_OUTPUTS_COMMISSIONED);
 assert(!boat_r23::SENSOR_POWER_SEQUENCE_ACCEPTED);
 assert(boat_r23::I2C_MCP23017==0x27);
 assert(boat_r23::I2C_INA231==0x40);
 assert(boat_r23::I2C_QMC5883L==0x0d);
 assert(boat_r23::MCP_EKM_GPA_MASK==0x0f);
 assert(boat_r23::MCP_MDD_GPB_MASK==0x03);
 assert(BOAT_NRF_PIN_AUDIT_SIGNED==0);
 assert(BOAT_E22_UART_PIN_AUDIT_SIGNED==0);
 assert(BOAT_E22_UART_LOGIC_LEVELS_VERIFIED==0);
 assert(BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED==0);
 std::cout<<"R23 18-pin candidate GPIO contract: independent, no collisions, no outputs\n";
}
