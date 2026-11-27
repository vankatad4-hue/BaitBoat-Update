#include "boat_application_port.h"
#include <cassert>
#include <iostream>
uint32_t fakeNow=0;
static BoatActuatorOutput output={0,0,-1};
static bool readSample(BoatHardwareSample &s){
 // Set only fields used by this fixture so optional sonar/weather additions
 // to BoatHardwareSample remain safely value-initialized without positional drift.
 s={};
 s.fix={422000000,275000000,1.25,90,true,true,true,true};
 s.safetyAt=s.gnssAt=s.headingAt=fakeNow;
 s.safetyValid=s.physicalStopReleased=s.batteryHealthy=s.stationary=true;
 s.releaseInterlocks=s.releaseIdle=s.watchdogReady=s.outputFeedbackValid=true;
 s.batteryPercent=70;s.gpsFixType=3;s.satellites=8;
 s.batteryMillivolts=10700;s.speedCmPerSecond=0;
 s.batteryAt=s.speedAt=fakeNow;s.voltageValid=s.speedValid=true;
 s.batteryBand=BOAT_BATTERY_UNKNOWN;s.batteryRthRequested=false;
 return true;
}
static bool apply(const BoatActuatorOutput &o,uint16_t lease){assert(lease==100);output=o;return true;}
static void disable(){output={0,0,-1};}
int main(){
 BoatDualRadioPort radio;BoatApplicationPort app(radio);
 radio.phase=MC_OUTBOUND;radio.tx=42;radio.returnAllowed=true;
 radio.intent={INTENT_NAV_TARGET,-1,0};radio.nav={true,false,10,0,0};
 assert(app.attach({readSample,apply,disable},{{40,45,50,55},400,3}));
 BoatCommand clear{};clear.type=CMD_CLEAR_STOP;clear.commandId=1;clear.createdAtMs=0;
 radio.queue.push_back({clear,LINK_NRF_PRIMARY});
 app.tick();assert(radio.lowBatteryReturnRequests==0);
 // Red battery holds outbound propulsion during the 3-second confirmation.
 for(uint32_t t=50;t<=3050;t+=50){fakeNow=t;app.tick();}
 assert(radio.lowBatteryReturnRequests==1);
 assert(radio.phase==MC_RETURNING&&radio.detached&&radio.failsafeRth);
 fakeNow+=50;app.tick();
 assert(output.left>0&&output.right>0&&output.releaseChannel==-1);
 std::cout<<"Low battery aborts outbound mission and selects Home-only navigation; no release output\n";
}
