#include "../source/boat_hardware_status.h"
#include <cassert>
#include <iostream>
int main(){
 BoatHardwareSample s{};
 s.fix={0,1000,1,90,true,true,true,true};
 s.safetyValid=s.physicalStopReleased=s.batteryHealthy=s.stationary=true;
 s.releaseInterlocks=s.releaseIdle=s.watchdogReady=s.outputFeedbackValid=true;
 s.batteryPercent=70;s.gpsFixType=3;s.satellites=8;s.batteryMillivolts=12000;
 s.voltageValid=s.speedValid=true;s.batteryBand=BOAT_BATTERY_GREEN;
 auto p=boatHardware_status(s,0,true,MC_IDLE,false,true);
 assert(p.stopState==1&&p.boatMode==0&&!(p.safetyFlags&1));
 p=boatHardware_status(s,0,false,MC_IDLE,false,true);assert(p.stopState==0&&(p.safetyFlags&1));
 p=boatHardware_status(s,0,false,MC_RETURNING,true,true);assert(p.boatMode==2&&(p.safetyFlags&64)&&!(p.safetyFlags&1));
 s.leftMotorActive=true;p=boatHardware_status(s,0,true,MC_IDLE,false,true);assert(p.stopState==2&&(p.safetyFlags&2));
 s.leftMotorActive=false;p=boatHardware_status(s,100,true,MC_IDLE,false,true);assert(p.stopState==2&&p.boatMode==3);
 s.outputFeedbackValid=false;p=boatHardware_status(s,0,false,MC_IDLE,false,true);assert(p.stopState==2&&!(p.safetyFlags&1));
 std::cout<<"Hardware status confirmation gates passed\n";
}
