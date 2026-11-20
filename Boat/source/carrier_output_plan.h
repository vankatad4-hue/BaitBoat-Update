#pragma once
#include "boat_hardware_supervisor.h"
// Cytron MDD20A: PWM low brakes; DIR low/high controls polarity.
// Independent carrier gate must isolate ALL actuator power on a fault.
struct CarrierOutputWords{uint16_t pwmLeft,pwmRight;bool dirLeft,dirRight;uint8_t solenoidMask;};
class CarrierOutputPlan{
public:
 bool configure(bool commissioned,bool reverseLeft,bool reverseRight,uint16_t maxDuty,uint16_t reversalDelayMs){
  configured_=commissioned&&maxDuty>0&&maxDuty<=1023&&reversalDelayMs>=20&&reversalDelayMs<=1000;
  invertLeft_=reverseLeft;invertRight_=reverseRight;limit_=maxDuty;delay_=reversalDelayMs;reset();return configured_;
 }
 void reset(){left_={};right_={};lastOff_=true;lastChannel_=-1;faulted_=false;}
 bool convert(const BoatActuatorOutput &in,uint32_t now,bool interlocks,CarrierOutputWords &out){
  out={};
  if(faulted_||!configured_||!interlocks||in.left< -1000||in.left>1000||in.right< -1000||in.right>1000||in.releaseChannel< -1||in.releaseChannel>3||(in.releaseChannel>=0&&(in.left||in.right))){faulted_=true;return false;}
  if(in.releaseChannel>=0){
   if(left_.moving||right_.moving||(lastChannel_>=0&&lastChannel_!=in.releaseChannel)){faulted_=true;return false;}
   out.solenoidMask=(uint8_t)(1U<<in.releaseChannel);lastChannel_=in.releaseChannel;lastOff_=false;return true;
  }
  if(!lastOff_){lastOff_=true;lastChannel_=-1;left_.offAt=right_.offAt=now;left_.blocked=right_.blocked=true;return true;}
  step(left_,in.left,now,invertLeft_,out.pwmLeft,out.dirLeft);
  step(right_,in.right,now,invertRight_,out.pwmRight,out.dirRight);
  return true;
 }
private:
 struct Motor{bool moving=false,dir=false,blocked=false;uint32_t offAt=0;};
 Motor left_,right_;bool configured_=false,invertLeft_=false,invertRight_=false,lastOff_=true,faulted_=false;int8_t lastChannel_=-1;uint16_t limit_=0,delay_=0;
 void step(Motor &m,int16_t command,uint32_t now,bool invert,uint16_t &pwm,bool &dir){
  const bool desired=(command<0)!=invert;dir=m.dir;pwm=0;
  if(!command){if(m.moving){m.moving=false;m.offAt=now;m.blocked=true;}return;}
  if(m.moving&&desired!=m.dir){m.moving=false;m.blocked=true;m.offAt=now;return;}
  if(m.blocked&&now-m.offAt<delay_)return;
  m.blocked=false;m.dir=desired;dir=desired;m.moving=true;
  const unsigned mag=command<0?(unsigned)-command:(unsigned)command;
  pwm=(uint16_t)((mag*limit_)/1000U);
 }
};
