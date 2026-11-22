#include "../source/mission_controller.h"
#include <cassert>
#include <iostream>
int main(){
 MissionPlan p={1,0,0,1,300,0,1000,0,0};
 ExecInputs in={{0,0,1,90,true,true,true,true},true,true,true,true,true,true,false};
 BoatMissionController c;MissionControlFeedback ack{};uint8_t bytes[8];MissionControlRequest req{MCTRL_START,1,1},decoded{};
 assert(missionControlEncode(req,bytes)&&missionControlDecode(bytes,decoded)&&decoded.request==1);
 bytes[7]=1;assert(!missionControlDecode(bytes,decoded));
 assert(!c.stage(p,0,true,false));assert(c.stage(p,0,true,true));
 assert(c.request(req,in,1,true,true,true,ack)&&ack.result==MC_REJECTED);
 in.stopActive=false;req.request=2;
 assert(!c.request(req,in,2,false,true,true,ack));
 assert(c.request(req,in,2,true,false,true,ack)&&ack.result==MC_REJECTED);
 req.request=3;assert(c.request(req,in,3,true,true,true,ack)&&ack.result==MC_ACCEPTED);
 assert(c.request(req,in,4,true,true,true,ack)&&ack.result==MC_ACCEPTED);
 assert(c.tick(in,5,true,true).intent==INTENT_NAV_TARGET);
 req={MCTRL_TEMP_OVERRIDE,1,4};assert(c.request(req,in,6,true,true,false,ack)&&ack.phase==MC_OVERRIDE_NEUTRAL);
 assert(c.tick(in,7,true,false).intent==INTENT_STOP&&!c.temporaryOverrideConfirmed());
 c.tick(in,8,true,true);assert(c.temporaryOverrideConfirmed());
 req={MCTRL_RESUME,1,5};assert(c.request(req,in,9,true,true,true,ack)&&ack.phase==MC_OUTBOUND);
 assert(c.tick(in,10,true,true).intent==INTENT_NAV_TARGET);
 req={MCTRL_SWITCH_MANUAL,1,6};assert(c.request(req,in,11,true,true,true,ack)&&ack.phase==MC_MANUAL_REQUESTED);
 assert(c.tick(in,12,true,true).intent==INTENT_STOP);
 in.stationary=false;assert(!c.confirmManual(in));in.stationary=true;assert(c.confirmManual(in)&&c.status().phase==MC_IDLE);
 assert(missionFeedbackEncode(ack,bytes));MissionControlFeedback read{};assert(missionFeedbackDecode(bytes,read)&&read.request==6);
 bytes[7]=1;assert(!missionFeedbackDecode(bytes,read));
 c.resetSession();in.stopActive=true;assert(c.stage(p,0,true,true));in.stopActive=false;
 req={MCTRL_START,1,1};assert(c.request(req,in,10000,true,true,true,ack)&&ack.result==MC_REJECTED);
 c.tick(in,10001,true,true);assert(c.status().phase==MC_ABORTED);
 c.resetSession();in.stopActive=true;assert(c.stage(p,0,true,true));in.stopActive=false;
 req={MCTRL_START,1,1};assert(c.request(req,in,1,true,true,true,ack)&&ack.result==MC_ACCEPTED);
 c.emergencyAbort();assert(c.request(req,in,2,true,true,true,ack)&&ack.result==MC_REJECTED&&ack.phase==MC_ABORTED);
 assert(c.lastRequestId()==1&&c.tick(in,3,true,true).intent==INTENT_STOP);

 // A lost primary link cancels temporary joystick override and returns to
 // the previously active autonomous phase; no old stick value is replayed.
 BoatMissionController linkLoss;
 MissionPlan lp={7,0,0,1,300,0,1000,0,0};
 ExecInputs lin={{0,0,1,90,true,true,true,true},true,true,true,true,true,true,false};
 assert(linkLoss.stage(lp,0,true,true));lin.stopActive=false;
 MissionControlFeedback linkAck{};
 assert(linkLoss.request({MCTRL_START,7,1},lin,1,true,true,true,linkAck)&&linkAck.result==MC_ACCEPTED);
 assert(linkLoss.tick(lin,2,true,true).intent==INTENT_NAV_TARGET);
 assert(linkLoss.request({MCTRL_TEMP_OVERRIDE,7,2},lin,3,true,true,false,linkAck)&&linkAck.phase==MC_OVERRIDE_NEUTRAL);
 linkLoss.tick(lin,4,true,true);assert(linkLoss.temporaryOverrideConfirmed());
 assert(linkLoss.cancelOverrideForLinkLoss());
 assert(linkLoss.status().phase==MC_OUTBOUND);
 lin.primaryFresh=false;lin.backupFresh=false;
 assert(linkLoss.tick(lin,5,true,false).intent==INTENT_STOP);
 lin.backupFresh=true;
 assert(linkLoss.tick(lin,6,true,false).intent==INTENT_NAV_TARGET);
 std::cout<<"Mission command and override supervisor simulation passed\n";
}
