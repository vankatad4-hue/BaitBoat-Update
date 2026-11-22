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
  // Battery health depends on a real commissioned, fresh local voltage sample
  // (selected INA231 R27/R28). Neither cached data nor remote telemetry
  // may replace a locally acquired battery reading.
  contacts.batteryHealthy=sample.voltageValid&&
    (uint32_t)(now-sample.batteryAt)<=BoatBatteryMonitor::MAX_SAMPLE_AGE_MS;
  batteryAt_=sample.batteryAt;
  boatPhysicalSafetyToSample(contacts,now,sample);
  // Two independent coil-zero indications must agree before arbitration.
  if(contacts.coilCurrentZero!=coil.allCoilCurrentsZero){fault();return false;}
  const bool common=contacts.isolationTestSigned&&contacts.stopLoopClosed&&
    contacts.watchdogContactClosed&&contacts.motorContactorFeedbackClosed&&
    contacts.coilContactorFeedbackClosed&&contacts.batteryHealthy&&
    (uint32_t)(now-contacts.sampledAt)<100U;
  if(!common||(!guardActive_&&!guard_.awaitingZero()&&!contacts.coilCurrentZero)){fault();return false;}
  contacts_=contacts;coil_=coil;lastRead_=now;
  return true;
 }
 bool apply(const BoatActuatorOutput &request,uint16_t leaseMs,uint32_t now){
  if(!initialized_||faulted_||!guard_.enabled()||leaseMs==0||leaseMs>100||
     (uint32_t)(now-lastRead_)>=50U||
     !contacts_.isolationTestSigned||!contacts_.stopLoopClosed||
     !contacts_.watchdogContactClosed||!contacts_.motorContactorFeedbackClosed||
     !contacts_.coilContactorFeedbackClosed||!contacts_.batteryHealthy||
     (uint32_t)(now-batteryAt_)>BoatBatteryMonitor::MAX_SAMPLE_AGE_MS||
     (uint32_t)(now-contacts_.sampledAt)>=100U)return fail();
  if(request.left< -1000||request.left>1000||request.right< -1000||
     request.right>1000||request.releaseChannel< -1||request.releaseChannel>3)return fail();
  if(request.releaseChannel>=0){
   if(request.left||request.right||!contacts_.motorCurrentZero)return fail();
   if(!guardActive_){
    if(!contacts_.coilCurrentZero||
      !guard_.request(request.releaseChannel,now,coil_))return fail();
    guardActive_=true;channel_=request.releaseChannel;
   }else if(channel_!=request.releaseChannel)return fail();
   if(!hal_.motorWrite(0,0,leaseMs))return fail();
   const int8_t active=guard_.tick(now,coil_);
   if(!guard_.enabled()||active!=request.releaseChannel)return fail();
   if(!hal_.coilWrite(active))return fail();
   return true;
  }
  // Turn off all coil outputs BEFORE applying motor demand.
  if(!hal_.coilWrite(-1))return fail();
  if(guardActive_){
   const int8_t active=guard_.tick(now,coil_);
   if(!guard_.enabled()||active>=0)return fail(); // software requested coil off too early
   guardActive_=false;channel_=-1;
  }
  if(guard_.awaitingZero()){
   guard_.tick(now,coil_);
   if(!guard_.enabled())return fail();
   if(guard_.awaitingZero()&&(request.left||request.right))return fail();
  }
  // Never authorize propulsion on stale coil feedback or cleared fault state.
  if((request.left||request.right)&&(!contacts_.coilCurrentZero||
     !coil_.allCoilCurrentsZero||!coil_.independentPulseLimiterReady||
     (uint32_t)(now-coil_.sampledAt)>=50U||!guard_.enabled()||
     guard_.awaitingZero()))return fail();
  if(!hal_.motorWrite(request.left,request.right,leaseMs))return fail();
  return true;
 }
 void fault(){if(hal_.cutOutputs)hal_.cutOutputs();guard_.reset();faulted_=true;guardActive_=false;channel_=-1;}
 bool faulted()const{return faulted_;}
private:
 bool fail(){fault();return false;}
 BoatPhysicalHal hal_{};
 BoatSafetyContacts contacts_{};Ekm014Feedback coil_{};
 Ekm014PulseGuard guard_{};
 uint32_t lastRead_=0,batteryAt_=0;bool initialized_=false,faulted_=true,guardActive_=false;
 int8_t channel_=-1;
};
