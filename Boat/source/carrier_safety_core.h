#pragma once
#include "carrier_link.h"
namespace carrier {
struct Inputs{uint8_t flags,coilIdleMask;};
struct Demand{int16_t left,right;int8_t coil;bool permit,pet;};
constexpr uint32_t LEASE_MS=40; // independent RP2040 receive lease; electrical cutoff separately required
class SafetyCore{
public:
 bool configure(bool commissioned,const uint16_t (&pulse)[4]){
  configured_=commissioned;
  for(unsigned i=0;i<4;++i){limits_[i]=pulse[i];if(!pulse[i]||pulse[i]>=4000)configured_=false;}
  lock(0);return configured_;
 }
 void lock(uint8_t fault){state_=fault?FAULT:LOCKED;fault_=fault;cmd_={DISARM,0,0,0,-1};seq_=0;last_=0;coil_=-1;coilAt_=0;awaitIdle_=false;fresh_=false;neutral_=false;pet_=false;}
 bool receive(const Command &c,uint32_t now,Inputs in){
  if(!valid(c)){lock(1);return false;}
  if(c.op==DISARM){lock(0);return true;}
  if(c.op==SYNC_OFF){
   // No re-sync while gate energized; hardware local arm is required afresh.
   if((in.flags&GATE_ON)||state_==ARMED){lock(2);return false;}
   lock(0);seq_=c.seq;last_=now;fresh_=true;state_=READY;neutral_=true;pet_=true;return true;
  }
  if(!fresh_||c.seq<=seq_||now-last_>=LEASE_MS){lock(3);return false;}
  seq_=c.seq;last_=now;
  const bool off=!c.left&&!c.right&&c.coil==-1;
  if(state_!=ARMED){
   if(!off){lock(4);return false;}
   neutral_=true;pet_=true;return true;
  }
  if(!safe(in)||!(in.flags&GATE_ON)){lock(5);return false;}
  if(c.coil>=0){
   if(!configured_||!neutral_||awaitIdle_||!(in.flags&LEFT_IDLE)||!(in.flags&RIGHT_IDLE)){lock(6);return false;}
   if(coil_<0){if(cmd_.left||cmd_.right||!allIdle(in)){lock(6);return false;}coil_=c.coil;coilAt_=now;}
   else if(coil_!=c.coil){lock(6);return false;}
  }else if(coil_>=0){coil_=-1;awaitIdle_=true;offAt_=now;neutral_=false;}
  if((c.left||c.right)&&(awaitIdle_||!allIdle(in)||!neutral_)){lock(6);return false;}
  if(off&&!awaitIdle_)neutral_=true;
  cmd_=c;pet_=true;return true;
 }
 Demand tick(uint32_t now,Inputs in){
  Demand d={0,0,-1,false,false};
  if(!fresh_||now-last_>=LEASE_MS){if(fresh_)lock(3);return d;}
  // Watchdog must receive pulses before the local hardware arm edge.
  // WATCHDOG_OK denotes separately measured WDO/ENOUT readiness.
  const uint8_t base=STOP_OK|INTERLOCK_OK|FEEDBACK_OK;
  if((in.flags&base)!=base){lock(5);return d;}
  if(state_==READY){
   if(!allIdle(in)||!(in.flags&LEFT_IDLE)||!(in.flags&RIGHT_IDLE)){lock(6);return d;}
   d.pet=pet_;pet_=false;
   // Permit alone never powers the actuators. A separate local arm edge
   // clocks the hardware latch; GATE_ON must be actual feedback.
   d.permit=configured_&&safe(in)&&neutral_;
   if((in.flags&GATE_ON)&&!d.permit){lock(5);return {0,0,-1,false,false};}
   if(d.permit&&(in.flags&GATE_ON)){state_=ARMED;cmd_={OUTPUT,seq_,0,0,-1};}
   return d;
  }
  if(state_!=ARMED||!safe(in)||!(in.flags&GATE_ON)){lock(5);return d;}
  if(coil_>=0&&now-coilAt_>=limits_[coil_]){lock(7);return d;}
  if(awaitIdle_){
   if(now-offAt_>=500U){lock(8);return d;}
   // Input acquired on current loop, and at least one later millisecond.
   if(now-offAt_>0U&&allIdle(in))awaitIdle_=false;
  }
  d.permit=true;d.pet=pet_;pet_=false;
  if(!awaitIdle_){d.left=cmd_.left;d.right=cmd_.right;d.coil=coil_;}
  return d;
 }
 Status status(uint16_t sample,Inputs in)const{return {state_,seq_,sample,in.flags,in.coilIdleMask,fault_};}
private:
 static bool safe(Inputs in){const uint8_t m=STOP_OK|WATCHDOG_OK|INTERLOCK_OK|FEEDBACK_OK;return (in.flags&m)==m;}
 static bool allIdle(Inputs in){return (in.flags&COILS_IDLE)&&in.coilIdleMask==15;}
 State state_=LOCKED;uint8_t fault_=0;bool configured_=false,fresh_=false,neutral_=false,pet_=false,awaitIdle_=false;
 uint32_t seq_=0,last_=0,coilAt_=0,offAt_=0;int8_t coil_=-1;uint16_t limits_[4]={};Command cmd_={DISARM,0,0,0,-1};
};
}
