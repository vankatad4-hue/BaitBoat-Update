// R30H-26: real BoatApplicationPort with host radio + non-actuating
// physical-driver fake; inject a stuck-motor feedback signal AFTER STOP.
// The requested OFF output and the observed signal are deliberately separate.
// No ESP32 GPIO, actual output current or mechanical state is measured.
#include "boat_application_port.h"
#include <cassert>
#include <cstdint>
#include <iostream>
uint32_t fakeNow=0;
static BoatActuatorOutput requested={0,0,-1};
static bool motorFeedbackStuck=false,coilFeedbackStuck=false;
static unsigned disables=0,reads=0,applies=0;
static bool readLocal(BoatHardwareSample &s){
 ++reads;
 s={};
 s.fix={420000000,270000000,1.0,90.0,true,true,true,true};
 s.safetyAt=s.gnssAt=s.headingAt=s.batteryAt=s.speedAt=fakeNow;
 s.safetyValid=true;s.outputFeedbackValid=true;s.watchdogReady=true;
 s.physicalStopReleased=true;s.batteryHealthy=true;
 s.releaseInterlocks=true;
 s.leftMotorActive=motorFeedbackStuck||requested.left!=0;
 s.rightMotorActive=requested.right!=0;
 s.stationary=!s.leftMotorActive&&!s.rightMotorActive;
 s.releaseIdle=!coilFeedbackStuck&&requested.releaseChannel<0;
 s.voltageValid=true;s.batteryMillivolts=12000;s.gpsFixType=3;
 s.satellites=9;s.batteryPercent=75;
 return true;
}
static bool applyLocal(const BoatActuatorOutput&v,uint16_t lease){
 assert(lease==100U);
 assert(v.releaseChannel<0||(!v.left&&!v.right));
 ++applies;requested=v;return true; // driver SAYS accepted, feedback differs
}
static void disableLocal(){
 ++disables;requested={0,0,-1}; // fake OFF demand does not clear stuck sensor
}
static bool demandedOff(){
 return requested.left==0&&requested.right==0&&requested.releaseChannel<0;
}
static void command(BoatDualRadioPort&r,BoatCommandType type,uint16_t id,
                    int16_t left=0,int16_t right=0){
 BoatCommand c{};c.type=type;c.mode=MODE_MANUAL;
 c.commandId=id;c.createdAtMs=fakeNow;c.leftMotor=left;c.rightMotor=right;
 c.activate=(type>=CMD_RELEASE_HOPPER_LEFT&&type<=CMD_RELEASE_HOOK_RIGHT);
 r.queue.push_back({c,LINK_NRF_PRIMARY});
}
static void tick(BoatApplicationPort&app,uint32_t when){
 fakeNow=when;app.tick();
}
int main(){
 BoatDualRadioPort radio;BoatApplicationPort app(radio);
 assert(app.attach({readLocal,applyLocal,disableLocal},
                   {{100,120,140,160},400,3.0}));
 tick(app,100U);
 command(radio,CMD_CLEAR_STOP,1);
 tick(app,101U);
 assert(!radio.acks.empty()&&radio.acks.back().success);
 command(radio,CMD_MANUAL_MOTORS,2,300,0);
 tick(app,102U);
 assert(requested.left==300);
 // Pretend the relay/drive transistor stays energized despite disable.
 motorFeedbackStuck=true;
 command(radio,CMD_EMERGENCY_STOP,3);
 tick(app,103U);
 assert(demandedOff());
 assert(radio.phase==MC_ABORTED);
 tick(app,352U); // only 249 ms since OFF
 assert(demandedOff());
 command(radio,CMD_CLEAR_STOP,4);
 tick(app,352U);
 assert(!radio.acks.back().success);
 tick(app,353U); // exactly 250-ms persistent motor mismatch

 assert(demandedOff());
 assert(radio.phase==MC_ABORTED);
 assert(!radio.telemetry.empty());
 auto status=radio.telemetry.back();
 assert(status.kind==RADIO_PAYLOAD_STATUS);
 assert(status.stopState==2U); // UNKNOWN, not false current-zero proof
 assert((status.safetyFlags&2U)!=0U);
 const auto readsAtLatch=reads;
 const auto disablesAtLatch=disables;
 motorFeedbackStuck=false;
 ++radio.gen;
 command(radio,CMD_CLEAR_STOP,5);
 command(radio,CMD_MANUAL_MOTORS,6,300,300);
 tick(app,354U);
 assert(demandedOff()&&reads==readsAtLatch&&disables>disablesAtLatch);
 assert(radio.acks.back().id==6&&!radio.acks.back().success);
 assert(radio.telemetry.back().stopState==2U);
 command(radio,CMD_MANUAL_MOTORS,7,0,0);
 command(radio,CMD_MANUAL_MOTORS,8,350,350);
 tick(app,355U);
 assert(demandedOff()&&radio.acks.back().id==8);
 assert(!radio.acks.back().success);
 std::cout<<"R30H-26 persistent motor output after STOP, UNKNOWN feedback, permanent software fault latch PASS (host only)\n";
}
