#include "boat_application_port.h"
#include <cassert>
#include <iostream>
uint32_t fakeNow=0;
static BoatActuatorOutput output={0,0,-1};
static bool available=true,idle=true,holdStamp=false,physical=true,applyOK=true;
static uint32_t stamp=0;
static unsigned disables=0;
static bool readSample(BoatHardwareSample &s){
 if(!available)return false;
 if(!holdStamp)stamp=fakeNow;
 s={};
 s.fix={422000000,275000000,1.25,90,true,true,true,true};
 s.safetyAt=stamp;s.gnssAt=s.headingAt=fakeNow;
 s.safetyValid=true;s.physicalStopReleased=physical;s.batteryHealthy=s.stationary=true;
 s.releaseInterlocks=true;s.releaseIdle=idle;s.watchdogReady=s.outputFeedbackValid=true;
 s.leftMotorActive=output.left!=0;s.rightMotorActive=output.right!=0;
 s.batteryPercent=70;s.gpsFixType=3;s.satellites=8;s.batteryMillivolts=12000;
 s.batteryAt=s.speedAt=fakeNow;s.voltageValid=s.speedValid=true;
 s.batteryBand=BOAT_BATTERY_GREEN;
 return true;
}
static bool apply(const BoatActuatorOutput &o,uint16_t lease){
 assert(lease==100);assert(o.releaseChannel<0||(!o.left&&!o.right));
 if(!applyOK)return false;
 output=o;return true;
}
static void disable(){++disables;output={0,0,-1};}
static void request(BoatDualRadioPort &r,BoatCommandType type,unsigned id,ManagedLink link=LINK_NRF_PRIMARY,int16_t l=0,int16_t rr=0){
 BoatCommand c{};c.type=type;c.mode=type==CMD_SET_MODE?MODE_RETURN_HOME:MODE_MANUAL;c.commandId=id;c.createdAtMs=fakeNow;c.activate=type>=CMD_RELEASE_HOPPER_LEFT;c.leftMotor=l;c.rightMotor=rr;r.queue.push_back({c,link});
}
static void at(BoatApplicationPort &a,uint32_t t){fakeNow=t;a.tick();}
int main(){
 BoatDualRadioPort r;BoatApplicationPort a(r);
 request(r,CMD_RELEASE_HOPPER_LEFT,1);at(a,1);assert(output.releaseChannel==-1&&r.acks.empty());
 assert(a.attach({readSample,apply,disable},{{40,45,50,55},400,3}));
 at(a,2);assert(r.input&&r.gate&&r.gate());
 // A backup RTH request has no authority while the local STOP is latched.
 r.phase=MC_OUTBOUND;r.tx=42;r.returnAllowed=true;r.backup=true;
 request(r,CMD_SET_MODE,190,LINK_LORA_BACKUP);at(a,3);
 assert(!r.acks.back().success && r.acks.back().reply==LINK_LORA_BACKUP);
 assert(r.returnRequests==0);
 r.phase=MC_IDLE;r.tx=0;r.returnAllowed=false;r.backup=false;
 request(r,CMD_CLEAR_STOP,2);at(a,4);assert(r.acks.back().success);
 // Missing active/validated mission must be rejected; no optimistic EXECUTED ACK.
 request(r,CMD_SET_MODE,191,LINK_NRF_PRIMARY);at(a,5);
 assert(!r.acks.back().success && r.returnRequests==1);
 r.phase=MC_OUTBOUND;r.tx=42;r.returnAllowed=true;r.backup=true;
 r.primary=false;
 request(r,CMD_SET_MODE,192,LINK_LORA_BACKUP);at(a,6);
 assert(r.acks.back().success && r.acks.back().reply==LINK_LORA_BACKUP);
 assert(r.returnRequests==2 && r.detached);
 r.primary=true;r.backup=false;r.returnAllowed=false;
 r.detached=false;r.phase=MC_IDLE;r.tx=0;
 // R78: the production application bridge passes the local, validated
 // failsafe-Home authorization to the actuator supervisor. With both radios
 // absent, only the Home intent may produce dummy motor output; coils stay OFF.
 r.phase=MC_RETURNING;r.tx=42;r.intent={INTENT_NAV_HOME,-1,0};
 r.nav={true,false,15,90,0};r.primary=false;r.backup=false;r.failsafeRth=true;
 at(a,7);assert(output.left>0&&output.right>0&&output.releaseChannel==-1);
 r.failsafeRth=false;r.primary=true;r.phase=MC_IDLE;r.tx=0;r.intent={INTENT_STOP,-1,0};
 at(a,8);assert(output.left==0&&output.right==0&&output.releaseChannel==-1);
 unsigned id=3;
 for(int c=0;c<4;++c){
  fakeNow+=10;request(r,(BoatCommandType)(CMD_RELEASE_HOPPER_LEFT+c),id++);
  const uint32_t start=fakeNow;at(a,start);assert(output.releaseChannel==c);
  request(r,CMD_RELEASE_HOPPER_LEFT,id++);at(a,start+1);assert(!r.acks.back().success&&output.releaseChannel==c);
  idle=false;at(a,start+60);assert(output.releaseChannel==-1);
  const size_t count=r.acks.size();idle=true;holdStamp=true;at(a,start+61);assert(r.acks.size()==count);
  holdStamp=false;at(a,start+62);assert(r.acks.back().success&&output.releaseChannel==-1);
 }
 request(r,CMD_RELEASE_HOOK_LEFT,id++,LINK_LORA_BACKUP);at(a,fakeNow+1);assert(!r.acks.back().success);
 request(r,CMD_MANUAL_MOTORS,id++,LINK_NRF_PRIMARY,100,100);at(a,fakeNow+1);assert(output.left==0);
 request(r,CMD_MANUAL_MOTORS,id++);at(a,fakeNow+1);
 request(r,CMD_MANUAL_MOTORS,id++,LINK_NRF_PRIMARY,100,100);at(a,fakeNow+1);assert(output.left==100);
 at(a,fakeNow+201);assert(output.left==0);
 at(a,fakeNow+1);request(r,CMD_RELEASE_HOOK_RIGHT,id++);at(a,fakeNow+1);assert(output.releaseChannel==3);
 r.primary=false;at(a,fakeNow+1);assert(output.releaseChannel==-1&&!r.acks.back().success);
 r.primary=true;request(r,CMD_CLEAR_STOP,id++);at(a,fakeNow+1);assert(r.acks.back().success);
 request(r,CMD_RELEASE_HOPPER_LEFT,id++);at(a,fakeNow+1);assert(output.releaseChannel==0);
 ++r.gen;at(a,fakeNow+1);assert(output.releaseChannel==-1);
 at(a,1000);bool coords=false;for(auto &p:r.telemetry)if(p.kind==RADIO_PAYLOAD_GNSS_LATITUDE)coords=true;assert(coords);
 available=false;at(a,1001);assert(output.left==0&&output.releaseChannel==-1&&disables>0);
 std::cout<<"Production application bridge with simulated drivers passed\n";
}
