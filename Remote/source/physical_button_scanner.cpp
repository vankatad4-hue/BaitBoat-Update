#include "physical_button_scanner.h"
#include "remote_pin_assignments_r30h32.h"
#include "release_button_guard.h"
#include <Wire.h>
#include "remote_i2c_owner.h"
static constexpr uint8_t BTN_EXP=remote_pins::BUTTONS_MCP, RADIO_IO=remote_pins::RADIO_IO_MCP;
static constexpr uint8_t IODIRA=0x00, IODIRB=0x01, GPPUA=0x0C, GPPUB=0x0D, GPIOA=0x12, GPIOB=0x13, OK_BIT=remote_pins::OK_GPA;
static constexpr uint32_t DEBOUNCE_MS=25;
static PhysicalButtonCallback cb=nullptr;
static uint16_t stable=0xFFFF, candidate=0xFFFF;
static uint32_t debounceAt[16]={};
static bool confirmStable=true, confirmCandidate=true, ready=false;
static uint32_t confirmAt=0;
static bool inputFault=false,releaseBlocked=false,okOwned=false;
static bool wr(uint8_t a,uint8_t r,uint8_t v){if(!flytec_rtos::remoteI2cAllowed())return false;Wire.beginTransmission(a);Wire.write(r);Wire.write(v);return Wire.endTransmission()==0;}
static bool rd(uint8_t a,uint8_t r,uint8_t &v){if(!flytec_rtos::remoteI2cAllowed())return false;Wire.beginTransmission(a);Wire.write(r);if(Wire.endTransmission(false)!=0)return false;if(Wire.requestFrom((int)a,1)!=1)return false;v=(uint8_t)Wire.read();return true;}
void physicalButtons_setCallback(PhysicalButtonCallback f){cb=f;}
bool physicalButtons_begin(){
 ready=false;inputFault=false;releaseBlocked=false;okOwned=false;releaseGuard_cancel();
 if(!wr(BTN_EXP,IODIRA,0xFF)||!wr(BTN_EXP,IODIRB,0xFF)||!wr(BTN_EXP,GPPUA,0xFF)||!wr(BTN_EXP,GPPUB,0xFF))return false;
 uint8_t dir=0,pull=0;if(!rd(RADIO_IO,IODIRA,dir)||!rd(RADIO_IO,GPPUA,pull))return false;
 dir|=(1U<<OK_BIT);pull|=(1U<<OK_BIT);
 if(!wr(RADIO_IO,IODIRA,dir)||!wr(RADIO_IO,GPPUA,pull))return false;
 uint8_t a=0,b=0,c=0;if(!rd(BTN_EXP,GPIOA,a)||!rd(BTN_EXP,GPIOB,b)||!rd(RADIO_IO,GPIOA,c))return false;
 stable=candidate=((uint16_t)a|((uint16_t)(b&0x7FU)<<8)|0x8000U);confirmStable=confirmCandidate=(c&(1U<<OK_BIT))!=0;
 // Held keys at boot must not generate UI or release actions. Request STOP and
 // require every input to return to neutral before processing any new press.
 if(stable!=0xFFFF||!confirmStable){inputFault=true;releaseBlocked=true;okOwned=true;if(cb)cb(0,true);}
 ready=true;return true;
}
void physicalButtons_update(uint32_t now){
 if(!ready)return;
 uint8_t a=0,b=0,c=0;
 if(!rd(BTN_EXP,GPIOA,a)||!rd(BTN_EXP,GPIOB,b)||!rd(RADIO_IO,GPIOA,c)){
  releaseGuard_cancel();releaseBlocked=true;okOwned=true;
  if(!inputFault&&cb)cb(0,true);
  inputFault=true;return;
 }
 uint16_t raw=((uint16_t)a|((uint16_t)(b&0x7FU)<<8)|0x8000U);
 bool rk=(c&(1U<<OK_BIT))!=0;
 if(inputFault){
  if(raw!=0xFFFF||!rk)return;
  stable=candidate=raw;confirmStable=confirmCandidate=true;inputFault=false;releaseBlocked=false;okOwned=false;return;
 }
 uint16_t releases=(uint16_t)((~raw)&0x0F00U);
 if((releases&&(releases&(releases-1)))||!(raw&1U)||(releases&&!rk)){
  releaseGuard_cancel();releaseBlocked=true;okOwned=true;
 }
 for(uint8_t i=0;i<remote_pins::BUTTON_BANK_COUNT;i++){
  bool rp=(raw&(1U<<i))==0,sp=(stable&(1U<<i))==0,cp=(candidate&(1U<<i))==0;
  if(rp!=cp){if(rp)candidate&=(uint16_t)~(1U<<i);else candidate|=(uint16_t)(1U<<i);debounceAt[i]=now;}
  else if(rp!=sp&&(uint32_t)(now-debounceAt[i])>=DEBOUNCE_MS){
   if(rp)stable&=(uint16_t)~(1U<<i);else stable|=(uint16_t)(1U<<i);
   if(i>=8&&i<=11){
    ReleaseChannel q=(ReleaseChannel)(i-8);
    if(!releaseBlocked){if(rp)releaseGuard_beginKey(q,now);else if(releaseGuard_pendingChannel()==q)releaseGuard_endKey(now);}
   }else if(cb&&((raw&1U)||i==0)){if(rp&&i!=0)releaseGuard_cancel();cb(i,rp);}
  }
 }
 if(rk!=confirmCandidate){confirmCandidate=rk;confirmAt=now;}
 else if(rk!=confirmStable&&(uint32_t)(now-confirmAt)>=DEBOUNCE_MS){
  confirmStable=rk;
  if(!rk){okOwned=releaseBlocked||releaseGuard_state()!=RELEASE_GUARD_IDLE;if(okOwned&&!releaseBlocked)releaseGuard_beginConfirm(now);}
  else {if(okOwned)releaseGuard_endConfirm(now);else if(!releaseBlocked&&cb){cb(15,true);cb(15,false);}okOwned=false;}
 }
 releaseGuard_tick(now);
 if(!releases&&rk&&(stable&0x0F00U)==0x0F00U&&confirmStable&&(raw&1U))releaseBlocked=false;
}
uint16_t physicalButtons_state(){return stable;}
