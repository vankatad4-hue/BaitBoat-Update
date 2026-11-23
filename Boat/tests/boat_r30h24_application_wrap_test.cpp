// R30H-24: actual BoatApplicationPort with host driver and radio stub.
// Shared-3S repeated low-battery abort spanning millis() wrap; no GPIO/RF.
#include "boat_application_port.h"
#include <cassert>
#include <cstdint>
#include <iostream>
uint32_t fakeNow=0;
static uint16_t supplyMv=12000;
static unsigned disableCount=0,applyCount=0;
static BoatActuatorOutput output={0,0,-1};
static bool measured=true,batteryVoltageValid=true,releaseIdle=true;
static bool readSample(BoatHardwareSample &s){
 if(!measured)return false;
 s={};
 s.fix={422000000,275000000,1.25,90.0,true,true,true,true};
 s.safetyAt=s.gnssAt=s.headingAt=fakeNow;
 s.safetyValid=true;s.physicalStopReleased=true;s.batteryHealthy=true;
 s.stationary=true;s.releaseInterlocks=true;s.releaseIdle=releaseIdle;
 s.watchdogReady=true;s.outputFeedbackValid=true;
 s.batteryPercent=75;s.gpsFixType=3;s.satellites=8;
 s.batteryMillivolts=supplyMv;s.batteryAt=fakeNow;
 s.voltageValid=batteryVoltageValid;
 return true;
}
static bool apply(const BoatActuatorOutput&o,uint16_t lease){
 assert(lease==100);
 assert(o.releaseChannel==-1||(!o.left&&!o.right));
 output=o;++applyCount;return true;
}
static void disable(){output={0,0,-1};++disableCount;}
static void tick(BoatApplicationPort&a,uint32_t t){fakeNow=t;a.tick();}
static bool off(){return !output.left&&!output.right&&output.releaseChannel==-1;}
int main(){
 const uint32_t base=UINT32_MAX-1900U;
 BoatDualRadioPort radio;BoatApplicationPort app(radio);
 assert(app.attach({readSample,apply,disable},{{120,140,160,180},400,3}));
 BoatCommand clear{};clear.type=CMD_CLEAR_STOP;clear.commandId=70;
 clear.createdAtMs=base;
 radio.queue.push_back({clear,LINK_NRF_PRIMARY});
 tick(app,base);
 assert(radio.acks.size()==1U&&radio.acks.back().success);
 assert(off());
 radio.phase=MC_OUTBOUND;radio.tx=74;radio.returnAllowed=true;
 radio.intent={INTENT_NAV_TARGET,-1,0};
 radio.nav={true,false,15,90,0};
 supplyMv=10700;
 // Execute real battery filtering and repeated app ticks across wrap.
 // This is a counter simulation; no physically measured 3S voltage exists.
 for(uint32_t elapsed=50U;elapsed<=6000U;elapsed+=50U){
   tick(app,base+elapsed);
   assert(output.releaseChannel==-1);
 }
 assert(radio.lowBatteryReturnRequests==1U);
 assert(radio.phase==MC_RETURNING&&radio.detached&&radio.failsafeRth);
 assert(output.left>0||output.right>0);
 assert(applyCount>0U);
 // Repeated red readings do NOT schedule a second Home request.
 const auto requests=radio.lowBatteryReturnRequests;
 for(uint32_t elapsed=6050U;elapsed<=8000U;elapsed+=50U){
    tick(app,base+elapsed);
    assert(output.releaseChannel==-1);
 }
 assert(radio.lowBatteryReturnRequests==requests);
 assert(!off());
 // Explicit 9.8V collapse bypasses averaging and immediately revokes Home.
 supplyMv=9800U;
 tick(app,base+8050U);
 assert(off());
 assert(radio.phase==MC_ABORTED);
 assert(disableCount>=2U);
 const unsigned afterFault=disableCount;
 tick(app,base+8100U);
 assert(off()&&disableCount>afterFault);
 // Critical battery alert remains latched even on one recovered reading.
 supplyMv=12600U;
 tick(app,base+8150U);
 assert(off()&&radio.phase==MC_ABORTED);
 std::cout<<"R30H-24 application repeated low 3S alerts across millis wrap, single Home request and immediate critical STOP PASS (host-only)\n";
}
