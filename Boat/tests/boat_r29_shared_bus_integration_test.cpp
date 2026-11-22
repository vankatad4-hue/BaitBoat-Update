#include "../source/boat_r29_shared_bus_contract.h"
#include "../source/boat_radio_safety.h"
#include "../source/boat_battery_policy.h"
#include "../source/boat_physical_driver_adapter.h"
#include "../source/ina231_acquisition_r28.h"
#include <cassert>
#include <iostream>
#include <limits>
using namespace boat_r29;
struct BusStub : boat_r28::RegisterBus {
 bool fail=false, ready=true;
 uint16_t raw=10080U, configured=0;
 bool write16(uint8_t a,uint8_t r,uint16_t v) override{
  if(fail||a!=0x40||r!=0)return false;
  configured=v;return true;
 }
 bool read16(uint8_t a,uint8_t r,uint16_t &v) override{
  if(fail||a!=0x40)return false;
  if(r==0){v=configured;return true;}
  if(r==6){v=ready?8U:0U;return true;}
  if(r==2){v=raw;return true;}
  return false;
 }
};
static BoatHardwareSample sample(uint32_t now) {
 BoatHardwareSample s{};
 s.fix={420000000,270000000,1.0,0.0,true,true,true,true};
 s.safetyAt=now;s.gnssAt=now;s.headingAt=now;
 s.safetyValid=true;s.physicalStopReleased=true;
 s.watchdogReady=true;s.outputFeedbackValid=true;
 s.batteryHealthy=true;s.stationary=true;
 s.releaseInterlocks=true;s.releaseIdle=true;
 s.batteryAt=now;s.batteryMillivolts=12600;s.voltageValid=true;
 s.batteryBand=BOAT_BATTERY_GREEN;
 return s;
}
static void zero(const BoatActuatorOutput &o){
 assert(o.left==0 && o.right==0 && o.releaseChannel==-1);
}
static BoatActuatorSupervisor armedFixture(const BoatHardwareSample &s,
                                           uint32_t now){
 BoatActuatorSupervisor sup;
 const BoatActuatorConfig c={{100,100,100,100},400,2.0};
 assert(sup.configure(c));
 assert(sup.clearStop(s,now,true,true));
 return sup; // copy of test fixture only; never a physical driver
}
int main(){
 static_assert(SHARED_3S_PACK,"Common 3S battery selected");
 static_assert(!SEPARATE_MOTOR_SOLENOID_POWER,"No separate high-current domains");
 static_assert(!SHARED_BUS_PHYSICALLY_ACCEPTED,"No installed high-current wiring");
 assert(exclusiveActuatorRequest({400,-200,-1}));
 assert(exclusiveActuatorRequest({0,0,2}));
 assert(!exclusiveActuatorRequest({400,0,2}));
 assert(!exclusiveActuatorRequest({0,0,4}));
 assert(!exclusiveActuatorRequest({1001,0,-1}));
 // Synthetic currents test a budget bound only; nothing was measured.
 assert(syntheticCurrentWithinBmsNameplate({8,7,0,1}));
 assert(syntheticCurrentWithinBmsNameplate({0,0,6,1}));
 assert(!syntheticCurrentWithinBmsNameplate({11,10,0,1}));
 assert(!syntheticCurrentWithinBmsNameplate({3,0,6,1}));
 assert(!syntheticCurrentWithinBmsNameplate({0,0,6,15}));
 assert(!syntheticCurrentWithinBmsNameplate({std::numeric_limits<double>::quiet_NaN(),0,0,0}));
 // Real physical driver NEVER begins: independent output hardware is absent.
 BoatPhysicalDriverAdapter driver;
 BoatPhysicalHal noHardware{};
 const uint16_t pulses[4]={100,100,100,100};
 assert(!driver.begin(noHardware,pulses,false));
 const uint32_t now=1000U;
 auto s=sample(now);
 BoatActuatorSupervisor sup=armedFixture(s,now);
 auto in=boatHardware_inputs(s,now,false);
 in.primaryFresh=true;
 BoatRadioWatchdog link;
 assert(link.update(true,false)==BOAT_RADIO_WATCHDOG_RESTORED);
 sup.manual(330,350,now,true);
 const ExecOutput idle={INTENT_STOP,-1,0};
 const MissionNavigationResult nav={true,false,35.0,90.0,0.0};
 auto output=sup.tick(in,now+1,idle,nav,false,false,0,0,now);
 assert(output.left==330&&output.right==350&&output.releaseChannel==-1);
 assert(exclusiveActuatorRequest(output));
 // Both RF paths die: immediate STOP commands, never continue old sticks.
 assert(link.update(false,false)==BOAT_RADIO_WATCHDOG_LOST);
 in.primaryFresh=false;in.backupFresh=false;
 output=sup.tick(in,now+2,idle,nav,false,false,0,0,now);
 zero(output);
 assert(!boatRadioLossRthDue(true,true,true,true,true,now,10999U));
 assert(boatRadioLossRthDue(true,true,true,true,true,now,11000U));
 assert(!boatRadioLossRthDue(true,true,false,true,true,now,11000U));
 assert(!boatRadioLossRthDue(true,true,true,false,true,now,11000U));
 // Backup link may be alive but cannot revive a stale primary manual lease.
 assert(link.update(false,true)==BOAT_RADIO_WATCHDOG_RESTORED);
 in.backupFresh=true;
 output=sup.tick(in,now+3,idle,nav,false,false,0,0,now);
 zero(output);
 // Local validated Home ONLY after 10s: expired local safety samples
 // must NOT be reused even when Home/navigation would otherwise be valid.
 assert(link.update(false,false)==BOAT_RADIO_WATCHDOG_LOST);
 const ExecOutput goHome={INTENT_NAV_HOME,-1,0};
 auto expired=boatHardware_inputs(s,11000U,false);
 expired.primaryFresh=false;expired.backupFresh=false;
 expired.autonomousRth=true;
 BoatActuatorSupervisor staleSup=armedFixture(s,now);
 zero(staleSup.tick(expired,11000U,goHome,nav,true,false,0,0,now));
 assert(staleSup.stopped());
 // A newer local safety sample is required for simulated RTH.
 s=sample(11000U);
 in=boatHardware_inputs(s,11000U,false);
 in.primaryFresh=false;in.backupFresh=false;in.autonomousRth=true;
 output=sup.tick(in,11000U,goHome,nav,true,false,0,0,11000U);
 assert(output.releaseChannel==-1&&output.left>0&&output.right>0);
 assert(exclusiveActuatorRequest(output));
 // Unsafe navigation/fault blocks that simulated route; no release replay.
 const MissionNavigationResult badNav={false,false,0,0,0};
 zero(sup.tick(in,11001U,goHome,badNav,true,false,0,0,now));
 // A low battery must suppress outbound even if the target intent remains.
 in.batteryLow=true;
 zero(sup.tick(in,11002U,{INTENT_NAV_TARGET,-1,0},nav,true,false,0,0,now));
 // Independent critical battery monitor wins over a Home request.
 BoatBatteryMonitor battery;
 auto b=battery.update(true,10800U,5000U);
 assert(b.valid&&b.band==BOAT_BATTERY_RED&&!b.rthRequested);
 b=battery.update(true,10800U,8000U);
 assert(b.rthRequested&&!b.critical);
 b=battery.update(true,9900U,8001U);
 assert(b.critical&&b.band==BOAT_BATTERY_CRITICAL);
 in.fix.batteryHealthy=false;
 zero(sup.tick(in,11003U,goHome,nav,true,false,0,0,now));
 assert(sup.stopped());
 b=battery.update(false,0,8002U);
 assert(!b.valid && b.critical);
 // INA231 integration: real state machine, simulated conversion and NACK.
 BusStub io;boat_r28::VoltageSampler adc;
 assert(!adc.begin(false,0x40));
 assert(adc.poll(io,0)==boat_r28::PollResult::Disabled);
 assert(adc.begin(true,0x40)); // host fixture, NOT signed physical evidence
 assert(adc.poll(io,200)==boat_r28::PollResult::Waiting);
 assert(adc.poll(io,203)==boat_r28::PollResult::Fresh);
 auto reading=adc.snapshot(203);
 assert(reading.valid&&reading.millivolts==12600);
 io.fail=true;
 assert(adc.poll(io,400)==boat_r28::PollResult::Failed);
 reading=adc.snapshot(401);
 assert(!reading.valid&&reading.millivolts==0);
 BoatBatteryMonitor adcBattery;
 auto invalid=adcBattery.update(reading.valid,reading.millivolts,401);
 assert(!invalid.valid&&invalid.band==BOAT_BATTERY_UNKNOWN);
 std::cout<<"R29 common 3S pack, radio battery mission supervisor and INA231 fault integration PASS (HOST ONLY)\n";
}
