#include "remote_async_mission_control.h"
#include "mission_controller.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
 AsyncMissionControl ctl;BoatMissionController boat;MissionPlan p{9,1,15,8,300,431000000,270000000,431010000,270010000};
 assert(boat.stage(p,100,true,true));assert(ctl.bind(9,1,90,101,101,true));
 ExecInputs in{};in.fix={431020000,270020000,1.0,0.0,true,true,true,true};
 in.primaryFresh=in.stationary=in.releaseIdle=in.releaseInterlocks=true;
 MissionControlAuthority a{true,true,true,true,true,true,true,true,true,false};
 uint32_t ticket=1;uint16_t seq=1;
 auto send=[&](MissionControlOp op,uint32_t now,MissionControlPhase phase){
  assert(ctl.request(op,now,a));MissionControlRequest req{},decoded{};uint8_t payload[8]{};
  assert(ctl.due(now,a,req)&&missionControlEncode(req,payload)&&missionControlDecode(payload,decoded));
  assert(ctl.queued(ticket,1,seq,1,now,ctl.lifetime(now)));
  MissionControlFeedback reply{},wire{};
  assert(boat.request(decoded,in,now+1,true,true,true,reply)&&reply.result==MC_ACCEPTED&&reply.phase==phase);
  assert(missionFeedbackEncode(reply,payload)&&missionFeedbackDecode(payload,wire));
  assert(ctl.observe(wire,1,now+1,now+1,true));assert(ctl.complete(ticket,1,seq,1,9,now,100));
  assert(ctl.phase()==phase);++ticket;++seq;
 };
 send(MCTRL_START,102,MC_OUTBOUND);send(MCTRL_PAUSE,104,MC_PAUSED);send(MCTRL_RESUME,106,MC_OUTBOUND);send(MCTRL_CANCEL,108,MC_ABORTED);
 assert(boat.status().phase==MC_ABORTED);
}
