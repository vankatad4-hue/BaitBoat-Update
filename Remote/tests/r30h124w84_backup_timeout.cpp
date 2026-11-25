#include "remote_backup_mission_control.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
 using S=BackupMissionControl::State;
 const BackupMissionAuthority safe{true,true,true,true,true,false,false};
 const uint32_t bound=BackupMissionControl::TxLifetime+BackupMissionControl::CompletionGrace;
 MissionControlRequest q{};
 BackupMissionControl timed;assert(timed.bind(9,1,4,1,true)&&timed.request(MCTRL_PAUSE,100,safe));
 assert(timed.due(100,safe,true,q)&&timed.queued(7,4,20,1,100,250));
 assert(timed.maintain(350,safe)&&timed.maintain(100+bound-1,safe));
 assert(!timed.maintain(100+bound,safe)&&timed.state()==S::Unknown&&timed.inFlight());
 assert(!timed.complete(8,4,20,1,9,100,250));
 assert(timed.complete(7,4,20,1,9,100,250));
 MissionControlFeedback late{9,MC_PAUSED,MCTRL_PAUSE,2,MC_ACCEPTED};
 assert(!timed.observe(late,1,4,100+bound+1,100+bound+1,true,safe));
 const MissionControlResult results[]={MC_ACCEPTED,MC_REJECTED};
 for(auto result:results){
  BackupMissionControl early;assert(early.bind(9,1,4,1,true)&&early.request(MCTRL_PAUSE,100,safe));
  assert(early.due(100,safe,true,q)&&early.queued(7,4,20,1,100,250));
  MissionControlFeedback f{9,MC_PAUSED,MCTRL_PAUSE,2,result};
  assert(early.observe(f,1,4,200,200,true,safe));
  const auto expected=result==MC_ACCEPTED?S::Accepted:S::Rejected;
  assert(early.maintain(100+bound,safe)&&early.state()==expected&&early.inFlight());
  assert(!early.request(MCTRL_CANCEL,100+bound,safe));
  assert(early.complete(7,4,20,1,9,100,250)&&early.state()==expected);
 }
 for(unsigned fault=0;fault<3;++fault){
  auto unsafe=safe;
  if(fault==0)unsafe.batteryOk=false;
  else if(fault==1)unsafe.safetyReturn=true;
  else unsafe.backupFresh=false;
  BackupMissionControl blocked;assert(blocked.bind(9,1,4,1,true));
  assert(!blocked.request(MCTRL_PAUSE,100,unsafe)&&!blocked.request(MCTRL_CANCEL,100,unsafe));
  assert(blocked.request(MCTRL_CANCEL,100,safe)&&blocked.due(100,safe,true,q));
  assert(blocked.queued(7,4,20,1,100,250));
  assert(!blocked.maintain(101,unsafe)&&blocked.state()==S::Unknown);
  assert(!blocked.due(101,unsafe,true,q));
  blocked.transportRevoked();assert(!blocked.inFlight()&&!blocked.request(MCTRL_CANCEL,102,unsafe));
 }
}
