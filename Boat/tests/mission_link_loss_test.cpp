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
 ExecInputs in={{0,1000,1,270,true,true,true,true},false,true,true,true,true,true,false,false};
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


 // Complete radio loss during a pending release intent also aborts it;
 // reconnect must never replay that one-shot action.
 BoatMissionController c2;
 in.primaryFresh=true;in.backupFresh=true;in.fix.lonE7=1000;
 start(c2,p,in);
 assert(c2.tick(in,300,true,true).intent==INTENT_NAV_TARGET);
 in.fix.lonE7=2000;
 assert(c2.tick(in,301,true,true).intent==INTENT_STOP); // arrived at target
 assert(c2.tick(in,302,true,true).intent==INTENT_RELEASE);
 in.primaryFresh=false;in.backupFresh=false;
 assert(c2.tick(in,303,true,false).intent==INTENT_STOP);
 assert(c2.status().phase==MC_ABORTED);
 in.primaryFresh=true;
 assert(c2.tick(in,304,true,true).intent==INTENT_STOP);
 assert(c2.status().phase==MC_ABORTED);

 // Explicit Home remains available as a user-requested mission action.
 in.primaryFresh=true;in.backupFresh=true;in.fix.lonE7=1000;
 start(c,p,in);
 assert(c.tick(in,200,true,true).intent==INTENT_NAV_TARGET);
 assert(c.divertHome(in,true));
 assert(c.tick(in,201,true,false).intent==INTENT_NAV_HOME);
 in.fix.lonE7=0;
 assert(c.tick(in,202,true,false).intent==INTENT_STOP&&c.status().phase==MC_COMPLETE);

 // R78: after the 10-second gate, only the locally authorized Home intent
 // may navigate without either radio. Before authorization the same outage
 // continues to hold all motor and release requests OFF.
 BoatMissionController c3;
 ExecInputs rthIn={{0,1000,1,270,true,true,true,true},false,true,true,true,true,true,false,false};
 start(c3,p,rthIn);
 assert(c3.tick(rthIn,400,true,true).intent==INTENT_NAV_TARGET);
 rthIn.primaryFresh=false;rthIn.backupFresh=false;
 assert(c3.tick(rthIn,401,true,false).intent==INTENT_STOP);
 assert(!c3.diverted());
 assert(c3.divertHome(rthIn,true));
 rthIn.autonomousRth=true;
 assert(c3.tick(rthIn,402,true,false).intent==INTENT_NAV_HOME);
 rthIn.fix.lonE7=0;
 assert(c3.tick(rthIn,403,true,false).intent==INTENT_STOP);
 assert(c3.status().phase==MC_COMPLETE);

 // Once RTH has started, radio recovery does not resume the old outbound
 // mission. Manual takeover needs primary radio, neutral sticks and a
 // stationary/idle boat.
 BoatMissionController c4;
 rthIn={{0,1000,1,270,true,true,true,true},false,true,true,true,true,true,false,false};
 start(c4,p,rthIn);
 assert(c4.tick(rthIn,500,true,true).intent==INTENT_NAV_TARGET);
 assert(c4.divertHome(rthIn,true));
 rthIn.autonomousRth=true;rthIn.stationary=true;
 MissionControlFeedback takeover{};
 assert(c4.request({MCTRL_SWITCH_MANUAL,p.transaction,2},rthIn,501,true,true,true,takeover));
 assert(takeover.result==MC_ACCEPTED&&takeover.phase==MC_MANUAL_REQUESTED);
 assert(c4.confirmManual(rthIn)&&c4.status().phase==MC_IDLE);

 // Invalid Home navigation cannot be used to authorize detached RTH.
 BoatMissionController c5;
 rthIn={{0,1000,1,270,true,true,true,true},false,true,true,true,true,true,false,false};
 start(c5,p,rthIn);
 rthIn.fix.headingValid=false;
 assert(!c5.divertHome(rthIn,true));
 assert(c5.status().phase==MC_ABORTED);

 // R79: low battery holds outbound motion, then a validated Home diversion
 // cancels the remaining target/release sequence.
 BoatMissionController c6;
 ExecInputs lowIn={{0,1000,1,270,true,true,true,true},false,true,true,true,true,true,false,false};
 start(c6,p,lowIn);
 assert(c6.tick(lowIn,600,true,true).intent==INTENT_NAV_TARGET);
 lowIn.batteryLow=true;
 assert(c6.tick(lowIn,601,true,true).intent==INTENT_STOP);
 assert(c6.status().phase==MC_OUTBOUND);
 assert(c6.canDivertHome()&&c6.divertHome(lowIn,true));
 lowIn.autonomousRth=true;
 assert(c6.tick(lowIn,602,true,false).intent==INTENT_NAV_HOME);
 lowIn.fix.batteryHealthy=false;
 assert(c6.tick(lowIn,603,true,false).intent==INTENT_STOP);

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
