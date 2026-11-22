#pragma once
#include <stdint.h>
// R20: MCP23017 bank=0 default register map; port B pins 0/1 direction.
// This adapter does not initialize a physical bus, authorize motor power,
// control PWM, or replace the independent STOP/contactor.
class Mcp23017RegisterBus {
public:
 virtual ~Mcp23017RegisterBus() = default;
 virtual bool writeRegister(uint8_t address,uint8_t reg,uint8_t value)=0;
 virtual bool readRegister(uint8_t address,uint8_t reg,uint8_t &value)=0;
};
class Mcp23017DirectionAdapter {
public:
 explicit Mcp23017DirectionAdapter(Mcp23017RegisterBus &bus,uint8_t address=0x27)
   :bus_(bus),address_(address){}
 bool prepareDisabled() {
   ready_=false;
   if(address_<0x20||address_>0x27)return false;
   // IOCON=0x00: BANK=0, sequential register addressing.
   // OLATB=0x15, IODIRB=0x01, GPIOB=0x13.
   // Set latch low BEFORE enabling output drivers.
   if(!bus_.writeRegister(address_,0x0A,0x00))return false;
   if(!bus_.writeRegister(address_,0x15,0x00))return false;
   if(!bus_.writeRegister(address_,0x01,0xFC))return false;
   uint8_t actual=0xFF;
   if(!bus_.readRegister(address_,0x13,actual)|| (actual&0x03U)!=0U) {
     failClosed();return false;
   }
   ready_=true;return true;
 }
 // This function may only be called while independent motor power is isolated
 // and both PWM outputs are physically confirmed zero.
 bool setDirections(uint8_t bits,bool motorPowerIsolated,bool pwmZero) {
   if(!ready_||!motorPowerIsolated||!pwmZero||bits>3U) {
     failClosed();return false;
   }
   if(!bus_.writeRegister(address_,0x15,bits)) {
     failClosed();return false;
   }
   uint8_t actual=0xFF;
   if(!bus_.readRegister(address_,0x13,actual)|| (actual&3U)!=bits) {
     failClosed();return false;
   }
   return true;
 }
 bool ready() const {return ready_;}
 bool failClosed() {
   ready_=false;
   // Best effort only: I2C can fail; independent hardware power cut is REQUIRED.
   (void)bus_.writeRegister(address_,0x15,0x00);return false;
 }
private:
 Mcp23017RegisterBus &bus_;
 uint8_t address_;
 bool ready_=false;
};
