#include "remote_async_mission_upload.h"
#include <cassert>
using namespace flytec_rtos;
using S=AsyncMissionUpload::State;
int main(){
 AsyncMissionUpload up;MissionAssembler boat;MissionPlan p{1,2,15,9,300,431000000,270000000,431010000,270010000};
 uint8_t data[8]{};assert(!up.begin(p,1,100,false));assert(up.begin(p,1,100,true));
 assert(up.due(100,true,data)&&up.due(101,true,data)&&up.reservations()==0);
 assert(!up.acknowledge(1,MISSION_BEGIN,MISSION_PARTIAL,1,101,101,true));
 assert(up.queued(1,4,1,1,102));assert(!up.due(103,true,data));
 auto result=boat.ingest(data,103,true,true);assert(result==MISSION_PARTIAL);
 assert(!up.acknowledge(1,MISSION_BEGIN,result,1,101,103,true));
 assert(!up.acknowledge(1,MISSION_BEGIN,result,1,103,103,false));
 assert(!up.acknowledge(2,MISSION_BEGIN,result,1,103,103,true));
 assert(!up.acknowledge(1,MISSION_BEGIN,result,2,103,103,true));
 assert(up.acknowledge(1,MISSION_BEGIN,result,1,103,103,true));
 assert(!up.due(104,true,data)); // peer ACK can precede local completion
 assert(!up.complete(1,3,1,1,1,102,100));
 assert(!up.complete(1,4,1,1,1,103,100));
 assert(up.complete(1,4,1,1,1,102,100));
 uint32_t now=105,ticket=2;uint16_t seq=2;
 for(unsigned op=MISSION_TARGET_LAT;op<=MISSION_COMMIT;++op){
  assert(up.due(now,true,data)&&data[0]==op);
  assert(up.queued(ticket,4,seq,1,now));
  result=boat.ingest(data,now+1,true,true);
  assert(up.complete(ticket,4,seq,1,1,now,100));
  assert(!up.acknowledge(1,static_cast<MissionOpcode>(op-1),result,1,now+1,now+1,true));
  assert(up.acknowledge(1,static_cast<MissionOpcode>(op),result,1,now+1,now+1,true));
  now+=2;++ticket;++seq;
 }
 MissionPlan prepared{};assert(up.prepared(now)&&boat.getPrepared(prepared,now));
 assert(missionPlan_digest(prepared)==missionPlan_digest(p));
 assert(!up.maintain(9100,true)&&!up.prepared(9100));
 assert(!up.begin(p,1,9200,true));p.transaction=2;assert(up.begin(p,1,9200,true));
 for(unsigned i=0;i<3;++i){const uint32_t at=9200+i*500;
  assert(up.due(at,true,data));assert(up.queued(20+i,4,20+i,1,at));
  assert(up.complete(20+i,4,20+i,1,2,at,100)); // local only, no peer ACK
  assert(!up.due(at+499,true,data));
 }
 assert(!up.due(10700,true,data)&&up.state()==S::Failed);
 p.transaction=3;assert(up.begin(p,1,11000,true));assert(up.due(11000,true,data));
 assert(up.queued(30,5,30,1,11000));assert(!up.maintain(11001,false)&&up.inFlight());
 p.transaction=4;assert(!up.begin(p,1,11001,true));up.transportRevoked();assert(up.begin(p,1,11001,true));
 assert(up.due(11001,true,data)&&up.queued(31,6,31,1,11001));
 assert(!up.complete(30,5,30,1,3,11000,100));
 assert(!up.maintain(11101,true)&&up.inFlight());up.transportRevoked();
 up.resetAfterSession();p.transaction=1;const uint32_t start=UINT32_MAX-5;
 assert(up.begin(p,2,start,true)&&up.due(1,true,data)&&up.queued(32,7,32,2,1));
 assert(up.complete(32,7,32,2,1,1,100));
 assert(!up.acknowledge(1,MISSION_BEGIN,MISSION_PARTIAL,2,0,2,true));
 assert(up.acknowledge(1,MISSION_BEGIN,MISSION_PARTIAL,2,2,2,true));
 assert(!up.maintain(8994,true));
}
