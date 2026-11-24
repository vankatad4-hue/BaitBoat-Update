#include "joystick_adc.h"
#include <Wire.h>
#include "remote_i2c_owner.h"

static JoystickAdcConfig cfg{0x4A,0,1,2,3};
static bool ready=false;
static const uint8_t ADS_REG_CONVERSION=0x00, ADS_REG_CONFIG=0x01;
static const uint16_t PGA_4096=0x0200, MODE_SINGLE=0x0100, RATE_860SPS=0x00E0, COMP_DISABLE=0x0003;

static bool writeRegister(uint8_t reg,uint16_t value){
  if(!flytec_rtos::remoteI2cAllowed())return false;
  Wire.beginTransmission(cfg.address); Wire.write(reg); Wire.write((uint8_t)(value>>8)); Wire.write((uint8_t)value);
  return Wire.endTransmission()==0;
}
static bool readRegister(uint8_t reg,uint16_t &value){
  if(!flytec_rtos::remoteI2cAllowed())return false;
  Wire.beginTransmission(cfg.address); Wire.write(reg);
  if(Wire.endTransmission(false)!=0) return false;
  if(Wire.requestFrom((int)cfg.address,2)!=2) return false;
  value=(uint16_t)Wire.read()<<8; value|=(uint8_t)Wire.read(); return true;
}
static bool readChannel(uint8_t channel,int &out){
  if(channel>3) return false;
  // MUX=100..111 selects AIN0..AIN3 against GND; OS starts conversion.
  const uint16_t mux=(uint16_t)(0x4000U+((uint16_t)channel<<12));
  if(!writeRegister(ADS_REG_CONFIG,(uint16_t)(0x8000U|mux|PGA_4096|MODE_SINGLE|RATE_860SPS|COMP_DISABLE))) return false;
  const uint32_t started=millis();
  uint16_t status=0;
  do{
    if(!readRegister(ADS_REG_CONFIG,status)) return false;
    if(status&0x8000U) break;
  }while((uint32_t)(millis()-started)<8U);
  if(!(status&0x8000U)) return false;
  uint16_t raw=0; if(!readRegister(ADS_REG_CONVERSION,raw)) return false;
  // Convert signed ADS1115 counts to an unsigned joystick-domain 0..4095.
  const int16_t signedRaw=(int16_t)raw;
  if(signedRaw<0) return false; // negative input is wiring fault / outside expected range
  out=constrain((int)signedRaw,0,32767)*4095L/32767L;
  return true;
}
bool joystickAdc_begin(const JoystickAdcConfig &config){
  // A reinitialisation failure must revoke any previous ready state.
  ready=false;
  if(config.address<0x48||config.address>0x4B)return false;
  const uint8_t channels[4]={config.channelLeftX,config.channelLeftY,
                              config.channelRightX,config.channelRightY};
  uint8_t used=0;
  for(const uint8_t channel:channels){
    if(channel>3||(used&(1U<<channel)))return false;
    used|=(uint8_t)(1U<<channel);
  }
  cfg=config;
  uint16_t configRegister=0;
  ready=readRegister(ADS_REG_CONFIG,configRegister);
  return ready;
}
bool joystickAdc_read(int &leftX,int &leftY,int &rightX,int &rightY){
  if(!ready)return false;
  // Publish a complete 4-axis frame or nothing; never expose half-read data.
  int lx=0,ly=0,rx=0,ry=0;
  if(!readChannel(cfg.channelLeftX,lx)||!readChannel(cfg.channelLeftY,ly)||
     !readChannel(cfg.channelRightX,rx)||!readChannel(cfg.channelRightY,ry))
      return false;
  leftX=lx;leftY=ly;rightX=rx;rightY=ry;
  return true;
}
bool joystickAdc_isReady(){return ready;}
