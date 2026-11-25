#include "remote_mission_prepare_tx.h"
#include <cassert>
using namespace flytec_rtos;
using S=AsyncMissionUpload::State;
int main(){
 AsyncMissionUpload up;MissionAssembler boat;MissionPlan p{9,1,15,8,300,431000000,270000000,431010000,270010000};
 uint8_t data[8]{};assert(up.begin(p,1,100,true));
 uint32_t now=100;for(unsigned op=1;op<=6;++op){
  assert(up.due(now,true,data)&&up.queued(op,3,op,1,now));
  auto result=boat.ingest(data,now+1,true,true);
  assert(up.acknowledge(9,static_cast<MissionOpcode>(op),result,1,now+1,now+1,true));
  assert(up.complete(op,3,op,1,9,now,100));now+=2;
 }
 assert(up.prepared(now));assert(!up.handoff(9,1,now-1,now,true));
 assert(!up.handoff(8,1,now,now,true));assert(!up.handoff(9,2,now,now,true));
 assert(!up.handoff(9,1,now,now,false));assert(up.handoff(9,1,now,now,true));
 assert(up.state()==S::HandedOff&&!up.prepared(now)&&!up.due(now,true,data));
 assert(up.beginCancel(now,true));assert(up.due(now,true,data)&&data[0]==MISSION_CANCEL);
 RadioFrame f{};radio_frameInit(f,RADIO_MSG_MISSION_PREPARE,1,2,7,9,now,1);
 memcpy(f.payload,data,8);assert(scopedCancelPreparationFrame(f)&&!scopedPrepareFrame(f));
 f.payload[7]=1;assert(!scopedCancelPreparationFrame(f));f.payload[7]=0;
 f.flags=1;assert(!scopedCancelPreparationFrame(f));f.flags=0;
 assert(up.queued(7,3,7,1,now));assert(!up.beginCancel(now,true));
 auto result=boat.ingest(data,now+1,true,true);assert(result==MISSION_CANCELLED);
 assert(!up.acknowledge(9,MISSION_CANCEL,result,1,now+1,now+1,false));
 assert(up.complete(7,3,7,1,9,now,100));assert(up.state()==S::Canceling);
 assert(up.due(now+500,true,data)&&up.queued(8,3,8,1,now+500));
 result=boat.ingest(data,now+501,true,true);assert(result==MISSION_DUPLICATE);
 assert(up.acknowledge(9,MISSION_CANCEL,result,1,now+501,now+501,true));
 assert(up.state()==S::Cancelled&&up.inFlight());assert(up.complete(8,3,8,1,9,now+500,100));
 MissionPlan got{};assert(!boat.getPrepared(got,now+501));
 up.resetAfterSession();assert(up.begin(p,2,1000,true));assert(up.beginCancel(1001,true));
 assert(up.due(1001,true,data)&&up.queued(9,4,9,2,1001));
 assert(up.complete(9,4,9,2,9,1001,100));assert(!up.maintain(2801,true)&&up.state()==S::Failed);
 assert(!up.acknowledge(9,MISSION_CANCEL,MISSION_CANCELLED,2,2801,2801,true));
}
