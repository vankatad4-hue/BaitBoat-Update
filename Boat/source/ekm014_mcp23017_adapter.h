#pragma once
#include <stdint.h>
#include "mcp23017_direction_adapter.h"
#include "ekm014_logical_interface_r16.h"

// Low-level R20 I2C expander adapter: MCP23017 GPA0..3 -> EKM014 IN1..IN4.
// This is only an output transport. It does NOT replace Ekm014PulseGuard,
// independent cutoff, physical current-zero feedback or the Boat supervisor.
class Ekm014Mcp23017Adapter {
public:
 explicit Ekm014Mcp23017Adapter(Mcp23017RegisterBus &bus,uint8_t address=0x27)
  :bus_(bus),address_(address){}
 bool beginDisabled(){
  ready_=false;activeMask_=0;
  if(address_<0x20||address_>0x27)return false;
  // BANK=0. Clear OLATA before changing GPA0..3 from inputs to outputs.
  if(!bus_.writeRegister(address_,0x0A,0x00))return false;
  if(!bus_.writeRegister(address_,0x14,0x00))return false;
  if(!bus_.writeRegister(address_,0x00,0xF0))return failClosed();
  uint8_t actual=0xFF;
  if(!bus_.readRegister(address_,0x12,actual)||(actual&0x0FU)!=0U)return failClosed();
  ready_=true;return true;
 }
 bool setChannel(flytec_ekm014_r16::Channel channel){
  const uint8_t mask=flytec_ekm014_r16::logicalMask(channel);
  if(!ready_||!flytec_ekm014_r16::isSingleChannelMask(mask))return false;
  if(activeMask_!=0U&&activeMask_!=mask)return false; // never switch channels hot
  if(!bus_.writeRegister(address_,0x14,mask))return failClosed();
  uint8_t actual=0xFF;
  if(!bus_.readRegister(address_,0x12,actual)||(actual&0x0FU)!=mask)return failClosed();
  activeMask_=mask;return true;
 }
 bool allOff(){
  if(!bus_.writeRegister(address_,0x14,0x00))return failClosed();
  uint8_t actual=0xFF;
  if(!bus_.readRegister(address_,0x12,actual)||(actual&0x0FU)!=0U)return failClosed();
  activeMask_=0;return true;
 }
 bool ready()const{return ready_;}
 uint8_t activeMask()const{return activeMask_;}
 bool failClosed(){
  ready_=false;activeMask_=0;
  // Best effort only: an I2C failure can leave the expander latch unchanged.
  return bus_.writeRegister(address_,0x14,0x00);
 }
private:
 Mcp23017RegisterBus &bus_;
 uint8_t address_;
 uint8_t activeMask_=0;
 bool ready_=false;
};
