#include "remote_async_mission_control.h"
#include <cassert>
using namespace flytec_rtos;
using S=AsyncMissionControl::State;
int main(){
 AsyncMissionControl ctl;MissionControlAuthority a{};MissionControlRequest req{};uint8_t p[8]{};
 assert(!ctl.bind(9,1,100,110,110,false));assert(!ctl.bind(9,1,100,110,210,true));
 assert(ctl.bind(9,1,100,110,110,true));assert(!ctl.request(MCTRL_START,111,a));
 a={true,true,true,true,true,true,true,true,true,false};a.neutralLease=false;
 assert(!ctl.request(MCTRL_START,111,a));a.neutralLease=true;
 assert(!ctl.request(MCTRL_PAUSE,111,a));assert(ctl.request(MCTRL_START,111,a));
 assert(ctl.due(111,a,req)&&ctl.due(112,a,req)&&ctl.reservations()==0);
 assert(req.transaction==9&&req.request==1&&req.op==MCTRL_START&&missionControlEncode(req,p));
 MissionControlRequest decoded{};assert(missionControlDecode(p,decoded)&&decoded.request==1);
 MissionControlFeedback f{9,MC_OUTBOUND,MCTRL_START,1,MC_ACCEPTED};
 assert(!ctl.observe(f,1,112,112,true));
 assert(ctl.queued(1,4,1,1,113,100));assert(!ctl.due(114,a,req));
 assert(!ctl.complete(1,3,1,1,9,113,100));assert(!ctl.complete(1,4,1,1,9,113,99));
 assert(!ctl.observe(f,1,112,114,true));assert(!ctl.observe(f,2,114,114,true));assert(!ctl.observe(f,1,114,114,false));
 auto wrong=f;wrong.request=2;assert(!ctl.observe(wrong,1,114,114,true));wrong=f;wrong.transaction=8;assert(!ctl.observe(wrong,1,114,114,true));
 assert(missionFeedbackEncode(f,p));MissionControlFeedback decodedF{};assert(missionFeedbackDecode(p,decodedF));
 assert(ctl.observe(decodedF,1,114,114,true)&&ctl.state()==S::Accepted);
 assert(!ctl.request(MCTRL_PAUSE,114,a));assert(ctl.complete(1,4,1,1,9,113,100));
 assert(ctl.request(MCTRL_PAUSE,115,a)&&ctl.due(115,a,req)&&req.request==2);
 assert(ctl.queued(2,4,2,1,115,100)&&ctl.complete(2,4,2,1,9,115,100));
 assert(ctl.observe({9,MC_PAUSED,0,0,MC_STATUS},1,116,116,true)&&ctl.state()==S::Pending);
 assert(!ctl.due(514,a,req));assert(ctl.due(515,a,req)&&req.request==2);
 assert(ctl.queued(3,4,3,1,515,100));assert(ctl.complete(3,4,3,1,9,515,100));
 assert(ctl.observe({9,MC_PAUSED,MCTRL_PAUSE,2,MC_ACCEPTED},1,516,516,true));
 a.neutralLocal=false;assert(!ctl.request(MCTRL_RESUME,517,a));a.neutralLocal=true;
 assert(ctl.request(MCTRL_RESUME,517,a)&&ctl.due(517,a,req)&&req.request==3);
 assert(ctl.queued(4,4,4,1,517,100));a.primaryFresh=false;
 assert(!ctl.maintain(518,a)&&ctl.inFlight());assert(!ctl.request(MCTRL_CANCEL,518,a));
 ctl.transportRevoked();assert(!ctl.complete(4,4,4,1,9,517,100));a.primaryFresh=true;
 assert(ctl.observe({9,MC_PAUSED,0,0,MC_STATUS},1,519,519,true)&&ctl.state()==S::Unknown);
 assert(!ctl.request(MCTRL_RESUME,519,a));a.failsafe=true;assert(!ctl.request(MCTRL_CANCEL,519,a));a.failsafe=false;
 assert(ctl.request(MCTRL_CANCEL,519,a)&&ctl.due(519,a,req)&&req.request==4);
 assert(ctl.queued(5,5,5,1,519,100)&&ctl.complete(5,5,5,1,9,519,100));
 assert(ctl.observe({9,MC_ABORTED,MCTRL_CANCEL,4,MC_ACCEPTED},1,520,520,true)&&ctl.state()==S::Accepted);
 assert(!ctl.request(MCTRL_START,521,a));
 ctl.resetAfterSession();assert(ctl.bind(1,2,100,8990,8990,true));
 assert(ctl.request(MCTRL_START,8991,a)&&ctl.lifetime(9099)==1&&ctl.lifetime(9100)==0);
 assert(ctl.queued(6,6,1,2,9099,1));assert(!ctl.observe({1,MC_OUTBOUND,MCTRL_START,1,MC_ACCEPTED},2,9100,9100,true));
 assert(!ctl.maintain(9100,a));ctl.transportRevoked();
 ctl.resetAfterSession();assert(ctl.bind(2,3,UINT32_MAX-20,UINT32_MAX-10,UINT32_MAX-10,true));
 assert(ctl.request(MCTRL_CANCEL,UINT32_MAX-5,a));assert(ctl.due(1,a,req));assert(ctl.queued(7,7,1,3,1,100));
 assert(ctl.complete(7,7,1,3,2,1,100));assert(!ctl.observe({2,MC_ABORTED,MCTRL_CANCEL,1,MC_ACCEPTED},3,1794,1794,true));
 assert(!ctl.maintain(1794,a));
 ctl.resetAfterSession();assert(ctl.bind(3,4,1,2,2,true));assert(ctl.request(MCTRL_CANCEL,3,a));
 for(unsigned i=0;i<3;++i){uint32_t at=3+i*400;assert(ctl.due(at,a,req)&&req.request==1);assert(ctl.queued(8+i,8,i+1,4,at,100));assert(ctl.complete(8+i,8,i+1,4,3,at,100));}
 assert(!ctl.observe({3,MC_ABORTED,MCTRL_CANCEL,1,MC_ACCEPTED},4,1203,1203,true));
 assert(!ctl.due(1203,a,req)&&ctl.state()==S::Unknown);
}
