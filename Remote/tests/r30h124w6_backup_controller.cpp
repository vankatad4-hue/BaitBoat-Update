#include "remote_backup_mission_control.h"
#include "mission_controller.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
 BoatMissionController boat;MissionPlan p{9,1,15,8,300,431000000,270000000,431010000,270010000};
 ExecInputs in{};in.fix={431020000,270020000,1.0,0.0,true,true,true,true};
 in.primaryFresh=in.stationary=in.releaseIdle=in.releaseInterlocks=true;
 MissionControlFeedback reply{};assert(boat.stage(p,100,true,true));
 assert(boat.request({MCTRL_START,9,1},in,101,true,true,true,reply)&&reply.result==MC_ACCEPTED);
 in.primaryFresh=false;in.backupFresh=true;
 assert(boat.tick(in,102,true,false).intent==INTENT_NAV_TARGET);
 BackupMissionControl ctl;BackupMissionAuthority a{true,true,true,true,true,false,false};
 assert(!ctl.bind(9,1,4,2,false)&&ctl.bind(9,1,4,2,true));
 assert(!ctl.request(MCTRL_START,103,a)&&!ctl.request(MCTRL_RESUME,103,a)&&!ctl.request(MCTRL_TEMP_OVERRIDE,103,a));
 assert(ctl.request(MCTRL_PAUSE,103,a)&&ctl.requestHighWater()==3);
 MissionControlRequest q{},decoded{};uint8_t bytes[8]{};
 assert(!ctl.due(103,a,false,q)&&ctl.due(103,a,true,q));
 assert(missionControlEncode(q,bytes)&&missionControlDecode(bytes,decoded)&&decoded.request==3);
 assert(ctl.queued(1,4,20,1,103,ctl.lifetime(103)));
 assert(boat.request(decoded,in,104,true,true,false,reply)&&reply.result==MC_ACCEPTED&&reply.phase==MC_PAUSED);
 assert(ctl.complete(1,4,20,1,9,103,250)&&ctl.state()==BackupMissionControl::State::Pending);
 assert(!ctl.observe(reply,1,4,104,104,false,a)); // local completion not ACK
 assert(!ctl.observe(reply,1,5,104,104,true,a));
 // Lost reply: same request ID after 3-second pacing; duplicate is idempotent.
 assert(!ctl.due(3102,a,true,q)&&ctl.due(3103,a,true,q)&&q.request==3);
 assert(ctl.queued(2,4,21,1,3103,ctl.lifetime(3103)));
 assert(boat.request(q,in,3104,true,true,false,reply)&&reply.result==MC_ACCEPTED&&reply.phase==MC_PAUSED);
 assert(ctl.observe(reply,1,4,3104,3104,true,a)&&ctl.complete(2,4,21,1,9,3103,250));
 assert(boat.tick(in,3105,true,false).intent==INTENT_STOP);
 assert(ctl.request(MCTRL_CANCEL,3105,a)&&!ctl.due(6102,a,true,q)&&ctl.due(6103,a,true,q)&&q.request==4);
 assert(ctl.queued(3,4,22,1,6103,ctl.lifetime(6103)));
 assert(boat.request(q,in,6104,true,true,false,reply)&&reply.result==MC_ACCEPTED&&reply.phase==MC_ABORTED);
 assert(ctl.observe(reply,1,4,6104,6104,true,a)&&ctl.complete(3,4,22,1,9,6103,250));
 // Independent actual controller safety diversion: neither ordinary mission
 // PAUSE nor CANCEL may interrupt low battery/local Home return.
 BoatMissionController returning;assert(returning.stage(p,100,true,true));in.primaryFresh=true;
 assert(returning.request({MCTRL_START,9,1},in,101,true,true,true,reply)&&reply.result==MC_ACCEPTED);
 in.batteryLow=true;assert(returning.divertHome(in,true));in.primaryFresh=false;
 assert(returning.request({MCTRL_PAUSE,9,2},in,102,true,true,false,reply)&&reply.result==MC_REJECTED&&reply.phase==MC_RETURNING);
 assert(returning.request({MCTRL_CANCEL,9,3},in,103,true,true,false,reply)&&reply.result==MC_REJECTED&&reply.phase==MC_RETURNING);
 in.batteryLow=false;in.backupFresh=false;in.autonomousRth=true;
 assert(returning.request({MCTRL_PAUSE,9,4},in,104,true,true,false,reply)&&reply.result==MC_REJECTED);
 assert(returning.request({MCTRL_CANCEL,9,5},in,105,true,true,false,reply)&&reply.result==MC_REJECTED);
 assert(returning.tick(in,106,true,false).intent==INTENT_NAV_HOME);
 returning.emergencyAbort();assert(returning.tick(in,107,true,false).intent==INTENT_STOP);
 BackupMissionControl lost;assert(lost.bind(9,1,4,4,true)&&lost.request(MCTRL_CANCEL,100,a));
 assert(lost.due(100,a,true,q)&&lost.queued(4,4,23,1,100,250));
 a.stopPending=true;assert(!lost.maintain(101,a)&&lost.state()==BackupMissionControl::State::Unknown);
 lost.transportRevoked();assert(!lost.inFlight()&&!lost.request(MCTRL_CANCEL,102,a));
}
