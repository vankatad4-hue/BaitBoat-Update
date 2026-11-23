// R30H-25: actual application after shared 3S abrupt brownout.
// Exercises software battery latch, radio session changes and no auto restart.
#include "boat_application_port.h"
#include <cassert>
#include <cstdint>
#include <iostream>
uint32_t fakeNow=0;
static uint16_t supply=12000U;
static unsigned disableCount=0;
static BoatActuatorOutput output={0,0,-1};
static bool sampleReader(BoatHardwareSample &s){
 s={};
 s.fix={420000000,270000000,1.0,90.0,true,true,true,true};
 s.safetyAt=s.gnssAt=s.headingAt=fakeNow;
 s.batteryAt=s.speedAt=fakeNow;
 s.safetyValid=true;s.outputFeedbackValid=true;s.watchdogReady=true;
 s.physicalStopReleased=true;s.batteryHealthy=true;s.stationary=true;
 s.releaseInterlocks=true;s.releaseIdle=true;
 s.voltageValid=true;s.batteryMillivolts=supply;
 s.batteryPercent=70;s.gpsFixType=3;s.satellites=9;
 return true;
}
static bool applyOutput(const BoatActuatorOutput &v,uint16_t lease){
 assert(lease==100U);output=v;return true;
}
static void disable(){output={0,0,-1};++disableCount;}
static void issue(BoatDualRadioPort &r,BoatCommandType type,uint16_t id,int16_t pwm=0){
 BoatCommand c{};c.type=type;c.mode=MODE_MANUAL;c.commandId=id;
 c.createdAtMs=fakeNow;c.leftMotor=pwm;c.rightMotor=pwm;
 r.queue.push_back({c,LINK_NRF_PRIMARY});
}
static void tick(BoatApplicationPort &a,uint32_t t){fakeNow=t;a.tick();}
static bool stopped(){return !output.left&&!output.right&&output.releaseChannel==-1;}
int main(){
 BoatDualRadioPort radio;BoatApplicationPort app(radio);
 assert(app.attach({sampleReader,applyOutput,disable},
                  {{80,100,120,140},400,3}));
 tick(app,100);issue(radio,CMD_CLEAR_STOP,20);
 tick(app,101);assert(radio.acks.back().success);
 issue(radio,CMD_MANUAL_MOTORS,21,300);
 tick(app,102);assert(output.left==300&&output.right==300);
 supply=9800U;
 tick(app,103);
 assert(stopped()&&radio.phase==MC_ABORTED);
 const unsigned stopCount=disableCount;
 supply=12600U;
 tick(app,104);
 assert(stopped()&&disableCount>stopCount);
 radio.gen++;
 issue(radio,CMD_CLEAR_STOP,22);
 issue(radio,CMD_MANUAL_MOTORS,23,350);
 tick(app,105);
 assert(stopped()&&radio.phase==MC_ABORTED);
 // Critical battery software alert is latched until explicit power-cycle
 // and physical requalification; re-handshake alone cannot restore output.
 for(uint32_t t=106;t<160;++t)tick(app,t);
 assert(stopped()&&radio.phase==MC_ABORTED);
 assert(output.releaseChannel==-1);
 std::cout<<"R30H-25 shared 3S brownout remains STOP across radio generation and voltage recovery PASS (host only)\n";
}
