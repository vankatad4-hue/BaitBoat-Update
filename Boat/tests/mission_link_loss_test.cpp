#include "../source/mission_controller.h"
#include <cassert>
#include <iostream>
static void start(BoatMissionController &c,MissionPlan p,ExecInputs &in){
 c.resetSession();in.stopActive=true;assert(c.stage(p,0,true,true));
 in.stopActive=false;MissionControlFeedback ack{};
 assert(c.request({MCTRL_START,p.transaction,1},in,1,true,true,true,ack)&&ack.result==MC_ACCEPTED);
}
int main(){
 MissionPlan p={1,0,0,15,300,0,2000,0,0};
 ExecInputs in={{0,1000,1,270,true,true,true,true},false,true,true,true,true,true};
 BoatMissionController c;start(c,p,in);
 assert(c.tick(in,2,true,true).intent==INTENT_NAV_TARGET);
 in.primaryFresh=in.backupFresh=false;
 assert(c.tick(in,100,true,false).intent==INTENT_STOP&&c.diverted());
 assert(c.tick(in,101,true,false).intent==INTENT_NAV_HOME);
 // Transport session expiry/rekey calls divertHome, not resetSession.
 assert(c.divertHome(in,true));
 assert(c.tick(in,10000,true,false).intent==INTENT_NAV_HOME);
 in.primaryFresh=true;MissionControlFeedback ack{};
 assert(c.request({MCTRL_PAUSE,1,2},in,10001,true,true,true,ack)&&ack.result==MC_REJECTED);
 assert(c.tick(in,10002,true,false).intent==INTENT_NAV_HOME);
 in.fix.lonE7=0;assert(c.tick(in,10003,true,true).intent==INTENT_STOP&&c.status().phase==MC_COMPLETE);
 // Prepared but never started: no autonomous movement.
 c.resetSession();assert(c.stage(p,0,true,true));assert(!c.divertHome(in,true));
 // Navigation failure and physical STOP always dominate RTH.
 in.fix.lonE7=1000;in.backupFresh=true;start(c,p,in);
 assert(c.divertHome(in,true));in.fix.fresh=false;
 assert(c.tick(in,5,true,false).intent==INTENT_STOP&&c.status().phase==MC_ABORTED);
 in.fix.fresh=true;start(c,p,in);assert(c.divertHome(in,true));in.stopActive=true;
 assert(c.tick(in,6,true,false).intent==INTENT_STOP&&c.status().phase==MC_ABORTED);
 // Never leave the release zone with unconfirmed solenoid completion.
 in.fix.lonE7=2000;start(c,p,in);c.tick(in,2,true,true);
 assert(c.tick(in,3,true,true).intent==INTENT_RELEASE);
 assert(!c.divertHome(in,true)&&c.status().phase==MC_ABORTED);
 std::cout<<"Detached return-home and loss safety regression passed\n";
}
