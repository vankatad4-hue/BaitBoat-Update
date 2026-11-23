// R30H-25: production BoatApplicationPort through replaceable host-only
// physical driver and host radio stub. No actual GPIO, radio, motor or coil.
#include "boat_application_port.h"
#include <cassert>
#include <cstdint>
#include <iostream>
uint32_t fakeNow=0;
static bool readOK=true,applyOK=true;
static unsigned readCount=0,applyCount=0,disableCount=0;
static BoatActuatorOutput output={0,0,-1};
static bool sampleReader(BoatHardwareSample &s){
 ++readCount;
 if(!readOK)return false;
 s={};
 s.fix={420000000,270000000,1.0,90.0,true,true,true,true};
 s.safetyAt=s.gnssAt=s.headingAt=fakeNow;
 s.batteryAt=s.speedAt=fakeNow;s.voltageValid=true;
 s.batteryMillivolts=12000;
 s.safetyValid=true;s.outputFeedbackValid=true;
 s.watchdogReady=true;s.physicalStopReleased=true;
 s.batteryHealthy=true;s.stationary=true;
 s.releaseInterlocks=true;s.releaseIdle=true;
 s.leftMotorActive=output.left!=0;s.rightMotorActive=output.right!=0;
 s.batteryPercent=80;s.gpsFixType=3;s.satellites=9;
 return true;
}
static bool applyOutput(const BoatActuatorOutput &v,uint16_t leaseMs){
 assert(leaseMs==100U);
 assert(v.releaseChannel<0 || (v.left==0&&v.right==0));
 ++applyCount;
 if(!applyOK)return false;
 output=v;return true;
}
static void disable(){output={0,0,-1};++disableCount;}
static bool stopped(){return output.left==0&&output.right==0&&output.releaseChannel==-1;}
static void command(BoatDualRadioPort &r,BoatCommandType type,uint16_t id,int16_t left=0,
                    int16_t right=0,ManagedLink link=LINK_NRF_PRIMARY){
 BoatCommand c{};c.type=type;c.mode=MODE_MANUAL;c.commandId=id;
 c.createdAtMs=fakeNow;c.leftMotor=left;c.rightMotor=right;
 r.queue.push_back({c,link});
}
static void tick(BoatApplicationPort &app,uint32_t t){fakeNow=t;app.tick();}
int main(){
 BoatDualRadioPort radio;
 BoatApplicationPort app(radio);
 assert(app.attach({sampleReader,applyOutput,disable},
                   {{80,100,120,140},400,3}));
 assert(disableCount==1&&stopped());
 tick(app,100U);
 command(radio,CMD_CLEAR_STOP,1);
 tick(app,101U);
 assert(!radio.acks.empty()&&radio.acks.back().success);
 command(radio,CMD_MANUAL_MOTORS,2,250,-120);
 tick(app,102U);
 assert(output.left==250&&output.right==-120);
 const unsigned firstApplies=applyCount;

 // Re-handshake (new generation) is *not* a hardware STOP, but it
 // revokes the old motor lease and requires a NEW neutral command.
 radio.gen++;
 tick(app,103U);
 assert(stopped()&&applyCount>firstApplies);
 command(radio,CMD_MANUAL_MOTORS,3,250,-120);
 tick(app,104U);
 assert(stopped()); // non-neutral cannot replay after reconnect
 command(radio,CMD_MANUAL_MOTORS,4,0,0);
 tick(app,105U);
 assert(stopped());
 command(radio,CMD_MANUAL_MOTORS,5,-200,150);
 tick(app,106U);
 assert(output.left==-200&&output.right==150);

 // A driver/sensor read failure forces STOP; once fresh local read
 // returns, clearing the STOP still requires primary authorization and
 // a neutral observation. Recovery is NOT automatic.
 readOK=false;
 tick(app,107U);
 assert(stopped()&&radio.phase==MC_ABORTED);
 const unsigned disabledOnRead=disableCount;
 readOK=true;
 tick(app,108U); // cannot auto resume
 assert(stopped()&&disableCount==disabledOnRead);
 command(radio,CMD_CLEAR_STOP,6,0,0,LINK_LORA_BACKUP);
 tick(app,109U);
 assert(!radio.acks.back().success);
 command(radio,CMD_CLEAR_STOP,7);
 tick(app,110U);
 assert(radio.acks.back().success);
 command(radio,CMD_MANUAL_MOTORS,8,250,250);
 tick(app,111U);
 assert(stopped()); // fault also requires neutral
 command(radio,CMD_MANUAL_MOTORS,9,0,0);
 tick(app,112U);
 command(radio,CMD_MANUAL_MOTORS,10,250,250);
 tick(app,113U);
 assert(output.left==250&&output.right==250);

 // If the actual physical driver's apply() fails, output state is
 // uncertain: latch a permanent safe-disabled application state.
 applyOK=false;
 tick(app,114U);
 assert(stopped());
 assert(radio.phase==MC_ABORTED);
 const unsigned afterFailure=disableCount;
 const unsigned readsBeforeLatch=readCount;
 applyOK=true; // transport seems recovered; still NO rearm
 radio.gen++;
 command(radio,CMD_CLEAR_STOP,11);
 command(radio,CMD_MANUAL_MOTORS,12,300,300);
 tick(app,115U);
 assert(stopped()&&disableCount>afterFailure);
 assert(readCount==readsBeforeLatch); // no sensor-driven auto-rearm
 assert(radio.acks.back().id==12&&!radio.acks.back().success);
 command(radio,CMD_MANUAL_MOTORS,13,0,0);
 command(radio,CMD_MANUAL_MOTORS,14,300,300);
 tick(app,116U);
 assert(stopped());
 assert(radio.acks.back().id==14&&!radio.acks.back().success);
 assert(!radio.telemetry.empty());
 assert(radio.telemetry.back().kind==RADIO_PAYLOAD_STATUS);
 assert(radio.telemetry.back().stopState==2U); // unknown physical STOP
 assert(radio.phase==MC_ABORTED);
 std::cout<<"R30H-25 production Boat application driver fail-latch, sensor STOP and neutral-session recovery PASS (host only)\n";
}
