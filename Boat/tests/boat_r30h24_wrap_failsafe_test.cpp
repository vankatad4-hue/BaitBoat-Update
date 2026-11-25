// R30H-24 host-only monotonic wrap, stale safety and output lease matrix.
// Uses real Boat sample, power, actuator, watchdog and status classes.
// No GPIO, RF, battery instrumentation, physical STOP measurement or flashing.
#include "../source/boat_hardware_supervisor.h"
#include "../source/boat_hardware_status.h"
#include "../source/boat_radio_safety.h"
#include "../source/boat_shared_supply_fault_guard.h"
#include <cassert>
#include <cstdint>
#include <limits>
#include <iostream>

static const ExecOutput STOP={INTENT_STOP,-1,0};
static const MissionNavigationResult HOME={true,false,12.0,270.0,0.0};
static bool off(BoatActuatorOutput v){return v.left==0&&v.right==0&&v.releaseChannel==-1;}
static bool moves(BoatActuatorOutput v){return (v.left>0||v.right>0)&&v.releaseChannel==-1;}
static BoatHardwareSample sample(uint32_t at){
 BoatHardwareSample s{};
 s.fix={0,1000,1.0,270.0,true,true,true,true};
 s.safetyAt=at;s.gnssAt=at;s.headingAt=at;s.batteryAt=at;s.speedAt=at;
 s.safetyValid=true;s.outputFeedbackValid=true;s.watchdogReady=true;
 s.physicalStopReleased=true;s.batteryHealthy=true;
 s.stationary=true;s.releaseInterlocks=true;s.releaseIdle=true;
 s.batteryBand=BOAT_BATTERY_GREEN;s.batteryPercent=75;
 s.batteryMillivolts=12000;s.voltageValid=true;s.gpsFixType=3;s.satellites=9;
 return s;
}
static ExecInputs ready(const BoatHardwareSample &s,uint32_t at,bool primary=true,bool backup=false){
 ExecInputs in=boatHardware_inputs(s,at,false);
 in.primaryFresh=primary;in.backupFresh=backup;return in;
}
static BoatActuatorSupervisor armed(const BoatHardwareSample &s,uint32_t at){
 BoatActuatorSupervisor a;
 assert(a.configure({{120,140,160,180},400,3.0}));
 assert(a.clearStop(s,at,true,true));
 return a;
}
static void wrapHardwareAge(){
 const uint32_t base=UINT32_MAX-100U;
 auto s=sample(base);
 assert(!boatHardware_inputs(s,base+99U,false).stopActive);
 assert(boatHardware_inputs(s,base+100U,false).stopActive);
 // Local timestamps remain separate; freshness comparison works across wrap.
 s=sample(base);
 auto in=boatHardware_inputs(s,base+299U,false);
 assert(in.fix.headingValid&&in.fix.fresh);
 in=boatHardware_inputs(s,base+300U,false);
 assert(!in.fix.headingValid); // heading boundary exactly 300 ms
 in=boatHardware_inputs(s,base+1999U,false);
 assert(in.fix.fresh);
 in=boatHardware_inputs(s,base+2000U,false);
 assert(!in.fix.fresh); // GPS boundary exactly 2000 ms
 assert(!boatRadioLossRthDue(true,true,true,true,true,base,base+9999U));
 assert(boatRadioLossRthDue(true,true,true,true,true,base,base+10000U));
 assert(!boatRadioLossRthDue(false,true,true,true,true,base,base+10000U));
}
static void navBoundaryStaleCachedResult(){
 const uint32_t start=UINT32_MAX-80U;
 auto s=sample(start);
 auto a=armed(s,start);
 auto in=ready(s,start);in.autonomousRth=true;
 assert(moves(a.tick(in,start,{INTENT_NAV_HOME,-1,0},HOME,true,false,0,0,start)));
 // A cached navigation result is deliberately still valid. The real
 // physical-sample ages, not that cache, must gate the final motor request.
 s.safetyAt=start+300U;s.headingAt=start; // heading age exactly 300 ms
 in=ready(s,start+300U,false,false);in.autonomousRth=true;
 assert(!in.fix.headingValid);
 assert(off(a.tick(in,start+300U,{INTENT_NAV_HOME,-1,0},HOME,true,false,0,0,s.safetyAt)));
 s.gnssAt=start+1U;s.headingAt=start+2100U;s.safetyAt=start+2100U;
 in=ready(s,start+2100U,false,false);in.autonomousRth=true;
 assert(!in.fix.fresh);
 assert(off(a.tick(in,start+2100U,{INTENT_NAV_HOME,-1,0},HOME,true,false,0,0,s.safetyAt)));
 s=sample(start+2200U);
 in=ready(s,start+2200U,false,false);in.autonomousRth=true;
 in.fix.geofenceValid=false;
 assert(off(a.tick(in,start+2200U,{INTENT_NAV_HOME,-1,0},HOME,true,false,0,0,s.safetyAt)));
 in=ready(s,start+2200U,false,false);in.autonomousRth=true;
 in.fix.accuracyM=std::numeric_limits<double>::quiet_NaN();
 assert(off(a.tick(in,start+2200U,{INTENT_NAV_HOME,-1,0},HOME,true,false,0,0,s.safetyAt)));
 in=ready(s,start+2200U,false,false);in.autonomousRth=true;
 in.fix.headingDeg=360.0;
 assert(off(a.tick(in,start+2200U,{INTENT_NAV_HOME,-1,0},HOME,true,false,0,0,s.safetyAt)));
 in=ready(s,start+2200U,false,false);in.autonomousRth=true;
 assert(moves(a.tick(in,start+2200U,{INTENT_NAV_HOME,-1,0},HOME,true,false,0,0,s.safetyAt)));
 // RTH never authorizes an outbound target or release without either radio.
 assert(off(a.tick(in,start+2201U,{INTENT_NAV_TARGET,-1,0},HOME,true,false,0,0,s.safetyAt)));
 assert(!a.beginRelease(82,1,0,in,start+2201U));
}
static void manualLeaseWrapAndBackup(){
 const uint32_t base=UINT32_MAX-100U;
 auto s=sample(base);
 auto a=armed(s,base);
 auto in=ready(s,base);
 a.manual(250,-100,base,true);
 assert(a.tick(in,base+199U,STOP,{},false,false,0,0,base).left==250);
 assert(off(a.tick(in,base+200U,STOP,{},false,false,0,0,base)));
 // Recompute local sample ages for the independent radio-loss/recovery case.
 s=sample(base+202U);in=ready(s,base+202U,false,true);
 assert(off(a.tick(in,base+202U,STOP,{},false,false,0,0,s.safetyAt)));
 in.primaryFresh=true;in.backupFresh=false;
 a.manual(250,-100,base+203U,true); // cannot replay non-neutral sticks
 assert(off(a.tick(in,base+204U,STOP,{},false,false,0,0,s.safetyAt)));
 a.manual(0,0,base+205U,true);
 a.manual(250,-100,base+206U,true);
 assert(a.tick(in,base+207U,STOP,{},false,false,0,0,s.safetyAt).left==250);
}
static void coilWrapAndTimeout(){
 const uint32_t base=UINT32_MAX-60U;
 auto s=sample(base);
 auto a=armed(s,base);
 auto in=ready(s,base);
 assert(a.beginRelease(90,1,0,in,base));
 assert(a.tick(in,base+119U,{INTENT_RELEASE,0,1},{},true,false,0,0,base).releaseChannel==0);
 assert(off(a.tick(in,base+120U,STOP,{},true,false,0,0,base+120U)));
 assert(a.releaseBusy());
 assert(off(a.tick(in,base+121U,STOP,{},true,false,0,0,base+120U)));
 assert(!a.takeCompletion().valid); // same-sample cannot certify OFF
 assert(off(a.tick(in,base+122U,STOP,{},true,false,0,0,base+122U)));
 const auto done=a.takeCompletion();
 assert(done.valid&&done.success&&done.token==1&&!a.stopped());
 assert(!a.beginRelease(90,1,0,in,base+123U)); // no replay of same token
 assert(a.beginRelease(90,2,1,in,base+124U));
 assert(a.tick(in,base+125U,{INTENT_RELEASE,1,2},{},true,false,0,0,base+125U).releaseChannel==1);
 // Backup-only interruption immediately kills an active pulse and latches STOP.
 in.primaryFresh=false;in.backupFresh=true;
 assert(off(a.tick(in,base+126U,STOP,{},true,false,0,0,base+126U)));
 assert(a.stopped());
 auto cancelled=a.takeCompletion();assert(cancelled.valid&&!cancelled.success);
 in.primaryFresh=true;
 assert(!a.beginRelease(90,2,1,in,base+127U));
 // Missing post-off feedback times out after 500 ms, even after rollover.
 const uint32_t second=UINT32_MAX-15U;
 auto s2=sample(second);
 auto b=armed(s2,second);
 auto bIn=ready(s2,second);
 assert(b.beginRelease(92,1,0,bIn,second));
 assert(off(b.tick(bIn,second+120U,STOP,{},true,false,0,0,second+120U)));
 bIn.releaseIdle=false; // no independently verified idle reading
 assert(off(b.tick(bIn,second+619U,STOP,{},true,false,0,0,second+120U)));
 assert(!b.stopped()&&b.releaseBusy());
 assert(off(b.tick(bIn,second+620U,STOP,{},true,false,0,0,second+120U)));
 assert(b.stopped());
 const auto timeout=b.takeCompletion();assert(timeout.valid&&!timeout.success);
}
static void lowBatteryRepeatedWrapAndCritical(){
 const uint32_t base=UINT32_MAX-1700U;
 BoatBatteryMonitor battery;
 auto s=battery.update(true,10700U,base);
 assert(s.valid&&!s.rthRequested&&!s.critical);
 assert(!battery.update(true,10700U,base+2999U).rthRequested);
 s=battery.update(true,10700U,base+3000U);
 assert(s.rthRequested&&!s.critical);
 for(uint32_t i=1;i<=200U;++i){
  s=battery.update(true,10700U,base+3000U+50U*i);
  assert(s.rthRequested&&!s.critical);
 }
 s=battery.update(true,12500U,base+14000U);
 assert(s.rthRequested); // alert cannot silently clear on recovery
 const auto stopped= battery.update(true,9800U,base+14001U);
 assert(stopped.critical&&!stopped.rthRequested);
 assert(boat_r30b::inspectBoatBattery(stopped,true,true,9800U,base+14001U,
       base+14001U)==boat_r30b::EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 const auto rebound=battery.update(true,12600U,base+14002U);
 assert(rebound.critical); // no automatic rearm
 auto noConversion=battery.update(false,0,base+14003U);
 assert(!noConversion.valid);
 assert(boat_r30b::inspectBoatBattery(noConversion,true,false,0,base+14003U,
       base+14003U)==boat_r30b::EnergyEvidence::STOP_INVALID_OR_CRITICAL);
}
static void statusDoesNotInventCurrentZero(){
 const uint32_t at=UINT32_MAX-12U;
 auto s=sample(at);s.physicalStopReleased=false;
 auto stopped=boatHardware_status(s,at+1U,true,MC_ABORTED,false,true);
 assert(stopped.stopState==1);
 s.leftMotorActive=true;
 assert(boatHardware_status(s,at+1U,true,MC_ABORTED,false,true).stopState==2U);
 s.leftMotorActive=false;s.outputFeedbackValid=false;
 assert(boatHardware_status(s,at+1U,true,MC_ABORTED,false,true).stopState==2U);
 s.outputFeedbackValid=true;s.safetyAt=at;
 assert(boatHardware_status(s,at+100U,true,MC_ABORTED,false,true).stopState==2U);
}
int main(){
 wrapHardwareAge();navBoundaryStaleCachedResult();
 manualLeaseWrapAndBackup();coilWrapAndTimeout();
 lowBatteryRepeatedWrapAndCritical();statusDoesNotInventCurrentZero();
 std::cout<<"R30H-24 rollover, local navigation sample, radio/solenoid timeouts, repeated battery alarms PASS (host-only)\n";
}
