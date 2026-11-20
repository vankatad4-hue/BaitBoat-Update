#include "../source/boat_hardware_supervisor.h"
#include <cassert>
#include <iostream>
static BoatHardwareSample sample(uint32_t t){
 return {{0,1000,1,90,true,true,true,true},t,t,t,true,true,true,true,true,true,true};
}
int main(){
 BoatActuatorSupervisor s;auto hw=sample(0);
 assert(!s.configure({{0,0,0,0},0,0}));
 assert(!s.clearStop(hw,0,true,true));
 assert(s.configure({{100,150,200,250},400,3}));
 assert(!s.clearStop(hw,100,true,true));
 assert(!s.clearStop(hw,0,false,true));
 assert(s.clearStop(hw,0,true,true));
 ExecInputs in=boatHardware_inputs(hw,0,s.stopped());in.primaryFresh=true;
 const ExecOutput stop={INTENT_STOP,-1,0};const MissionNavigationResult nav{};
 s.manual(200,-200,0,true);
 assert(s.tick(in,199,stop,nav,false,false,0,0).left==200);
 assert(s.tick(in,200,stop,nav,false,false,0,0).left==0);
 assert(s.beginRelease(1,1,0,in,300));assert(!s.beginRelease(1,2,1,in,301));
 assert(s.tick(in,399,stop,nav,true,false,0,0).releaseChannel==0);
 assert(s.tick(in,400,stop,nav,true,false,0,0).releaseChannel==-1);
 assert(!s.takeCompletion().valid);assert(s.tick(in,401,stop,nav,true,false,0,0).releaseChannel==-1);
 auto done=s.takeCompletion();assert(done.valid&&done.success&&done.token==1);
 assert(!s.beginRelease(1,1,0,in,402));
 assert(s.beginRelease(1,2,1,in,403));in.primaryFresh=false;
 assert(s.tick(in,404,stop,nav,true,false,0,0).releaseChannel==-1);
 done=s.takeCompletion();assert(done.valid&&!done.success);
 MissionNavigationResult home={true,false,15,90,0};
 assert(s.tick(in,405,{INTENT_NAV_HOME,-1,0},home,true,false,0,0).left==400);
 in.stopActive=true;assert(s.tick(in,406,{INTENT_NAV_HOME,-1,0},home,true,false,0,0).left==0&&s.stopped());
 hw=sample(500);assert(s.clearStop(hw,500,true,true));
 assert(boatHardware_inputs(hw,600,false).stopActive);
 auto fresh=boatHardware_inputs(sample(700),700,false);assert(!fresh.stopActive);
 std::cout<<"Hardware sample validity, propulsion watchdog and release executor passed\n";
}
