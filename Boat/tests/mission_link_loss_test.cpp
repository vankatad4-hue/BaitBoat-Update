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

 // Total loss stops propulsion but preserves the autonomous mission phase.
 in.primaryFresh=false;in.backupFresh=false;
 assert(c.tick(in,100,true,false).intent==INTENT_STOP);
 assert(!c.diverted()&&c.status().phase==MC_OUTBOUND);
 assert(c.tick(in,101,true,false).intent==INTENT_STOP);
 // Either authenticated link restores the held mission; backup can supervise
 // autonomous navigation but cannot authorize a release.
 in.backupFresh=true;
 assert(c.tick(in,102,true,false).intent==INTENT_NAV_TARGET);
 in.primaryFresh=true;in.backupFresh=false;
 assert(c.tick(in,103,true,false).intent==INTENT_NAV_TARGET);

 // At the target, backup-only keeps the boat stopped until primary returns.
 in.fix.lonE7=2000;
 assert(c.tick(in,104,true,true).intent==INTENT_STOP);
 assert(c.status().phase==MC_RELEASING);
 in.primaryFresh=false;in.backupFresh=true;
 assert(c.tick(in,105,true,false).intent==INTENT_STOP);
 assert(c.status().phase==MC_RELEASING);
 in.primaryFresh=true;
 assert(c.tick(in,106,true,true).intent==INTENT_RELEASE);

 // If primary is lost during an actual release wait, abort rather than
 // repeating a potentially completed hopper/hook action after reconnect.
 in.primaryFresh=false;in.backupFresh=true;
 assert(c.tick(in,107,true,false).intent==INTENT_STOP);
 assert(c.status().phase==MC_ABORTED);

 // Explicit Home remains available as a user-requested mission action.
 in.primaryFresh=true;in.backupFresh=true;in.fix.lonE7=1000;
 start(c,p,in);
 assert(c.tick(in,200,true,true).intent==INTENT_NAV_TARGET);
 assert(c.divertHome(in,true));
 assert(c.tick(in,201,true,false).intent==INTENT_NAV_HOME);
 in.fix.lonE7=0;
 assert(c.tick(in,202,true,false).intent==INTENT_STOP&&c.status().phase==MC_COMPLETE);

 // Prepared but never started remains stopped; link recovery does not start it.
 c.resetSession();assert(c.stage(p,0,true,true));
 in.primaryFresh=false;in.backupFresh=false;
 assert(c.tick(in,5,true,true).intent==INTENT_STOP);
 in.primaryFresh=true;
 assert(c.tick(in,10001,true,true).intent==INTENT_STOP);
 assert(c.status().phase==MC_ABORTED);

 // Physical STOP and failed navigation still dominate link recovery.
 in.fix.lonE7=1000;start(c,p,in);
 assert(c.tick(in,2,true,true).intent==INTENT_NAV_TARGET);
 in.stopActive=true;
 assert(c.tick(in,3,true,true).intent==INTENT_STOP&&c.status().phase==MC_ABORTED);
 std::cout<<"Radio-loss stop, mission hold/resume and release non-replay tests passed\n";
}
