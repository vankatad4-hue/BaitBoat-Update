#pragma once
#include <stdint.h>
#include "boat_hardware_supervisor.h"
#include "physical_driver_commissioning_gate.h"
#include "ekm014_pulse_guard.h"
// R50 transport-neutral physical-driver coordinator. Hardware HAL callbacks
// must be wired to actual local, independently verified contacts. No fake
// feedback or default GPIO assignments. This is not commissioned hardware.
struct BoatPhysicalHal{
 bool (*sample)(BoatHardwareSample &,BoatSafetyContacts &,Ekm014Feedback &)=nullptr;
 bool (*motorWrite)(int16_t,int16_t,uint16_t)=nullptr;
 bool (*coilWrite)(int8_t)=nullptr; // -1 deenergizes all four channels
 void (*cutOutputs)()=nullptr;
};
class BoatPhysicalDriverAdapter {
public:
 bool begin(const BoatPhysicalHal &hal,const uint16_t pulseMs[4],
            bool independentHardwareCommissioned){
  if(initialized_||!hal.sample||!hal.motorWrite||!hal.coilWrite||
     !hal.cutOutputs||!independentHardwareCommissioned||
     !guard_.configure(pulseMs,true))return false;
  hal_=hal;hal_.cutOutputs();initialized_=true;faulted_=false;return true;
 }
 bool read(BoatHardwareSample &sample,uint32_t now){
  if(!initialized_||faulted_)return false;
  BoatSafetyContacts contacts{};Ekm014Feedback coil{};
  if(!hal_.sample(sample,contacts,coil)){fault();return false;}
  boatPhysicalSafetyToSample(contacts,now,sample);
  if(!boatPhysicalOutputsPermitted(contacts,now)){fault();return false;}
  contacts_=contacts;coil_=coil;lastRead_=now;
  return true;
 }
 bool apply(const BoatActuatorOutput &request,uint16_t leaseMs,uint32_t now){
  if(!initialized_||faulted_||leaseMs==0||leaseMs>100||
     (uint32_t)(now-lastRead_)>=50U||
     !boatPhysicalOutputsPermitted(contacts_,now))return fail();
  if(request.left< -1000||request.left>1000||request.right< -1000||
     request.right>1000||request.releaseChannel< -1||request.releaseChannel>3)return fail();
  if(request.releaseChannel>=0){
   if(request.left||request.right||!contacts_.motorCurrentZero)return fail();
   if(!guardActive_){
    if(!guard_.request(request.releaseChannel,now,coil_))return fail();
    guardActive_=true;channel_=request.releaseChannel;
   }else if(channel_!=request.releaseChannel)return fail();
   if(!hal_.motorWrite(0,0,leaseMs))return fail();
   const int8_t active=guard_.tick(now,coil_);
   if(active!=request.releaseChannel)return fail();
   if(!hal_.coilWrite(active))return fail();
   return true;
  }
  // Turn off all coil outputs BEFORE applying motor demand.
  if(!hal_.coilWrite(-1))return fail();
  if(guardActive_){
   const int8_t active=guard_.tick(now,coil_);
   if(active>=0)return fail(); // software requested coil off too early
   guardActive_=false;channel_=-1;
  }
  if(guard_.awaitingZero()){
   if(!guard_.tick(now,coil_) || guard_.awaitingZero()){
    // tick() returns -1 on normal idle; no motor drive while settling
    if(request.left||request.right)return fail();
   }
  }
  return hal_.motorWrite(request.left,request.right,leaseMs)||fail();
 }
 void fault(){if(hal_.cutOutputs)hal_.cutOutputs();guard_.reset();faulted_=true;guardActive_=false;channel_=-1;}
 bool faulted()const{return faulted_;}
private:
 bool fail(){fault();return false;}
 BoatPhysicalHal hal_{};
 BoatSafetyContacts contacts_{};Ekm014Feedback coil_{};
 Ekm014PulseGuard guard_{};
 uint32_t lastRead_=0;bool initialized_=false,faulted_=true,guardActive_=false;
 int8_t channel_=-1;
};
