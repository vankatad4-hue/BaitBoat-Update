#pragma once
#include <stdint.h>
// Software interlock for a factory-built EKM014 interface. It does not
// replace a separately powered one-hot pulse limiter or DC power cutoff.
// Inputs are fresh local feedback, never remote acknowledgements.
struct Ekm014Feedback{
 bool coilRailFeedback=false;
 bool motorCurrentZero=false;
 bool allCoilCurrentsZero=false;
 bool independentPulseLimiterReady=false;
 uint32_t sampledAt=0;
};
class Ekm014PulseGuard{
public:
 bool configure(const uint16_t widths[4],bool independentlyCommissioned){
  enabled_=false;reset();
  if(!independentlyCommissioned||!widths)return false;
  for(unsigned i=0;i<4;++i){
   if(widths[i]<1||widths[i]>500)return false;
   width_[i]=widths[i];
  }
  enabled_=true;return true;
 }
 bool request(int8_t channel,uint32_t now,const Ekm014Feedback &f){
  if(!enabled_||active_>=0||awaitZero_||channel<0||channel>=4||!f.allCoilCurrentsZero||!ready(f,now))return false;
  active_=channel;started_=now;return true;
 }
 // Returns the ONLY allowed active channel; -1 means all outputs OFF.
 int8_t tick(uint32_t now,const Ekm014Feedback &f){
  if(!enabled_||!ready(f,now)){reset();enabled_=false;return -1;}
  if(active_>=0&&(uint32_t)(now-started_)>=width_[active_]){
   active_=-1;awaitZero_=true;offAt_=now;
  }
  if(awaitZero_){
   if((uint32_t)(now-offAt_)>500U){reset();enabled_=false;}
   else if((int32_t)(f.sampledAt-offAt_)>0&&f.allCoilCurrentsZero)awaitZero_=false;
  }
  return active_;
 }
 void reset(){active_=-1;awaitZero_=false;started_=offAt_=0;}
 bool ready(const Ekm014Feedback &f,uint32_t now)const{
  return f.coilRailFeedback&&f.motorCurrentZero&&f.independentPulseLimiterReady&&
   (uint32_t)(now-f.sampledAt)<50U;
 }
 bool awaitingZero()const{return awaitZero_;}
 bool enabled()const{return enabled_;}
private:
 bool enabled_=false,awaitZero_=false;
 int8_t active_=-1;uint16_t width_[4]={};
 uint32_t started_=0,offAt_=0;
};
