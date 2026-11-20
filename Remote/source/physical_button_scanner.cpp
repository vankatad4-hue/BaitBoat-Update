#include "physical_button_scanner.h"
#include "release_button_guard.h"
#include <Wire.h>
static constexpr uint8_t BTN_EXP=0x20, RADIO_IO=0x21;
static constexpr uint8_t IODIRA=0x00, IODIRB=0x01, GPPUA=0x0C, GPPUB=0x0D, GPIOA=0x12, GPIOB=0x13, OK_BIT=4;
static constexpr uint32_t DEBOUNCE_MS=25;
static PhysicalButtonCallback cb=nullptr;
static uint16_t stable=0xFFFF, candidate=0xFFFF;
static uint32_t debounceAt[16]={};
static bool confirmStable=true, confirmCandidate=true, ready=false;
static uint32_t confirmAt=0;
static bool wr(uint8_t a,uint8_t r,uint8_t v){Wire.beginTransmission(a);Wire.write(r);Wire.write(v);return Wire.endTransmission()==0;}
static bool rd(uint8_t a,uint8_t r,uint8_t &v){Wire.beginTransmission(a);Wire.write(r);if(Wire.endTransmission(false)!=0)return false;if(Wire.requestFrom((int)a,1)!=1)return false;v=(uint8_t)Wire.read();return true;}
void physicalButtons_setCallback(PhysicalButtonCallback f){cb=f;}
bool physicalButtons_begin(){
 if(!wr(BTN_EXP,IODIRA,0xFF)||!wr(BTN_EXP,IODIRB,0xFF)||!wr(BTN_EXP,GPPUA,0xFF)||!wr(BTN_EXP,GPPUB,0x7F))return false;
 uint8_t dir=0,pull=0;if(!rd(RADIO_IO,IODIRA,dir)||!rd(RADIO_IO,GPPUA,pull))return false;
 dir|=(1U<<OK_BIT);pull|=(1U<<OK_BIT);
 if(!wr(RADIO_IO,IODIRA,dir)||!wr(RADIO_IO,GPPUA,pull))return false;
 uint8_t a=0,b=0,c=0;if(!rd(BTN_EXP,GPIOA,a)||!rd(BTN_EXP,GPIOB,b)||!rd(RADIO_IO,GPIOA,c))return false;
 stable=candidate=((uint16_t)a|((uint16_t)(b&0x7FU)<<8)|0x8000U);confirmStable=confirmCandidate=(c&(1U<<OK_BIT))!=0;ready=true;return true;
}
void physicalButtons_update(uint32_t now){
 if(!ready)return;
 uint8_t a=0,b=0,c=0;if(!rd(BTN_EXP,GPIOA,a)||!rd(BTN_EXP,GPIOB,b)||!rd(RADIO_IO,GPIOA,c)){releaseGuard_cancel();return;}
 uint16_t raw=((uint16_t)a|((uint16_t)(b&0x7FU)<<8)|0x8000U);
 for(uint8_t i=0;i<15;i++){
  bool rp=(raw&(1U<<i))==0,sp=(stable&(1U<<i))==0,cp=(candidate&(1U<<i))==0;
  if(rp!=cp){if(rp)candidate&=(uint16_t)~(1U<<i);else candidate|=(uint16_t)(1U<<i);debounceAt[i]=now;}
  else if(rp!=sp&&(uint32_t)(now-debounceAt[i])>=DEBOUNCE_MS){
   if(rp)stable&=(uint16_t)~(1U<<i);else stable|=(uint16_t)(1U<<i);
   if(i>=8&&i<=11){ReleaseChannel q=(ReleaseChannel)(i-8);if(rp)releaseGuard_beginKey(q,now);else releaseGuard_endKey(now);}
   else if(cb)cb(i,rp);
  }
 }
 bool rk=(c&(1U<<OK_BIT))!=0;
 if(rk!=confirmCandidate){confirmCandidate=rk;confirmAt=now;}
 else if(rk!=confirmStable&&(uint32_t)(now-confirmAt)>=DEBOUNCE_MS){confirmStable=rk;if(!rk)releaseGuard_beginConfirm(now);else releaseGuard_endConfirm(now);}
 releaseGuard_tick(now);
}
uint16_t physicalButtons_state(){return stable;}
